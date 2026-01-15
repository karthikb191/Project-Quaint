#ifndef _H_VULKAN_RESOURCES
#define _H_VULKAN_RESOURCES
#include <vulkan/vulkan.h>
#include <GFX/Interface/IRenderer.h>
#include <GFX/Interface/IEntityInterfaces.h>
#include <GFX/Vulkan/Internal/Entities/VulkanTexture.h>
#include "../VulkanShaderGroup.h"

namespace Bolt{ 
    class CombinedImageSamplerTextureBuilder;
    class BufferResourceBuilder;

    namespace vulkan {
    
    //TODO: Should other resource class also be created and have a separation with entities?
    
    // Resource would own the enclosing API onbject created
    class VulkanCombinedImageSamplerResource : public IImageSamplerImpl
    {
    public:
        VulkanCombinedImageSamplerResource(Quaint::IMemoryContext* context, VkSampler sampler, VulkanTextureRef& textureRef)
        : IImageSamplerImpl(context)
        , m_sampler(sampler)
        , m_texture(std::move(textureRef))
        {}

        virtual void constructFromPath(char* path) override;
        virtual void constructFromPixels(void* pixels, uint32_t width, uint32_t height) override;
        virtual void destroy() override;
        
        VkSampler getSampler() const { return m_sampler; }
        const VulkanTextureRef& getTexture() { return m_texture; }

    private:
        void setSamplerInfo(const VkSamplerCreateInfo& info) { m_samplerInfo = info; }

        VkSamplerCreateInfo     m_samplerInfo;

        VkSampler               m_sampler = VK_NULL_HANDLE;
        VulkanTextureRef        m_texture;
    };

    class VulkanBufferObjectResource : public IBufferImpl
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
        : IBufferImpl(context)
        {}

        virtual void construct() override;
        virtual void construct(void* data) override;
        virtual void destroy() override;
        VkBuffer getBufferhandle() { return m_buffer; }
        VkDeviceMemory getDeviceMemoryHandle() { return m_gpuMemoryHandle; }
        const BufferInfo& getBufferInfo() { return m_info; }
        
        virtual void map() override;
        virtual void unmap() override;
        bool isUniformBuffer() const { return m_info.usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
        bool isMapped() const { return m_isMapped; }
        virtual void** getMappedRegion() override { return &m_mapRegion; }

    private:
        friend class Bolt::BufferResourceBuilder;

        void setBufferInfo(const BufferInfo& info) { m_info = info; }

        //TODO: Maybe encapsulate into a buffer obejct?
        VkDeviceMemory          m_gpuMemoryHandle = VK_NULL_HANDLE;
        VkBuffer                m_buffer = VK_NULL_HANDLE;
        BufferInfo              m_info;
        VkBufferUsageFlags      m_usageFlags;
        VkMemoryPropertyFlags   m_memFlags;
        bool                    m_isMapped = false;
        void*                   m_mapRegion;
    };
}}

#endif