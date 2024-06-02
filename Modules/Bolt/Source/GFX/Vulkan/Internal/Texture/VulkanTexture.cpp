#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Texture/VulkanTexture.h>
#include <GFX/Vulkan/Internal/DeviceManager.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <QuaintLogger.h>

namespace Bolt
{
    DECLARE_LOG_CATEGORY(VKTEXTURE_LOGGER);
    DEFINE_LOG_CATEGORY(VKTEXTURE_LOGGER);

    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height)
    {
        m_imageInfo = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;

    }
    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format)
    {
        VkImageCreateInfo info = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
        m_imageInfo.format = format;
    }
    VulkanTexture::VulkanTexture(const VkImageCreateInfo& imageInfo)
    {
        m_imageInfo = imageInfo;
        createTexture(m_imageInfo);
    }

    VkImageCreateInfo VulkanTexture::getDefaultImageCreateInfo()
    {
        //TODO: Handle limitations for samples, arrayLayers
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());
        DeviceManager* deviceManager = renderer->getDeviceManager();

        VkImageCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.extent.width = 512;
        info.extent.height = 512;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_LINEAR;
        // Setting default usage to be used by transfer operations
        info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; 
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        uint32_t queues[] = { deviceManager->getDeviceDefinition().getQueueOfType(EQueueType::Transfer).getQueueFamily() };
        info.queueFamilyIndexCount = 1;
        info.pQueueFamilyIndices = queues;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        info.flags = 0;

        return info;
    }

    void VulkanTexture::createTexture(const VkImageCreateInfo& imageInfo)
    {
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());
        DeviceManager* deviceManager = renderer->getDeviceManager();
        VkResult res = vkCreateImage(deviceManager->getDeviceDefinition().getDevice(), &imageInfo, renderer->getAllocationCallbacks(), &m_image);

        assert (res == VK_SUCCESS && "Failed to create texture.");
        if(res != VK_SUCCESS)
        {
            QLOG_E(VKTEXTURE_LOGGER, "Failed to create texture. Texture handle will be null!!!");
            m_image = VK_NULL_HANDLE;
        }
    }

    void VulkanTexture::createBackingMemory()
    {
        assert(!m_isBacked && "Image is already backed");
    }
    void VulkanTexture::transitionLayout(const VkImageLayout from, const VkImageLayout to)
    {
        assert(m_isBacked && "Image is not backed in GPU Memory");
    }
    void VulkanTexture::transferOwnership(const EQueueType from, const EQueueType to)
    {
        assert(m_isBacked && "Image is not backed in GPU Memory");
    }
}