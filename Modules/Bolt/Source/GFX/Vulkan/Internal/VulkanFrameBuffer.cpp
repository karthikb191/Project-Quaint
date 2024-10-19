#include <GFX/Vulkan/Internal/VulkanFrameBuffer.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/RenderScene.h>

namespace Bolt{ namespace vulkan{

    FrameBuffer::FrameBuffer(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_attachments(context)
    {
        m_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    }

    void FrameBuffer::destroy()
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        for(auto& attachment : m_attachments)
        {
            attachment.destroy();
        }
    }

    void FrameBuffer::construct(const RenderScene* scene)
    {
        DeviceManager* dm = VulkanRenderer::get()->getDeviceManager();
        const VkDevice& device = dm->getDeviceDefinition().getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        Quaint::QArray<VkImageView> views(m_context);
        for(auto& attachment : m_attachments)
        {
            //If it's not valid, try creating it
            if(!attachment.isValid())
            {
                attachment.create();
            }

            assert(attachment.isValid() && "Attachment is still not valid even after trying to build it.");
            if(!attachment.isBacked() && !attachment.getIsSwapchainImage())
            {
                attachment.createBackingMemory();
            }
            attachment.createImageView();
            assert(attachment.isBacked() && "Could not back attachment to GPU Memory.");
            views.pushBack(attachment.getImageView());
        }
        m_info.attachmentCount = views.getSize();
        m_info.pAttachments = views.getBuffer();

        ASSERT_SUCCESS(vkCreateFramebuffer(device, &m_info, callbacks, &m_framebuffer), "Failed to create framebuffer!!");
    }

    FrameBuffer& FrameBuffer::addAttachment(const VkImage swapchainImage, const AttachmentInfo* info, const Quaint::QArray<uint32_t>& queueFamilies)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = VK_NULL_HANDLE;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = info->getFormat();
        //components field allows to swizzle color channels around. For eg, you can map all channels to red for a monochromatic view
        viewInfo.components = info->getImageViewComponentMapping();

        //subresource range selects mipmap levels and array layers to be accessible to the view
        viewInfo.subresourceRange = info->getImageViewSubresourceRange();

        VulkanTexture texture(swapchainImage);
        texture.setFormat(info->getFormat())
        .setHeight(info->getExtent().height)
        .setWidth(info->getExtent().width)
        .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
        .setQueueFamilies(queueFamilies.getSize(), queueFamilies.getBuffer())
        .setUsage(info->getImageUsage())
        .setMemoryProperty(info->getMemoryPropertyFlags())
        .setImageViewInfo(viewInfo)
        .setIsSwapchainImage(info->getIsSwapchainImage());

        m_attachments.pushBack(texture);
        return *this;
    }
}}