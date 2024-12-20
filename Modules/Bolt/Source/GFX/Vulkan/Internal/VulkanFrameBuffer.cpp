#include <GFX/Vulkan/Internal/VulkanFrameBuffer.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/VulkanRenderScene.h>
#include <GFX/Vulkan/Internal/Entities/VulkanSwapchain.h>

namespace Bolt{ namespace vulkan{

    FrameBuffer::FrameBuffer(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_framebuffers(context)
    {
        m_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    }

    void FrameBuffer::destroy()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        for(auto& framebuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(device, framebuffer, callbacks);
        }
    }

    void FrameBuffer::construct(const VulkanRenderScene* scene)
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VulkanSwapchain* swapchain = VulkanRenderer::get()->getSwapchain();
        uint32_t numImages = swapchain->getNumSwapchainImages();

        
        m_info.width = swapchain->getSwapchainExtent().width;
        m_info.height = swapchain->getSwapchainExtent().height;
        m_info.layers = 1;
        m_info.pNext = nullptr;
        m_info.renderPass = scene->getRenderpass();
        for(int i = 0; i < numImages; ++i)
        {
            auto& attachments = scene->getAttachments();    
            Quaint::QArray<VkImageView> views(m_context);
            for(auto& attachment : attachments)
            {
                Bolt::AttachmentDefinition::Type type = attachment->getInfo().type;
                switch (type)
                {
                case Bolt::AttachmentDefinition::Type::Image:
                {
                    VkImageView view = attachment->As<ImageAttachment>()->texture.getImageView();
                    views.pushBack(view);
                    break;
                }
                case Bolt::AttachmentDefinition::Type::Swapchain:
                    views.pushBack(swapchain->getSwapchainImageView(i));
                    break;
                default:
                    break;
                }
            }
            m_info.attachmentCount = (uint32_t)views.getSize();
            m_info.pAttachments = views.getBuffer();

            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            ASSERT_SUCCESS(vkCreateFramebuffer(device, &m_info, callbacks, &framebuffer), "Failed to create framebuffer!!");
            m_framebuffers.pushBack(framebuffer);
        }
    }
}}