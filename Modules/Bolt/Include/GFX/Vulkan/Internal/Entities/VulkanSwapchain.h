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
        bool hasValidSwapchain() { return m_valid; }
        uint32_t getNumSwapchainImages() { return m_swapchainViews.getSize(); }
        VkImageView getSwapchainImageView(uint32_t index) { return m_swapchainViews[index]; }
        VkExtent2D getSwapchainExtent() { return m_extent; }
        
    private:
        Quaint::IMemoryContext*         m_context = nullptr;
        VkSwapchainKHR                  m_swapchain = VK_NULL_HANDLE;
        VkSwapchainKHR                  m_oldSwapchain = VK_NULL_HANDLE;
        Quaint::QArray<VkImageView>     m_swapchainViews;
        VkExtent2D                      m_extent = {512, 512};
        bool                            m_valid = false;
    };
}}

#endif //_H_VULKAN_SWAPCHAIN