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

        const VkExtent2D& extents = scene->getRenderExtent();
        m_info.width = extents.width;
        m_info.height = extents.height;
        m_info.layers = 1;
        m_info.pNext = nullptr;
        m_info.renderPass = scene->getRenderpass();
        
        auto& attachments = scene->getAttachments(); 
        for(auto& attachment : attachments)
        {
            Bolt::AttachmentDefinition::Type type = attachment->getInfo().type;
            if(type == Bolt::AttachmentDefinition::Type::Swapchain)
            {
                m_dependsOnSwapchain = true;
            }
        }

        uint8_t numFramebuffersRequired = 1;
        if(m_dependsOnSwapchain)
        {
            numFramebuffersRequired = numImages;
        }

        for(size_t i = 0; i < numFramebuffersRequired; ++i)
        {   
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
                {
                    assert(m_dependsOnSwapchain && "If framebuffer is dependent on swapchain, the flag should be true");
                    views.pushBack(swapchain->getSwapchainImageView(i));
                    break;
                }
                case Bolt::AttachmentDefinition::Type::Depth:
                {
                    VkImageView view = attachment->As<ImageAttachment>()->texture.getImageView();
                    views.pushBack(view);
                    break;
                }
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


    void FrameBuffer::construct(const VulkanCubeMapRenderScene* scene)
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        VulkanSwapchain* swapchain = VulkanRenderer::get()->getSwapchain();
        uint32_t numImages = swapchain->getNumSwapchainImages();

        const VkExtent2D& extents = scene->getRenderExtent();
        m_info.width = extents.width;
        m_info.height = extents.height;
        m_info.layers = 1;
        m_info.pNext = nullptr;
        m_info.renderPass = scene->getRenderpass();
        
        auto& attachments = scene->getAttachments();
        const uint8_t numFramebuffersRequired = 6;

        for(size_t i = 0; i < numFramebuffersRequired; ++i)
        {   
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
                case Bolt::AttachmentDefinition::Type::Depth:
                {
                    VkImageView view = attachment->As<ImageAttachment>()->texture.getImageView();
                    views.pushBack(view);
                    break;
                }
                case Bolt::AttachmentDefinition::Type::CubeMap:
                {
                    VkImageView view = attachment->As<CubemapAttachment>()->cubemapViews[i];
                    views.pushBack(view);
                }
                default:
                    assert(false && "Unsupported attachment type");
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

    VkFramebuffer FrameBuffer::getHandle(uint32_t idx)
    {
        VulkanSwapchain* swapchain = VulkanRenderer::get()->getSwapchain();
        
        // If framebuffer has a swapchain attachment, it depends on the swapchain retrieved for presentation.
        // So we need multiple framebuffers
        if(m_dependsOnSwapchain)
        {
            uint8_t index = swapchain->getCurrentSwapchainImageIndex();
            assert(index < m_framebuffers.getSize()); 
            return m_framebuffers[index];
        }
        else
        {
            assert(idx < m_framebuffers.getSize() && "Invalid framebuffer size");
            return m_framebuffers[idx];
        }
    }
}}