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
        /*Default Texure*/
        VulkanTexture(const uint32_t width, const uint32_t height);
        VulkanTexture(const uint32_t width, const uint32_t height, VkFormat format);
        VulkanTexture(const VkImageCreateInfo& imageInfo);

        /* From must match with the current layout */
        void createBackingMemory();
        void transitionLayout(const VkImageLayout from, const VkImageLayout to);
        void transferOwnership(const EQueueType from, const EQueueType to);

        static VkImageCreateInfo getDefaultImageCreateInfo();
        VkImage getImage() { return m_image; }
        bool isValid() { return m_image != VK_NULL_HANDLE; }
        bool isBacked() { return m_isBacked; }
    private:
        //void init();
        void createTexture(const VkImageCreateInfo& imageInfo);

        VkImage                     m_image = VK_NULL_HANDLE;
        VkImageCreateInfo           m_imageInfo;
        bool                        m_isBacked = false;
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