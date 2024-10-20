#include <GFX/Vulkan/Internal/VulkanRenderObject.h>
#include <GFX/Entities/RenderObject.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/RenderScene.h>
#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Entities/Resources.h>
#include <Types/QMap.h>

namespace Bolt { namespace vulkan {
    VulkanRenderObject::VulkanRenderObject(RenderObject* ro)
    : IRenderObjectImpl(ro)
    //, m_shaderGroup(nullptr,  Deleter<VulkanShaderGroup>(ro->getMemoryContext()))
    , m_setLayouts(ro->getMemoryContext())
    , m_sets(ro->getMemoryContext())
    {

    }

    void VulkanRenderObject::build(const GeometryRenderInfo& renderInfo)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        
        m_shaderGroupResource = static_cast<VulkanShaderGroupResource*>(renderInfo.shaderGroupResource->getGpuResourceProxy());

        ShaderInfo shaderInfo{};
        
        shaderInfo.maxResourceSets = 1;
        shaderInfo.resources = m_shaderGroupResource->getAttachmentRefs();
        //TODO: Hardcoding for now, but get this information from ShaderData. Doesn't make sense otherwise
        //createShaderGroup(shaderinfo); // Probably not needed as shader group would've already been created
        
        createDescriptorLayoutInformation(shaderInfo);
        allocateDescriptorPool(shaderInfo);
        createDescriptors(shaderInfo);
        createPipeline();

        //CombinedImageSamplerInfo info{};
        //ShaderResource<EShaderResourceType::COMBINED_IMAGE_SAMPLER> imageSamplerResource(info);
        //Resource* resource = &imageSamplerResource;

        //ShaderResource<EShaderResourceType::COMBINED_IMAGE_SAMPLER>* shRes = resource
        //                            ->get<EResourceType::ShaderResource>()
        //                            ->get<EShaderResourceType::COMBINED_IMAGE_SAMPLER>();

        //TODO: Probably move this to a texture class
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.unnormalizedCoordinates = VK_FALSE; // We are using normalized coordinates
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.magFilter = VK_FILTER_LINEAR;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        //TODO: Not entirely sure of the way this functions. Need to read more on this
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 0;

        VkResult res = vkCreateSampler(device, &samplerInfo, callbacks, &m_sampler);

        //TODO: These should come from some resource class in association with shader info
        //Descriptor sets are allocated, but they arent filled with approproate values        
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = getUBOBuffer_Temp(); //TODO: Remove this
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        // For frame-in-flight buffer, I could try playing with offsets and assigning a single buffer

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = getTexture_Temp()->getImageView();
        imageInfo.sampler = m_sampler;
    
        Quaint::QFastArray<VkWriteDescriptorSet, 2> writes;
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].dstSet = m_sets[0];
        writes[0].dstBinding = 0;
        writes[0].dstArrayElement = 0;
        writes[0].pBufferInfo = &bufferInfo;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].dstSet = m_sets[0];
        writes[1].dstBinding = 1;
        writes[1].dstArrayElement = 0;
        writes[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, writes.getSize(), writes.getBuffer(), 0, nullptr);
    }

    void VulkanRenderObject::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        //if(m_shaderGroup.get())
        //{
        //    m_shaderGroup->destroy();
        //    m_shaderGroup.release();
        //}

        //Destroying pipeline layouts
        for(auto& layout : m_setLayouts)
        {
            vkDestroyDescriptorSetLayout(device, layout, callbacks);
        }
        m_setLayouts.clear();

        if(m_pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, m_pipelineLayout, callbacks);
            m_pipelineLayout = VK_NULL_HANDLE;
        }

        if(m_descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device, m_descriptorPool, callbacks);
            m_descriptorPool = VK_NULL_HANDLE;
        }
    }

    //void VulkanRenderObject::createShaderGroup(const ShaderInfo& shaderinfo)
    //{
    //    m_shaderGroup.reset(QUAINT_NEW(m_renderObject->getMemoryContext(), VulkanShaderGroup
    //                        , shaderinfo.vertShaderPath.getBuffer()
    //                        , shaderinfo.fragShaderPath.getBuffer()));
    //}
    
    void VulkanRenderObject::createDescriptorLayoutInformation(const ShaderInfo& shaderinfo)
    {
        Quaint::IMemoryContext* memContext = m_renderObject->getMemoryContext();
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        Quaint::QArray<
        Quaint::QArray<VkDescriptorSetLayoutBinding>> setBindingInfos(memContext, (size_t)shaderinfo.maxResourceSets);
        //Initialize Inner array
        for(auto& it : setBindingInfos)
        {
            it = Quaint::QArray<VkDescriptorSetLayoutBinding>(memContext);
        }

        for(auto& resource : shaderinfo.resources)
        {
            // TODO: Handle this better
            assert(resource.set < shaderinfo.maxResourceSets && "Ivalid Resource set index");
            assert(resource.count > 0 && "Invalid resouce count");

            Quaint::QArray<VkDescriptorSetLayoutBinding>& bindings = setBindingInfos[resource.set];
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = resource.binding;
            binding.descriptorCount = resource.count;
            binding.descriptorType = toVulkanDescriptorType(resource.resourceType);
            binding.stageFlags = toVulkanShaderStageFlagBits(resource.shaderStage);
            bindings.pushBack(binding);
        }

        //Create descriptor set layouts here
        for(auto& bindings : setBindingInfos)
        {
            VkDescriptorSetLayoutCreateInfo info {};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.flags = 0;
            info.pNext = nullptr;
            info.bindingCount = bindings.getSize();
            info.pBindings = bindings.getBuffer();

            VkDescriptorSetLayout layout;
            ASSERT_SUCCESS(vkCreateDescriptorSetLayout(device, &info, callbacks, &layout), "Failed to create descriptor set layout");
            m_setLayouts.pushBack(layout);
        }

        VkPipelineLayoutCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = m_setLayouts.getSize();
        info.pSetLayouts = m_setLayouts.getBuffer();

        info.pushConstantRangeCount = 0; //TODO: Not handling it for now 
        info.pPushConstantRanges = nullptr; //TODO: Not handling it for now
        info.flags = 0;
        info.pNext = nullptr;
        ASSERT_SUCCESS(vkCreatePipelineLayout(device, &info, callbacks, &m_pipelineLayout), "Failed to create pipeline layout");
    }

    void VulkanRenderObject::allocateDescriptorPool(const ShaderInfo& shaderInfo)
    {
        Quaint::IMemoryContext* memContext = m_renderObject->getMemoryContext();
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        
        Quaint::QMap<VkDescriptorType, uint32_t> numTypes(m_renderObject->getMemoryContext());
        for(auto& resource : shaderInfo.resources)
        {
            VkDescriptorType type = toVulkanDescriptorType(resource.resourceType);
            if(!numTypes.contains(type))
            {
                numTypes.insert({type, 1});
            }
            else
            {
                uint8_t increment = 1; // !!TODO: resource.perFrame ? MAX_FRAMES_IN_FLIGHT : 1;
                numTypes[type] += increment; 
            }
        }

        Quaint::QArray<VkDescriptorPoolSize> poolSizes(m_renderObject->getMemoryContext());

        for(auto& entry : numTypes)
        {
            VkDescriptorPoolSize size{};
            size.type = entry.first;
            size.descriptorCount = entry.second;
            poolSizes.pushBack(size);
        }

        //TODO: Get this data from shader data
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = m_setLayouts.getSize();
        poolInfo.poolSizeCount = poolSizes.getSize();
        poolInfo.pPoolSizes = poolSizes.getBuffer();
        poolInfo.flags = 0;
        poolInfo.pNext = nullptr;

        ASSERT_SUCCESS(vkCreateDescriptorPool(device, &poolInfo, callbacks, &m_descriptorPool), "Failed to create descriptor pool");
    }

    void VulkanRenderObject::createDescriptors(const ShaderInfo& shaderInfo)
    {
        assert(m_descriptorPool != VK_NULL_HANDLE && "descriptor pool not allocated");
        
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        //Allocating Descriptor sets from descriptor pool
        VkDescriptorSetAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = m_descriptorPool;
        info.descriptorSetCount = m_setLayouts.getSize();
        info.pSetLayouts = m_setLayouts.getBuffer();

        //Creating descriptor Sets
        m_sets.resize(m_setLayouts.getSize());
        VkResult res = vkAllocateDescriptorSets(device, &info, m_sets.getBuffer_NonConst());
        assert(res == VK_SUCCESS && "Could not allocate descriptor sets");
    }

    //TODO: how to handle this?
    void VulkanRenderObject::writeDescriptorSets()
    {

    }

    void VulkanRenderObject::createPipeline()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VulkanShaderGroup& shaderGroup = m_shaderGroupResource->getShaderGroup();

        VkPipelineShaderStageCreateInfo vertStageInfo{};
        vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageInfo.module = shaderGroup.getVertexShader()->getHandle();
        vertStageInfo.pName = "main"; //TODO: Pass from shader info
        //vertStageInfo.pSpecializationInfo = //This lets us specify values for shader constants. TODO: Read more on this later 

        VkPipelineShaderStageCreateInfo fragStageInfo{};
        fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStageInfo.module = shaderGroup.getFragmentShader()->getHandle();
        fragStageInfo.pName = "main"; // TODO: 

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};

        //create Dynamic states. Specifying these ignores the configuration of these values and these must be specified at drawing time
        //TODO: No dynamic states for now
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = 0;
        dynamicStateInfo.pDynamicStates = nullptr;

        //Pass in vertex input. This structure describes the format of vertex data that will be passed to the vertex shader
        //Populated below
        VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
        vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        //TODO: Fill these
        //Spacing between data and whether data is per-vertex or per-instance
        vertexInputStateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateInfo.pVertexBindingDescriptions = nullptr;

        //Types of attributes passed to vertex shader, which binding to load them from and at which offset
        vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateInfo.pVertexAttributeDescriptions = nullptr;

        //TODO: Get this info from RenderObejct data
        //Input Assembly: We specify what kind of geometry to be drawn an if primitiveRestart should be enabled. Not sure what this is yet
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        //Setting up Viewport and scissor
        const VkExtent2D& extent = VulkanRenderer::get()->getRenderFrameScene()->getSwapchainExtent();
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;

        VkPipelineViewportStateCreateInfo viewportStateInfo{};
        viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.pScissors = &scissor;
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports = &viewport;

        //Rasterizer: Takes geometry shaped by the vertices and turns it into fragments
        //Also performs depth testing, face culling
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        //If this is set to true, fragments that are beyond near and far planes are clamped as opposed to discarded.
        //This is useful in shadow maps;
        rasterizationInfo.depthClampEnable = VK_FALSE;
        //If this is true, geometry never passes through rasterization stage and nothing gets drawn to framebuffer
        rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.lineWidth = 1.0f;
        rasterizationInfo.cullMode = VK_CULL_MODE_NONE;// VK_CULL_MODE_BACK_BIT; //Cull back faces
        rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //Vertices are processed in clock-wise direction
        rasterizationInfo.depthBiasEnable = VK_FALSE;

        //Multi-sampling: helps with alnti aliasing. More on this later
        VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
        multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingInfo.sampleShadingEnable = VK_FALSE;
        multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        //TODO: Depth and Stencil testing: We have to set up a structure for depth and stencil testing here late

        //Color Blending: 
        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo blendInfo{};
        blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendInfo.attachmentCount = 1;
        blendInfo.pAttachments = &blendAttachmentState;
        blendInfo.logicOpEnable = VK_FALSE;
        blendInfo.logicOp = VK_LOGIC_OP_COPY;
        blendInfo.blendConstants[0] = 0.0f;
        blendInfo.blendConstants[1] = 0.0f;
        blendInfo.blendConstants[2] = 0.0f;
        blendInfo.blendConstants[3] = 0.0f;

        VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
        graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        //Shader Modules setup
        graphicsPipelineInfo.stageCount = 2;
        graphicsPipelineInfo.pStages = shaderStages;
        
        //Fixed stage setup
        graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;

//-------------Vertex Shader input binding----------------------
        //TODO: Generate this information from a string

        //TODO: Get this from some info structure or a render object
        VkVertexInputBindingDescription inputBindingDesc{};
        inputBindingDesc.binding = 0;
        inputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDesc.stride = sizeof(Quaint::QVertex);

        //auto inputBindingDesc = QVertex::getBindingDescription();
        Quaint::QFastArray<VkVertexInputAttributeDescription, 3> attributeDesc;
        constexpr auto posOff = Quaint::QVertex::getPositionOffset();
        constexpr auto colorOff = Quaint::QVertex::getColorOffset();
        constexpr auto texCoordOff = Quaint::QVertex::getTexCoordOffset();
        //Position
        attributeDesc[0].binding = 0;
        attributeDesc[0].location = 0;
        attributeDesc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDesc[0].offset = Quaint::QVertex::getPositionOffset();

        //Color
        attributeDesc[1].binding = 0;
        attributeDesc[1].location = 1;
        attributeDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDesc[1].offset = Quaint::QVertex::getColorOffset();

        //Texcoord
        attributeDesc[2].binding = 0;
        attributeDesc[2].location = 2;
        attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDesc[2].offset = Quaint::QVertex::getTexCoordOffset();

        //TODO: This probably doesnt get correct values
        vertexInputStateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateInfo.pVertexBindingDescriptions = &inputBindingDesc;

        vertexInputStateInfo.vertexAttributeDescriptionCount = attributeDesc.getSize();
        vertexInputStateInfo.pVertexAttributeDescriptions = attributeDesc.getBuffer();

        graphicsPipelineInfo.pVertexInputState = &vertexInputStateInfo;
//---------------------------------------------------------------

        graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        graphicsPipelineInfo.pViewportState = &viewportStateInfo;
        graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;
        graphicsPipelineInfo.pMultisampleState = &multisamplingInfo;
        graphicsPipelineInfo.pColorBlendState = &blendInfo;
        graphicsPipelineInfo.pDepthStencilState = nullptr;

        graphicsPipelineInfo.layout = m_pipelineLayout;

        //TODO: Somehow handle this better
        VulkanRenderPass* renderPass = VulkanRenderer::get()->getRenderFrameScene()->getRenderPass();
        
        graphicsPipelineInfo.renderPass = renderPass->getRenderPass();
        graphicsPipelineInfo.subpass = 0; 

        //Vulkan lets you create new pipelines by deriving from existing ones. More on this later
        graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipelineInfo.basePipelineIndex = 0;

        ASSERT_SUCCESS(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, callbacks, &m_pipeline)
        , "Failed to create Graphics pipeline");
    }

    void VulkanRenderObject::draw()
    {
        FrameInfo& info = VulkanRenderer::get()->getRenderFrameScene()->getCurrentFrameInfo();
        
        vkCmdBindPipeline(info.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        vkCmdBindDescriptorSets(info.commandBuffer,
         VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, m_sets.getBuffer(), 0, nullptr);
    }

}}