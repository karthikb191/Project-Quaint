#ifndef _H_VULKAN_RESOURCES
#define _H_VULKAN_RESOURCES
#include <vulkan/vulkan.h>
#include <GFX/Interface/IRenderer.h>
#include <GFX/Vulkan/Internal/Texture/VulkanTexture.h>
#include "../VulkanShaderGroup.h"

namespace Bolt{ 
    class CombinedImageSamplerTextureBuilder;

    namespace vulkan {
    
    // Resource would own the enclosing API onbject created
    class VulkanCombinedImageSamplerResource : public ResourceGPUProxy
    {
    public:
        VulkanCombinedImageSamplerResource(Quaint::IMemoryContext* context)
        : ResourceGPUProxy(context)
        {}
        virtual void destroy() override;
        
        VkSampler getSampler() const { return m_sampler; }
        const VulkanTexture& getTexture() const { return m_texture; }

    private:
        friend class CombinedImageSamplerTextureBuilder;

        void wrap(VkSampler sampler, VulkanTexture& texture);

        VkSampler           m_sampler = VK_NULL_HANDLE;
        VulkanTexture       m_texture;
    };

    class VulkanBufferObjectResource : public ResourceGPUProxy
    {
    public:
        VulkanBufferObjectResource(Quaint::IMemoryContext* context)
        : ResourceGPUProxy(context)
        {}

        void wrap(VkDeviceMemory deviceMemory, VkBuffer buffer, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memFlags);
        virtual void destroy() override;
        VkBuffer getBufferhandle() { return m_buffer; }
        VkDeviceMemory getDeviceMemoryHandle() { return m_gpuMemoryHandle; }

    private:
        //TODO: Maybe encapsulate into a buffer obejct?
        VkDeviceMemory          m_gpuMemoryHandle = VK_NULL_HANDLE;
        VkBuffer                m_buffer = VK_NULL_HANDLE;
        VkBufferUsageFlags      m_usageFlags;
        VkMemoryPropertyFlags   m_memFlags;
    };

    class VulkanShaderGroupResource : public ResourceGPUProxy
    {
    public:
        VulkanShaderGroupResource(Quaint::IMemoryContext* context)
        : ResourceGPUProxy(context)
        , m_attachmentRefs(context)
        {}

        void wrap(const Quaint::QArray<ShaderAttachmentInfo>& attachments, VulkanShaderGroup&& shaderGroup);
        virtual void destroy() override;

        VulkanShaderGroup& getShaderGroup() { return m_shaderGroup; }
        Quaint::QArray<ShaderAttachmentInfo>& getAttachmentRefs() { return m_attachmentRefs; }
    private:
        VulkanShaderGroup                       m_shaderGroup;
        Quaint::QArray<ShaderAttachmentInfo>    m_attachmentRefs;
    };
}}

#endif