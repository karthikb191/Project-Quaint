#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Entities/VulkanTexture.h>
#include <GFX/Vulkan/Internal/DeviceManager.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <QuaintLogger.h>

namespace Bolt
{
    DECLARE_LOG_CATEGORY(VKTEXTURE_LOGGER);
    DEFINE_LOG_CATEGORY(VKTEXTURE_LOGGER);

// Vulkan Texture Builder ===========================================================

    VulkanImageCreateInfo getDefaultCreateInfo()
    {
        VulkanImageCreateInfo info{};
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.extent.width = 512;
        imageInfo.extent.height = 512;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
        // Setting default usage to be used by transfer operations
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; 
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        //Queue params are effectively ignored
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices = nullptr;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.flags = 0;

        //Default ImageView info
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = VK_NULL_HANDLE;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        //components field allows to swizzle color channels around. For eg, you can map all channels to red for a monochromatic view
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        //subresource range selects mipmap levels and array layers to be accessible to the view
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;

        info.imageInfo = imageInfo;
        info.imageViewInfo = imageViewInfo;
        info.isSwapchainImage = false;
        info.isValid = true;
        info.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        return info;
    }

    VulkanTextureBuilder::VulkanTextureBuilder(Quaint::IMemoryContext* context)
    : m_context(context)
    {
        m_createInfo = getDefaultCreateInfo();
    }
    
    VulkanTextureBuilder& VulkanTextureBuilder::setWidth(const uint32_t width)
    {
        m_createInfo.imageInfo.extent.width = width;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setHeight(const uint32_t height)
    {
        m_createInfo.imageInfo.extent.height = height;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setFormat(const VkFormat format)
    {
        m_createInfo.imageInfo.format = format;
        m_createInfo.imageViewInfo.format = format;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setUsage(const VkImageUsageFlags usage)
    {
        m_createInfo.imageInfo.usage = usage;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setInitialLayout(const VkImageLayout layout)
    {
        m_createInfo.imageInfo.initialLayout = layout;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setTiling(const VkImageTiling tiling)
    {
        m_createInfo.imageInfo.tiling = tiling;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setSharingMode(const VkSharingMode sharingMode)
    {
        m_createInfo.imageInfo.sharingMode = sharingMode;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setQueueFamilies(const uint32_t numFamilyIndices, const uint32_t* queueFamilyIndices)
    {
        m_createInfo.imageInfo.queueFamilyIndexCount = numFamilyIndices;
        m_createInfo.imageInfo.pQueueFamilyIndices = queueFamilyIndices;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setMemoryProperty(const VkMemoryPropertyFlags flags) 
    { 
        m_createInfo.memoryFlags = flags;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setImageCreateInfo(const VkImageCreateInfo& imageInfo)
    {
        m_createInfo.imageInfo = imageInfo;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setLayerCount(uint16_t layers)
    {
        m_createInfo.imageInfo.arrayLayers = layers;
        return *this;
    }
    VulkanTextureBuilder& VulkanTextureBuilder::setMipLevelCount(uint16_t levels)
    {
        m_createInfo.imageInfo.mipLevels = levels;
        return *this;
    }

    VulkanTextureBuilder& VulkanTextureBuilder::setImageViewInfo(const VkImageViewCreateInfo& info)
    {
        m_createInfo.imageViewInfo = info;
        return *this;
    }

    VulkanTextureBuilder& VulkanTextureBuilder::setSwapchainImage(VkImage swapchainImage)
    {
        m_swapchainImage = swapchainImage;
        m_createInfo.isSwapchainImage = true;
        return *this;
    }

    VulkanTexture VulkanTextureBuilder::build()
    {
        if(m_isCubeMap)
        {
            m_createInfo.imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            assert(m_createInfo.imageInfo.arrayLayers % 6 == 0 && "Invalid number of array layers passed to cubemap texture. Should be a multiple of 6");
        }
        else
        {
            if(m_createInfo.imageInfo.arrayLayers > 1)
            {
                m_createInfo.imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            }
        }

        if(m_buildImageView)
        {
            if(m_isCubeMap)
            {
                m_createInfo.imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                m_createInfo.imageViewInfo.subresourceRange.layerCount = m_createInfo.imageInfo.arrayLayers;
                if(m_createInfo.imageInfo.arrayLayers > 6 && m_createInfo.imageInfo.arrayLayers % 6 == 0)
                {
                    m_createInfo.imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                }
            }
            else if(m_createInfo.imageInfo.arrayLayers > 1)
            {
                m_createInfo.imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            }
        }

        //If it's a swapchain image
        if(m_createInfo.isSwapchainImage)
        {
            VulkanTexture texture(m_context, m_createInfo, m_swapchainImage, true);
            if(m_setBackingMemory)
            {
                texture.createBackingMemory();
            }
            if(m_buildImageView)
            {
                if(!texture.isBacked())
                {
                    texture.createBackingMemory();
                }
                texture.createImageView();
            }
            return texture;
        }

        VulkanTexture texture(m_context, m_createInfo);
        if(m_buildImage)
        {
            texture.createImage();
        }
        if(m_setBackingMemory)
        {
            texture.createBackingMemory();
        }
        if(m_buildImageView)
        {
            if(!texture.isBacked())
            {
                texture.createBackingMemory();
            }
            texture.createImageView();
        }
        return texture;
    }

// ==================================================================================

    VulkanTexture::VulkanTexture(Quaint::IMemoryContext* context) 
    : ResourceGPUProxy(context)
    {}
    VulkanTexture::VulkanTexture(Quaint::IMemoryContext* context, const VulkanImageCreateInfo& info)
    : ResourceGPUProxy(context)
    , m_createInfo(info)
    {}
    VulkanTexture::VulkanTexture(Quaint::IMemoryContext* context, const VulkanImageCreateInfo& info, VkImage image, bool isSwapchainImage)
    : ResourceGPUProxy(context)
    , m_createInfo(info)
    , m_isSwapchainImage(isSwapchainImage)
    {
        m_image = image;
    }
    VulkanTexture::~VulkanTexture()
    {
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

    VulkanTexture* VulkanTexture::create(Quaint::IMemoryContext* context)
    {
        return QUAINT_NEW(context, VulkanTexture, context);
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

    void VulkanTexture::createImage()
    {
        assert(m_createInfo.isValid && "Doesn't have valid create info to create texture");
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());
        DeviceManager* deviceManager = renderer->getDeviceManager();
        VkResult res = vkCreateImage(deviceManager->getDeviceDefinition().getDevice(), &m_createInfo.imageInfo, renderer->getAllocationCallbacks(), &m_image);

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
        desc.samples = m_createInfo.imageInfo.samples;
        desc.format = m_createInfo.imageInfo.format;
        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        return desc;
    }    

    void VulkanTexture::createBackingMemory()
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
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            VkMemoryPropertyFlags propertyFlags = m_createInfo.memoryFlags;
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
        m_isBacked = res == VK_SUCCESS;
    }

    void VulkanTexture::createImageView()
    {
        if(m_imageView != VK_NULL_HANDLE || m_image == VK_NULL_HANDLE)
        {
            assert(false && "Image view is not a null handle");
            return;
        }
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Bolt::RenderModule::get().getBoltRenderer()->GetRenderer());

        VkImageViewCreateInfo info = m_createInfo.imageViewInfo;
        info.image = m_image;
        VkResult res = vkCreateImageView(renderer->getDeviceManager()->getDeviceDefinition().getDevice()
                                        , &info
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