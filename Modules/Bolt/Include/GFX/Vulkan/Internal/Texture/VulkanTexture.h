#ifndef _H_VULKAN_TEXTURE
#define _H_VULKAN_TEXTURE

#include <stdint.h>
#include <Vulkan/vulkan.h>
#include <GFX/Texture/BoltTexture.h>

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
        ~VulkanTexture();
        void destroy();

        static VulkanTexture create();
        static VulkanTexture* create(Quaint::IMemoryContext* context);
        VulkanTexture& defaultInit();
        VulkanTexture& setWidth(const uint32_t width);
        VulkanTexture& setHeight(const uint32_t height);
        VulkanTexture& setFormat(const VkFormat format);
        VulkanTexture& setUsage(const VkImageUsageFlagBits usage);
        VulkanTexture& setSharingMode(const VkSharingMode sharingMode);
        VulkanTexture& setQueueFamilies(const uint32_t numFamilyIndices, const uint32_t* queueFamilyIndices);


        /* From must match with the current layout */
        VulkanTexture& build();
        VulkanTexture& createBackingMemory(VkMemoryPropertyFlags propertyFlags);
        VulkanTexture& createImageView();
        void transitionLayout(const VkImageLayout from, const VkImageLayout to);
        void transferOwnership(const EQueueType from, const EQueueType to);

        VkImage* getImageRef() { return &m_image; }
        VkImageView* getImageViewRef() { return &m_imageView; }
        bool isValid() { return m_image != VK_NULL_HANDLE; }
        bool isBacked() { return m_isBacked; }
        uint32_t getWidth() { return m_imageInfo.extent.width; }
        uint32_t getHeight() { return m_imageInfo.extent.height; }
        VkAttachmentDescription buildAttachmentDescription(VkImageLayout initialLayout, VkImageLayout finalLayout,
                                                            VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                            VkAttachmentDescriptionFlags flags);
        
        static VkImageCreateInfo getDefaultImageCreateInfo();

    private:
        //void init();
        void createTexture();

        VkImageCreateInfo           m_imageInfo;
        bool                        m_isCreated = false;
        bool                        m_isBacked = false;
        VkImage                     m_image = VK_NULL_HANDLE;
        VkImageView                 m_imageView = VK_NULL_HANDLE;
        VkDeviceMemory              m_gpuMemory = VK_NULL_HANDLE;
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