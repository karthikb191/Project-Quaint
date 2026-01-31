#ifndef _H_VULKAN_FRAME_BUFFER
#define _H_VULKAN_FRAME_BUFFER

#include "Interface/IMemoryContext.h"
#include <Types/QArray.h>
#include <GFX/Vulkan/Internal/Entities/VulkanTexture.h>

namespace Bolt{ namespace vulkan{

    class VulkanRenderScene;
    struct AttachmentInfo;

    /* Creates and Owns any attachments provided */
    class FrameBuffer
    {
    public:
        FrameBuffer(Quaint::IMemoryContext* context);
        void construct(const VulkanRenderScene* scene);
        void destroy();

        VkFramebuffer getHandle();
    private:
        Quaint::IMemoryContext*             m_context;
        VkFramebufferCreateInfo             m_info = {};
        Quaint::QArray<VkFramebuffer>       m_framebuffers;
        bool                                m_dependsOnSwapchain = false;
    };

} }

#endif