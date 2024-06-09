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

    VulkanTexture::VulkanTexture() {}
    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height)
    {
        m_imageInfo = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
        createTexture(m_imageInfo);
    }
    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format)
    {
        VkImageCreateInfo info = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
        m_imageInfo.format = format;
        createTexture(m_imageInfo);
    }
    VulkanTexture::VulkanTexture(const VkImageCreateInfo& imageInfo)
    {
        m_imageInfo = imageInfo;
        createTexture(m_imageInfo);
    }

    void VulkanTexture::create(const uint32_t width, const uint32_t height)
    {
        m_imageInfo = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
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
        info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 
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

    VkAttachmentDescription VulkanTexture::buildAttachmentDescription(VkImageLayout initialLayout, VkImageLayout finalLayout, 
                                                        VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                        VkAttachmentDescriptionFlags flags)
    {
        VkAttachmentDescription desc{};
        desc.initialLayout = initialLayout;
        desc.finalLayout = finalLayout;
        desc.loadOp = loadOp;
        desc.storeOp = storeOp;
        desc.flags = flags;
        desc.samples = m_imageInfo.samples;
        desc.format = m_imageInfo.format;
        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        return desc;
    }    

    void VulkanTexture::createBackingMemory(VkMemoryPropertyFlags propertyFlags)
    {
        if(m_isBacked)
        {
            assert(false && "Image is already backed");
            return;
        }
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());
        const DeviceDefinition& deviceDefinition = renderer->getDeviceManager()->getDeviceDefinition();

        //TODO: Most of this logic should ideally be a part of a "Memory Manager"
        VkMemoryRequirements memReq{};
        vkGetImageMemoryRequirements(deviceDefinition.getDevice(), m_image, &memReq);

        VkMemoryAllocateInfo memInfo{};
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.allocationSize = memReq.size;
        
        uint32_t memoryTypeIndex;

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(deviceDefinition.getPhysicalDevice(), &memoryProperties);
        bool found = false;
        for(size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if((memReq.memoryTypeBits & (1 << i)) && 
            (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
            {
                memoryTypeIndex = i;
                found = true;
                break;
            }
        }

        assert(found && "Could not find suitable a memory type for device memory allocation");
        memInfo.memoryTypeIndex = memoryTypeIndex;

        VkResult res = vkAllocateMemory(deviceDefinition.getDevice(), &memInfo, renderer->getAllocationCallbacks(), &m_gpuMemory);
        assert(res == VK_SUCCESS && "Could not allocate memory on GPU for the texture image");

        res = vkBindImageMemory(deviceDefinition.getDevice(), m_image, m_gpuMemory, 0);
        assert(res == VK_SUCCESS && "Failed to bind Image to GPU Backing Memory");
    }

    void VulkanTexture::createImageView()
    {
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        //components field allows to swizzle color channels around. For eg, you can map all channels to red for a monochromatic view
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        //subresource range selects mipmap levels and array layers to be accessible to the view
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;

        VkResult res = vkCreateImageView(renderer->getDeviceManager()->getDeviceDefinition().getDevice()
                                        , &createInfo
                                        , renderer->getAllocationCallbacks()
                                        , &m_imageView);
        assert(res == VK_SUCCESS && "Failed to create image view of sample texture");
    }

    void VulkanTexture::transitionLayout(const VkImageLayout from, const VkImageLayout to)
    {
        assert(m_isBacked && "Image is not backed in GPU Memory");
        //TODO:
    }
    void VulkanTexture::transferOwnership(const EQueueType from, const EQueueType to)
    {
        assert(m_isBacked && "Image is not backed in GPU Memory");
        //TODO: 
    }
}