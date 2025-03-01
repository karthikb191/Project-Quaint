#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt { namespace vulkan{
    
    void VulkanCombinedImageSamplerResource::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        if(m_sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, m_sampler, callbacks);
        }

        if(m_texture.get())
        {
            m_texture->destroy();
            m_texture.release();
        }
        m_sampler = VK_NULL_HANDLE;
    }

    void VulkanBufferObjectResource::wrap(VkDeviceMemory deviceMemory, VkBuffer buffer, const BufferInfo& info)
    {
        m_gpuMemoryHandle = deviceMemory;
        m_buffer = buffer;
        m_info = info;
    }

    void VulkanBufferObjectResource::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        
        if(m_gpuMemoryHandle != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, m_gpuMemoryHandle, callbacks);
        }
        if(m_buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, m_buffer, callbacks);
        }
        m_gpuMemoryHandle = VK_NULL_HANDLE;
        m_buffer = VK_NULL_HANDLE;
    }

    //void VulkanShaderGroupResource::wrap(const Quaint::QArray<ShaderAttachmentInfo>& attachments, VulkanShaderGroup&& shaderGroup)
    //{
    //    m_attachmentRefs = attachments;
    //    m_shaderGroup.moveFrom(shaderGroup);
    //}
    //void VulkanShaderGroupResource::destroy()
    //{
    //    m_shaderGroup.destroy();
    //}
}}