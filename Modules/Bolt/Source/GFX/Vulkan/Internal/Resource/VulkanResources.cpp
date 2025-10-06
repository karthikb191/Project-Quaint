#include <GFX/Vulkan/Internal/Resource/VulkanResources.h>
#include <GFX/Vulkan/VulkanRenderer.h>

namespace Bolt { namespace vulkan{
    
    void VulkanCombinedImageSamplerResource::constructFromPath(char* path)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        //TODO: Should get proper usage flags from a builder or something
        VulkanTexture* texture = QUAINT_NEW(m_context, VulkanTexture, m_context);
        //TODO: Hardcoding flag for now. Create a comming enum for that later
        VulkanRenderer::get()->createTextureFromFile(path, *texture, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        m_texture.reset(texture);
        
        VkResult res = vkCreateSampler(device, &m_samplerInfo, callbacks, &m_sampler);
    }
    void VulkanCombinedImageSamplerResource::constructFromPixels(void* pixels, uint32_t width, uint32_t height)
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VkResult res = vkCreateSampler(device, &m_samplerInfo, callbacks, &m_sampler);
        
        //TODO: Should get proper usage flags from a builder or something
        VulkanTexture* texPtr = QUAINT_NEW(m_context, VulkanTexture, m_context);
        VulkanRenderer::get()->createShaderTextureFromPixels(*texPtr, (unsigned char*)pixels, width, height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    }

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
    
    void VulkanBufferObjectResource::construct()
    {
        //TODO: Null check buffer handles
        VulkanRenderer::get()->createBuffer(m_info.size, m_info.usageFlags, m_info.memFlags, m_gpuMemoryHandle, m_buffer);
    }

    void VulkanBufferObjectResource::construct(void* data)
    {
        //TODO: Null check buffer handles
        VulkanRenderer::get()->createBuffer(m_info.size, data, m_info.usageFlags, m_info.memFlags, m_gpuMemoryHandle, m_buffer);
    }

    void VulkanBufferObjectResource::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        
        if(m_gpuMemoryHandle != VK_NULL_HANDLE)
        {
            if(isMapped())
            {
                unmap();
            }

            vkFreeMemory(device, m_gpuMemoryHandle, callbacks);
        }
        if(m_buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, m_buffer, callbacks);
        }
        m_gpuMemoryHandle = VK_NULL_HANDLE;
        m_buffer = VK_NULL_HANDLE;
    }

    void VulkanBufferObjectResource::map()
    {
        if(!isUniformBuffer() || isMapped())
        {
            //TODO: Add a log
            return;
        }
        
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VkResult res = vkMapMemory(device, m_gpuMemoryHandle, 0, m_info.size, 0, &m_mapRegion);
        ASSERT_SUCCESS(res, "could not map memory");
    }

    void VulkanBufferObjectResource::unmap()
    {
        if(!isUniformBuffer() || !isMapped())
        {
            //add a log
            return;
        }
        
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        vkUnmapMemory(device, m_gpuMemoryHandle);
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