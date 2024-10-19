#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt { namespace vulkan{
    void VulkanCombinedImageSamplerResource::wrap(VkSampler sampler, VulkanTexture& texture)
    {
        m_sampler = sampler;
        m_texture = texture;
    }
    
    void VulkanCombinedImageSamplerResource::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        if(m_sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, m_sampler, callbacks);
        }

        m_texture.destroy();
        m_sampler = VK_NULL_HANDLE;
    }


    void VulkanUniformBufferObjectResource::destroy()
    {
        //TODO:
    }
}}