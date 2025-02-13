#include <GFX/Vulkan/Internal/Entities/VulkanPipeline.h>
#include <GFX/Vulkan/Internal/VulkanShader.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/VulkanHelpers.h>
#include <GFX/ResourceBuilder.h>

namespace Bolt
{
    VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(Quaint::IMemoryContext* context)
    : m_context(context)
    {
        m_pipeline = QUAINT_NEW(m_context, vulkan::VulkanGraphicsPipeline, m_context);
    }
    
    VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::setupShaders(const ShaderDefinition& definition)
    {
        m_pipeline->buildShaders(definition);
    }
        
    vulkan::VulkanGraphicsPipeline* VulkanGraphicsPipelineBuilder::build()
    {
        m_pipeline->init();
        return m_pipeline;
        //TODO: Initialize with some default information. Can specialize later
    }


    namespace vulkan
    {
        void VulkanGraphicsPipeline::buildShaders(const ShaderDefinition& definition)
        {
            m_shaderDefinition = definition;
            for(auto& def : m_shaderDefinition.shaders)
            {
                VulkanShader* shader = QUAINT_NEW(m_context, VulkanShader, def.stage, def.path.getBuffer(), def.entry);
                VkPipelineShaderStageCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                info.pName = def.name.getBuffer();
                info.module = shader->getHandle();
                info.stage = toShaderStageFlags(def.stage);
                info.flags = 0;
                info.pNext = nullptr;
                m_shaderStageInfos.pushBack(info);
                m_shaders.pushBack(shader);
            }
        }


        void VulkanGraphicsPipeline::init()
        {
            VkGraphicsPipelineCreateInfo info{};

            VkDevice device = VulkanRenderer::get()->getDevice();
            VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
            if(m_pipeline != VK_NULL_HANDLE)
            {
                assert(false && "Cannot initialize with a valid pipeline already available. Destroy pipeline and it's related resources");
                return;
            }
            VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, callbacks, &m_pipeline);

            ASSERT_SUCCESS(res, "Could not create graphics pipeline");
        }

        void VulkanGraphicsPipeline::destroy()
        {

        }
    }
}