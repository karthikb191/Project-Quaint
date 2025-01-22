#ifndef _H_VULKAN_RESOURCES
#define _H_VULKAN_RESOURCES
#include <vulkan/vulkan.h>
#include <GFX/Interface/IRenderer.h>
#include <GFX/Vulkan/Internal/Entities/VulkanTexture.h>
#include "../VulkanShaderGroup.h"

namespace Bolt{ 
    class CombinedImageSamplerTextureBuilder;

    namespace vulkan {
    
    // Resource would own the enclosing API onbject created
    class VulkanCombinedImageSamplerResource : public Bolt::ResourceGPUProxy
    {
    public:
        VulkanCombinedImageSamplerResource(Quaint::IMemoryContext* context)
        : Bolt::ResourceGPUProxy(context)
        {}
        virtual void destroy() override;
        
        VkSampler getSampler() const { return m_sampler; }
        const VulkanTexture& getTexture() { return m_texture; }

    private:
        friend class CombinedImageSamplerTextureBuilder;

        void wrap(VkSampler sampler, VulkanTexture& texture);

        VkSampler           m_sampler = VK_NULL_HANDLE;
        VulkanTexture       m_texture;
    };

    class VulkanBufferObjectResource : public Bolt::ResourceGPUProxy
    {
    public:
        struct BufferInfo
        {
            uint32_t size;
            uint32_t offset;
            VkBufferUsageFlags usageFlags;
            VkMemoryPropertyFlags memFlags;
        };

        VulkanBufferObjectResource(Quaint::IMemoryContext* context)
        : Bolt::ResourceGPUProxy(context)
        {}

        void wrap(VkDeviceMemory deviceMemory, VkBuffer buffer, const BufferInfo& info);
        virtual void destroy() override;
        VkBuffer getBufferhandle() { return m_buffer; }
        VkDeviceMemory getDeviceMemoryHandle() { return m_gpuMemoryHandle; }
        const BufferInfo& getBufferInfo() { return m_info; }

    private:
        //TODO: Maybe encapsulate into a buffer obejct?
        VkDeviceMemory          m_gpuMemoryHandle = VK_NULL_HANDLE;
        VkBuffer                m_buffer = VK_NULL_HANDLE;
        BufferInfo              m_info;
        VkBufferUsageFlags      m_usageFlags;
        VkMemoryPropertyFlags   m_memFlags;
    };
}}

#endif