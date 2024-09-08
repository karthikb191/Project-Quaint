#ifndef _H_VULKAN_TEXTURE
#define _H_VULKAN_TEXTURE

#include <stdint.h>
#include <Vulkan/vulkan.h>
#include <GFX/Texture/BoltTexture.h>
#include <GFX/Vulkan/Internal/DeviceManager.h>

namespace Bolt
{
    class VulkanRenderer;

    class VulkanTexture : public BoltTextureBase
    {
    public:
        /*Default Texure*/
        VulkanTexture();
        VulkanTexture(const uint32_t width, const uint32_t height);
        VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format);
        VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSharingMode sharingMode);
        VulkanTexture(const VkImageCreateInfo& imageInfo);
        /* Takes ownership of the VkImage passed */
        VulkanTexture(VkImage image);
        ~VulkanTexture();
        void destroy();

        static VulkanTexture create();
        static VulkanTexture* create(Quaint::IMemoryContext* context);
        VulkanTexture& defaultInit();
        VulkanTexture& setWidth(const uint32_t width);
        VulkanTexture& setHeight(const uint32_t height);
        VulkanTexture& setFormat(const VkFormat format);
        VulkanTexture& setUsage(const VkImageUsageFlags usage);
        VulkanTexture& setInitialLayout(const VkImageLayout layout);
        VulkanTexture& setTiling(const VkImageTiling tiling);
        VulkanTexture& setSharingMode(const VkSharingMode sharingMode);
        VulkanTexture& setQueueFamilies(const uint32_t numFamilyIndices, const uint32_t* queueFamilyIndices);
        VulkanTexture& setMemoryProperty(const VkMemoryPropertyFlags flags);
        
        /* Null image handle can be passed. Will be replaced with the image handle of this this object */
        VulkanTexture& setImageViewInfo(const VkImageViewCreateInfo& info);
        VulkanTexture& setIsSwapchainImage(const bool isSwapchainImage);

        /* From must match with the current layout */
        VulkanTexture& build();
        VulkanTexture& createBackingMemory(VkMemoryPropertyFlags propertyFlags);
        VulkanTexture& createBackingMemory();
        VulkanTexture& createImageView();
        void transitionLayout(const VkImageLayout from, const VkImageLayout to);
        void transferOwnership(const EQueueType from, const EQueueType to);

        VkImage* getImageRef() { return &m_image; }
        VkImageView* getImageViewRef() { return &m_imageView; }
        VkImage getHandle() { return m_image; }
        VkImageView getImageView() { return m_imageView; }
        bool isValid() { return m_image != VK_NULL_HANDLE; }
        bool isBacked() { return m_isBacked || getIsSwapchainImage(); }
        uint32_t getWidth() { return m_imageInfo.extent.width; }
        uint32_t getHeight() { return m_imageInfo.extent.height; }
        bool getIsSwapchainImage() { return m_isSwapchainImage; }
        VkAttachmentDescription buildAttachmentDescription(VkImageLayout initialLayout, VkImageLayout finalLayout,
                                                            VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                            VkAttachmentDescriptionFlags flags);
        
        static VkImageCreateInfo getDefaultImageCreateInfo();

    private:
        //void init();
        void createTexture();

        VkImageCreateInfo           m_imageInfo;
        VkImageViewCreateInfo       m_imageViewInfo;
        bool                        m_isCreated = false;
        bool                        m_isBacked = false;
        VkImage                     m_image = VK_NULL_HANDLE;
        VkImageView                 m_imageView = VK_NULL_HANDLE;
        VkDeviceMemory              m_gpuMemory = VK_NULL_HANDLE;
        VkMemoryPropertyFlags       m_memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bool                        m_isSwapchainImage =  false;
    };

    /* Has an Image-View that can be bound */
    class VulkanRenderTexture : public VulkanTexture, BoltRenderTexture
    {
    public:
        VulkanRenderTexture();

        void draw() override;
    };
}

#endif //_H_VULKAN_TEXTURE