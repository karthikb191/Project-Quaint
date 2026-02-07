#ifndef _H_VULKAN_TEXTURE
#define _H_VULKAN_TEXTURE

#include <stdint.h>
#include <Vulkan/vulkan.h>
#include <GFX/Texture/BoltTexture.h>
#include <GFX/Vulkan/Internal/DeviceManager.h>
#include <GFX/Interface/IRenderer.h>
#include <Types/QUniquePtr.h>

namespace Bolt { 
    class VulkanRenderer;
    
    namespace vulkan {
    class VulkanTexture;
    class VulkanTextureBuilder;

    struct VulkanImageCreateInfo
    {
        VkImageCreateInfo               imageInfo = {};
        VkImageViewCreateInfo           imageViewInfo = {};
        bool                            isSwapchainImage = false;
        VkMemoryPropertyFlags           memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bool                            isValid = false;
    };

    /* Copy-able entity */
    class VulkanTexture : public Bolt::ResourceGPUProxy
    {
        friend class VulkanTextureBuilder;

    public:
        /* Default Texure */
        VulkanTexture(Quaint::IMemoryContext* context);
        VulkanTexture(Quaint::IMemoryContext* context, const VulkanImageCreateInfo& info);
        /* Takes ownership of the VkImage passed */
        VulkanTexture(Quaint::IMemoryContext* context, const VulkanImageCreateInfo& info, VkImage image, bool isSwapchainImage = false);
        ~VulkanTexture();

        // TODO: Delete copy, move and assignment.
        // VulkanTexture is only supposed to be passed by reference to adhere to RAII principles.
        // Becomes extremely hard to track down the original owner of the resource otherwise

        void destroy();

        static VulkanTexture* create(Quaint::IMemoryContext* context);
        VulkanTexture& defaultInit();
        
        /* From must match with the current layout */
        void createImage();
        void createBackingMemory();
        void createImageView();
        void transitionLayout(const VkImageLayout from, const VkImageLayout to);
        void transferOwnership(const EQueueType from, const EQueueType to);

        const VkImage* getImageRef() const { return &m_image; }
        const VkImageView* getImageViewRef() const { return &m_imageView; }
        const VkImage getHandle() const { return m_image; }
        const VkImageView getImageView() const { return m_imageView; }
        const bool isValid() const { return m_image != VK_NULL_HANDLE; }
        const bool isBacked() const { return m_isBacked || getIsSwapchainImage(); }
        const uint32_t getWidth() const { return m_createInfo.imageInfo.extent.width; }
        const uint32_t getHeight() const { return m_createInfo.imageInfo.extent.height; }
        const VulkanImageCreateInfo& getCreateInfo() const { return m_createInfo; }
        const bool getIsSwapchainImage() const { return m_isSwapchainImage; }
        const uint16_t getLayerCount() const { return m_layerCount; }
        const uint16_t getMipCount() const { return m_mipCount; }
        VkAttachmentDescription buildAttachmentDescription(VkImageLayout initialLayout, VkImageLayout finalLayout,
                                                            VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                            VkAttachmentDescriptionFlags flags);
        
        static VkImageCreateInfo getDefaultImageCreateInfo();

    private:
        //void init();
        
        void resetImage(VkImage image);

        VulkanImageCreateInfo       m_createInfo = {};
        bool                        m_isCreated = false;
        bool                        m_isBacked = false;
        VkImage                     m_image = VK_NULL_HANDLE;
        VkImageView                 m_imageView = VK_NULL_HANDLE;
        VkDeviceMemory              m_gpuMemory = VK_NULL_HANDLE;
        VkMemoryPropertyFlags       m_memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageLayout               m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        bool                        m_isSwapchainImage =  false;
        bool                        m_isCubemap = false;
        uint16_t                    m_layerCount = 1;
        uint16_t                    m_mipCount = 1;
    };
    using VulkanTextureRef = Quaint::QUniquePtr<VulkanTexture, Quaint::Deleter<VulkanTexture>>;
    
    // This should probably depend on a different builder that's platform agnostic.
    // Very convoluted and doesn't really serve any real purpose. Easier to just populate the image and view structs directly
    class VulkanTextureBuilder
    {
    public:
        VulkanTextureBuilder(Quaint::IMemoryContext* context);
        VulkanTextureBuilder& setWidth(const uint32_t width);
        VulkanTextureBuilder& setHeight(const uint32_t height);
        VulkanTextureBuilder& setFormat(const VkFormat format);
        VulkanTextureBuilder& setUsage(const VkImageUsageFlags usage);
        VulkanTextureBuilder& setInitialLayout(const VkImageLayout layout);
        VulkanTextureBuilder& setTiling(const VkImageTiling tiling);
        VulkanTextureBuilder& setSharingMode(const VkSharingMode sharingMode);
        VulkanTextureBuilder& setQueueFamilies(const uint32_t numFamilyIndices, const uint32_t* queueFamilyIndices);
        VulkanTextureBuilder& setMemoryProperty(const VkMemoryPropertyFlags flags);
        
        /* Null image handle can be passed. Will be replaced with the image handle of this this object */
        VulkanTextureBuilder& setSwapchainImage(VkImage swapchainImage);
        VulkanTextureBuilder& setBuildImage() { m_buildImage = true; return *this;}
        VulkanTextureBuilder& setBackingMemory() { m_setBackingMemory = true; return *this;}
        VulkanTextureBuilder& setBuildImageView() { m_buildImageView = true; return *this;}
        VulkanTextureBuilder& setIsCubeMap() { m_isCubeMap = true; return *this; }
        VulkanTextureBuilder& setLayerCount(uint16_t layers);
        VulkanTextureBuilder& setMipLevelCount(uint16_t layers);

        VulkanTextureBuilder& setImageCreateInfo(const VkImageCreateInfo& imageInfo);
        VulkanTextureBuilder& setImageViewInfo(const VkImageViewCreateInfo& info);
        VulkanTexture build();

        // VkImageCreateInfo getDefaultCreateInfoForShaderTexture();
        // VkImageCreateInfo getDefaultCreateInfoForShaderCubemapTexture();
        // VkImageCreateInfo getDefaultImageViewInfoForShaderTexture();

    private:
        Quaint::IMemoryContext*         m_context = nullptr;
        VulkanImageCreateInfo           m_createInfo{};
        VkImage                         m_swapchainImage = VK_NULL_HANDLE;
        bool                            m_buildImage = false;
        bool                            m_setBackingMemory = false;
        bool                            m_buildImageView = false; 
        bool                            m_isCubeMap = false;
    };

    /* Has an Image-View that can be bound */
    class VulkanRenderTexture : public VulkanTexture, BoltRenderTexture
    {
    public:
        VulkanRenderTexture();

        void draw() override;
    };
}}

#endif //_H_VULKAN_TEXTURE