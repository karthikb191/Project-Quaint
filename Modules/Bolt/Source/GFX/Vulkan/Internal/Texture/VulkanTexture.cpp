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

    VkImageViewCreateInfo getDefaultImageViewCreateInfo()
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = VK_NULL_HANDLE;
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
        return createInfo;
    }

    VulkanTexture::VulkanTexture() 
    {
        m_imageViewInfo = getDefaultImageViewCreateInfo();
    }
    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height)
    {
        m_imageViewInfo = getDefaultImageViewCreateInfo();
        m_imageInfo = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
        createTexture();
    }
    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format)
    {
        m_imageViewInfo = getDefaultImageViewCreateInfo();
        m_imageInfo = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
        m_imageInfo.format = format;
        createTexture();
    }
    VulkanTexture::VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSharingMode sharingMode)
    {
        m_imageViewInfo = getDefaultImageViewCreateInfo();
        m_imageInfo = getDefaultImageCreateInfo();
        m_imageInfo.extent.width = width;
        m_imageInfo.extent.height = height;
        m_imageInfo.format = format;
        m_imageInfo.usage = usage;
        m_imageInfo.sharingMode = sharingMode;
        createTexture();
    }
    VulkanTexture::VulkanTexture(const VkImageCreateInfo& imageInfo)
    {
        m_imageViewInfo = getDefaultImageViewCreateInfo();
        m_imageInfo = imageInfo;
        createTexture();
    }
    VulkanTexture::VulkanTexture(VkImage image)
    {
        m_image = image;
    }
    VulkanTexture::~VulkanTexture()
    {
        //TODO: Is it better if this is non-copyable
        //assert(m_imageView == VK_NULL_HANDLE
        //&& m_image == VK_NULL_HANDLE
        //&& m_gpuMemory == VK_NULL_HANDLE && "Texture not destroyed completely");
    }
    
    void VulkanTexture::destroy()
    {
        VkAllocationCallbacks *callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        DeviceManager* deviceManager = VulkanRenderer::get()->getDeviceManager();
        VkDevice device = deviceManager->getDeviceDefinition().getDevice();

        if(m_imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, m_imageView, callbacks);
        }
        if(m_gpuMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, m_gpuMemory, callbacks);
        }
        if(m_image != VK_NULL_HANDLE)
        {
            vkDestroyImage(device, m_image, callbacks);
        }
        m_imageView = VK_NULL_HANDLE;
        m_image = VK_NULL_HANDLE;
        m_gpuMemory = VK_NULL_HANDLE;
    }

    VulkanTexture VulkanTexture::create()
    {
        return VulkanTexture();
    }
    VulkanTexture* VulkanTexture::create(Quaint::IMemoryContext* context)
    {
        return QUAINT_NEW(context, VulkanTexture);
    }
    VulkanTexture& VulkanTexture::defaultInit()
    {
        m_imageInfo = getDefaultImageCreateInfo();
        return *this;
    }
    VulkanTexture& VulkanTexture::setWidth(const uint32_t width)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.extent.width = width;
        return *this;
    }
    VulkanTexture& VulkanTexture::setHeight(const uint32_t height)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.extent.height = height;
        return *this;
    }
    VulkanTexture& VulkanTexture::setFormat(const VkFormat format)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.format = format;
        return *this;
    }
    VulkanTexture& VulkanTexture::setUsage(const VkImageUsageFlags usage)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.usage = usage;
        return *this;
    }
    VulkanTexture& VulkanTexture::setInitialLayout(const VkImageLayout layout)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.initialLayout = layout;
        return *this;
    }
    VulkanTexture& VulkanTexture::setTiling(const VkImageTiling tiling)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.tiling = tiling;
        return *this;
    }
    VulkanTexture& VulkanTexture::setSharingMode(const VkSharingMode sharingMode)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.sharingMode = sharingMode;
        return *this;
    }
    VulkanTexture& VulkanTexture::setQueueFamilies(const uint32_t numFamilyIndices, const uint32_t* queueFamilyIndices)
    {
        assert(!m_isCreated && "Texture already created. Cannot modify texture params once created");
        m_imageInfo.queueFamilyIndexCount = numFamilyIndices;
        m_imageInfo.pQueueFamilyIndices = queueFamilyIndices;
        return *this;
    }
    VulkanTexture& VulkanTexture::setMemoryProperty(const VkMemoryPropertyFlags flags) 
    { 
        m_memoryPropertyFlags = flags; 
        return *this;
    }

    VulkanTexture& VulkanTexture::setImageViewInfo(const VkImageViewCreateInfo& info)
    {
        m_imageViewInfo = info;
        return *this;
    }
    VulkanTexture& VulkanTexture::setIsSwapchainImage(const bool isSwapchainImage)
    {
        m_isSwapchainImage = isSwapchainImage;
        return *this;
    }

    VulkanTexture& VulkanTexture::build()
    {
        //TODO: Add any assert checks here
        createTexture();
        return *this;
    }

    VkImageCreateInfo VulkanTexture::getDefaultImageCreateInfo()
    {
        //TODO: Handle limitations for samples, arrayLayers
        VulkanRenderer* renderer = VulkanRenderer::get();
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

        uint32_t queues[] = { deviceManager->getDeviceDefinition().getQueueOfType(EQueueType::Graphics).getQueueFamily() };
        info.queueFamilyIndexCount = 1;
        info.pQueueFamilyIndices = queues;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        info.flags = 0;

        return info;
    }

    void VulkanTexture::createTexture()
    {
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());
        DeviceManager* deviceManager = renderer->getDeviceManager();
        VkResult res = vkCreateImage(deviceManager->getDeviceDefinition().getDevice(), &m_imageInfo, renderer->getAllocationCallbacks(), &m_image);

        assert (res == VK_SUCCESS && "Failed to create texture.");
        if(res != VK_SUCCESS)
        {
            QLOG_E(VKTEXTURE_LOGGER, "Failed to create texture. Texture handle will be null!!!");
            m_image = VK_NULL_HANDLE;
        }
        m_isCreated = res == VK_SUCCESS;
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

    VulkanTexture& VulkanTexture::createBackingMemory()
    {
        return createBackingMemory(m_memoryPropertyFlags);
    }
    VulkanTexture& VulkanTexture::createBackingMemory(VkMemoryPropertyFlags propertyFlags)
    {
        if(m_isBacked)
        {
            assert(false && "Image is already backed");
            return *this;
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
        
        return *this;
    }

    VulkanTexture& VulkanTexture::createImageView()
    {
        if(m_imageView != VK_NULL_HANDLE)
        {
            assert(false && "Image view is not a null handle");
            return *this;
        }
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());

        m_imageViewInfo.image = m_image;
        VkResult res = vkCreateImageView(renderer->getDeviceManager()->getDeviceDefinition().getDevice()
                                        , &m_imageViewInfo
                                        , renderer->getAllocationCallbacks()
                                        , &m_imageView);
        assert(res == VK_SUCCESS && "Failed to create image view of sample texture");

        return *this;
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