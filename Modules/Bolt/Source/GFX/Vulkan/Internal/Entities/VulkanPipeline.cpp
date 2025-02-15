#include <GFX/Vulkan/Internal/Entities/VulkanPipeline.h>
#include <GFX/Vulkan/Internal/VulkanShader.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/Entities/RenderScene.h>
#include <GFX/ResourceBuilder.h>

namespace Bolt
{
    VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(Quaint::IMemoryContext* context)
    : m_context(context)
    {
        m_pipeline = QUAINT_NEW(m_context, vulkan::VulkanGraphicsPipeline, m_context);
        //Initially setting it up with defaults
        m_pipeline->setInputAssemblyState(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        // Fill the polygons and discard backfaces by default
        m_pipeline->setRasterizationInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    }
    
    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::setupShaders(const ShaderDefinition& definition)
    {
        m_pipeline->buildShaders(definition);
        m_pipeline->setVertexAttributes(definition, VK_VERTEX_INPUT_RATE_VERTEX);
    }
    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::setupPrimitiveTopology(bool primitiveRestartEnabled, VkPrimitiveTopology topology)
    {
        m_pipeline->setInputAssemblyState(primitiveRestartEnabled, topology);
    }
    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::setupRenderStageInfo(const RenderScene* const scene, const uint32_t stageIndex, bool addViewport)
    {
        assert(scene != nullptr && "Invalid render scene passed");
        assert(stageIndex >= 0 && stageIndex < scene->getRenderStages().getSize(), "Invalid render stage passed");
        m_pipeline->setRenderStage(scene, stageIndex);
        if(addViewport)
        {
            const RenderInfo& renderInfo = scene->getRenderInfo();
            m_pipeline->addViewport(renderInfo.offset.x, renderInfo.offset.y, renderInfo.extents.x, renderInfo.extents.y, 0.0f, 1.0f
                            , VkOffset2D{0, 0}, VkExtent2D{(uint32_t)renderInfo.extents.x, (uint32_t)renderInfo.extents.y});
        }
    }
    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::setupRasterizationInfo(VkPolygonMode polyMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace)
    {
        m_pipeline->setRasterizationInfo(polyMode, cullMode, frontFace);
    }
    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::setupBlendState(const RenderStage* const stage)
    {
        auto& attachmentRefs = stage->attachmentRefs;
        for(int i = 0; i < attachmentRefs.getSize(); ++i)
        {
            if(attachmentRefs[i].attachmentName == "swapchain")
            {
                
            }
        }
    }
        
    vulkan::VulkanGraphicsPipeline* VulkanGraphicsPipelineBuilder::build()
    {
        m_pipeline->init();
        return m_pipeline;
        //TODO: Initialize with some default information. Can specialize later
    }


    namespace vulkan
    {
        VulkanGraphicsPipeline::VulkanGraphicsPipeline(Quaint::IMemoryContext* context)
        : m_context(context)
        , m_shaders(context)
        , m_shaderStageInfos(context)
        , m_bindingDescs(context)
        , m_attrDescs(context)
        , m_viewports(context)
        , m_scissors(context)
        {
        }
        void VulkanGraphicsPipeline::buildShaders(const ShaderDefinition& definition)
        {
            m_shaderDefinition = definition;
            for(auto& def : m_shaderDefinition.shaders)
            {
                VulkanShader* shader = QUAINT_NEW(m_context, VulkanShader, def.stage, def.path.getBuffer(), def.entry);
                VkPipelineShaderStageCreateInfo stageInfo{};
                stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stageInfo.pName = def.name.getBuffer();
                stageInfo.module = shader->getHandle();
                stageInfo.stage = toShaderStageFlags(def.stage);
                stageInfo.flags = 0;
                stageInfo.pNext = nullptr;
                m_shaderStageInfos.pushBack(stageInfo);
                m_shaders.pushBack(shader);
            }
        }

        void VulkanGraphicsPipeline::setVertexAttributes(const ShaderDefinition& definition, VkVertexInputRate pInputRate)
        {
            //TODO: This could be improved to be able to validate data much better
            uint32_t bindingIdx = 0;
            for(uint32_t i = 0; i < definition.attributeSets.getSize(); ++i)
            {
                auto& set = definition.attributeSets[i];
                uint32_t stride = 0;
                VkVertexInputBindingDescription bindingDesc {};

                uint32_t initialOffset = 0;
                for(int j = 0; j < set.getSize(); ++j)
                {
                    VkVertexInputAttributeDescription attrDesc{};
                    auto& attribute = set[j];
                    attrDesc.binding = i;
                    attrDesc.format = toVulkanVkFormat(attribute.format);
                    attrDesc.location = j;
                    attrDesc.offset = initialOffset;
                    m_attrDescs.pushBack(attrDesc);
                }
                assert(stride > 0 && "No valid attributes found");
                if(stride <= 0) continue; 

                bindingDesc.binding = i; 
                bindingDesc.inputRate = pInputRate;
                m_bindingDescs.pushBack(bindingDesc);
            }
        }

        void VulkanGraphicsPipeline::setInputAssemblyState(const bool pRestartEnabled, const VkPrimitiveTopology pTopology)
        {
            m_restartEnabled = pRestartEnabled;
            m_topology = pTopology;
        }

        void VulkanGraphicsPipeline::setRenderStage(const RenderScene* const scene, const uint32_t stageIndex)
        {
            const VulkanRenderScene* vulkanScene = scene->getRenderSceneImplAs<vulkan::VulkanRenderScene>();
            assert(vulkanScene != nullptr && "Vulkan scene has not been constructed");
            if(!vulkanScene) return;

            m_renderPass = vulkanScene->getRenderpass();
            m_subPass = stageIndex;
        }
        void VulkanGraphicsPipeline::addViewport(float x, float y, float width, float height, float minDepth, float maxDepth, VkOffset2D scissorOffset, VkExtent2D scissorExtent)
        {
            VkViewport viewport{};
            viewport.x = x;
            viewport.y = y;
            viewport.width = width;
            viewport.height = height;
            viewport.minDepth = minDepth;
            viewport.maxDepth = maxDepth;
            
            VkRect2D scissor{};
            scissor.offset = scissorOffset;
            scissor.extent = scissorExtent;

            m_viewports.pushBack(viewport);
            m_scissors.pushBack(scissor);
        }
        void VulkanGraphicsPipeline::setRasterizationInfo(VkPolygonMode polyMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace)
        {
            m_polygonMode = polyMode;
            m_cullMode = cullMode;
            m_frontFace = frontFace;
        }


        void VulkanGraphicsPipeline::init()
        {
            VkDevice device = VulkanRenderer::get()->getDevice();
            VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
            if(m_pipeline != VK_NULL_HANDLE)
            {
                assert(false && "Cannot initialize with a valid pipeline already available. Destroy pipeline and it's related resources");
                return;
            }
            VkGraphicsPipelineCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            //Shader stages Setup
            info.stageCount = m_shaderStageInfos.getSize();
            info.pStages = m_shaderStageInfos.getBuffer();

            //Vertex attribute setup
            VkPipelineVertexInputStateCreateInfo vertinfo{};
            vertinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertinfo.pNext = nullptr;
            vertinfo.vertexBindingDescriptionCount = m_bindingDescs.getSize();
            vertinfo.pVertexBindingDescriptions = m_bindingDescs.getBuffer();
            vertinfo.vertexAttributeDescriptionCount = m_attrDescs.getSize();
            vertinfo.pVertexAttributeDescriptions = m_attrDescs.getBuffer();

            info.pVertexInputState = &vertinfo;

            //Topology
            VkPipelineInputAssemblyStateCreateInfo assembleInfo{};
            assembleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            assembleInfo.flags = 0;
            assembleInfo.pNext = nullptr;
            assembleInfo.primitiveRestartEnable = m_restartEnabled;
            assembleInfo.topology = m_topology;

            info.pInputAssemblyState = &assembleInfo;

            //Tesselation
            //TODO: Not handling it currently
            info.pTessellationState = nullptr;

            //Viewport
            VkPipelineViewportStateCreateInfo viewportInfo{};
            viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportInfo.viewportCount = m_viewports.getSize();
            viewportInfo.pViewports = m_viewports.getBuffer();
            viewportInfo.scissorCount = m_scissors.getSize();
            viewportInfo.pScissors = m_scissors.getBuffer();

            info.pViewportState = &viewportInfo;

            //Rasterization
            VkPipelineRasterizationStateCreateInfo rasterInfo{};
            rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterInfo.pNext = nullptr;
            rasterInfo.flags = 0;
            rasterInfo.depthClampEnable = VK_FALSE;
            rasterInfo.rasterizerDiscardEnable = VK_FALSE;
            rasterInfo.polygonMode = m_polygonMode;
            rasterInfo.cullMode = m_cullMode;
            rasterInfo.frontFace = m_frontFace;
            rasterInfo.depthBiasEnable = VK_FALSE;
            rasterInfo.depthBiasConstantFactor = 0.f;
            rasterInfo.depthBiasClamp = 0.f;
            rasterInfo.depthBiasSlopeFactor = 1.f;
            rasterInfo.lineWidth = 1.f;

            info.pRasterizationState = &rasterInfo;

            //Multi-Sampling
            //TODO: This pipeline doesn't support multisampling currently. Extend as required later
            VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
            multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisamplingInfo.sampleShadingEnable = VK_FALSE;
            multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            info.pMultisampleState = &multisamplingInfo;

            
            // No Depth setup for now
            info.pDepthStencilState = nullptr;



            // No dynamic state for now
            info.pDynamicState = nullptr;

            //RenderPass, Subpass setup
            info.renderPass = m_renderPass;
            info.subpass = m_subPass;

            //Base Pipeline. TODO: Explore
            info.basePipelineHandle = VK_NULL_HANDLE;

            VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, callbacks, &m_pipeline);

            ASSERT_SUCCESS(res, "Could not create graphics pipeline");
        }

        void VulkanGraphicsPipeline::destroy()
        {

        }
    }
}