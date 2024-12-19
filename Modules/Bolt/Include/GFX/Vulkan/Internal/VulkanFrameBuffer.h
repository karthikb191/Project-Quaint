#ifndef _H_VULKAN_FRAME_BUFFER
#define _H_VULKAN_FRAME_BUFFER

#include "Interface/IMemoryContext.h"
#include <Types/QArray.h>
#include <GFX/Vulkan/Internal/Entities/VulkanTexture.h>

namespace Bolt{ namespace vulkan{

    class Bolt::RenderScene;
    struct AttachmentInfo;

    /* Creates and Owns any attachments provided */
    class FrameBuffer
    {
    public:
        FrameBuffer(Quaint::IMemoryContext* context);
        void construct(const RenderScene* scene);
        void destroy();

        FrameBuffer& addAttachment(const VkImage swapchainImage, const AttachmentInfo* info, const Quaint::QArray<uint32_t>& queueIndices);
        FrameBuffer& setExtent(const VkExtent3D& extent) { m_info.width = extent.width; m_info.height = extent.height; m_info.layers = extent.depth; return *this; }
        FrameBuffer& setRenderpass(const VkRenderPass renderpass) { m_info.renderPass = renderpass; return *this; }
        VkFramebuffer getHandle() { return m_framebuffer; }
    private:
        Quaint::IMemoryContext*             m_context;
        Quaint::QArray<VulkanTexture>       m_attachments;
        VkFramebufferCreateInfo             m_info = {};
        VkFramebuffer                       m_framebuffer = VK_NULL_HANDLE;
    };

} }

#endif