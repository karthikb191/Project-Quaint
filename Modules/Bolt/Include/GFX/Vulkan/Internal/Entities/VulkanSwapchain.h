#ifndef _H_VULKAN_SWAPCHAIN
#define _H_VULKAN_SWAPCHAIN
#include <Vulkan/vulkan.h>
#include <Interface/IMemoryContext.h>
#include <Types/QArray.h>

namespace Bolt { namespace vulkan {
    
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(Quaint::IMemoryContext* context);
        ~VulkanSwapchain();
        void construct();
        void destroy();
        void rebuildSwapchain();
        bool hasValidSwapchain() const { return m_valid; }
        uint32_t getNumSwapchainImages() const { return m_swapchainViews.getSize(); }
        VkImageView getSwapchainImageView(uint32_t index) const { return m_swapchainViews[index]; }
        VkExtent2D getSwapchainExtent() const { return m_extent; }
        VkSampleCountFlagBits getSamples() const { return m_samples; }
        VkFormat getFormat() const { return m_format; }
        
    private:
        Quaint::IMemoryContext*         m_context = nullptr;
        VkSwapchainKHR                  m_swapchain = VK_NULL_HANDLE;
        VkSwapchainKHR                  m_oldSwapchain = VK_NULL_HANDLE;
        Quaint::QArray<VkImageView>     m_swapchainViews;
        VkExtent2D                      m_extent = {512, 512};
        bool                            m_valid = false;
        VkFormat                        m_format = VK_FORMAT_R8G8B8A8_SRGB;
        VkSampleCountFlagBits           m_samples = VK_SAMPLE_COUNT_1_BIT;
    };
}}

#endif //_H_VULKAN_SWAPCHAIN