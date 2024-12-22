#include <GFX/Vulkan/Internal/Entities/VulkanSwapchain.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Interface/IWindow_Impl.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <LoggerModule.h>

namespace Bolt { namespace vulkan {

    DECLARE_LOG_CATEGORY(VK_SWAPCHAIN);
    DEFINE_LOG_CATEGORY(VK_SWAPCHAIN);

    uint32_t getMinSwapchainImageCount()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VkSurfaceKHR surface = VulkanRenderer::get()->getSurface();
        VkPhysicalDevice phyDevice = VulkanRenderer::get()->getPhysicalDevice();

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, surface, &capabilities);
        
        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }
        return imageCount;
    }

    VkSurfaceFormatKHR getSwapchainFormat()
    {
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VkSurfaceKHR surface = VulkanRenderer::get()->getSurface();
        VkPhysicalDevice phyDevice = VulkanRenderer::get()->getPhysicalDevice();
        Quaint::IMemoryContext* context = VulkanRenderer::get()->getMemoryContext();

        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &surfaceFormatCount, nullptr);
        Quaint::QArray<VkSurfaceFormatKHR> surfaceFormats(context);
        surfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &surfaceFormatCount, surfaceFormats.getBuffer_NonConst());

        for(auto& format : surfaceFormats)
        {
            if(format.format == VK_FORMAT_R8G8B8A8_UNORM
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }
        assert(false && "Required format is not supposed by physical device");
        return VkSurfaceFormatKHR{};
    }

    VkExtent2D getSwapchainExtent()
    {
        VkSurfaceKHR surface = VulkanRenderer::get()->getSurface();
        VkPhysicalDevice phyDevice = VulkanRenderer::get()->getPhysicalDevice();
        
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, surface, &capabilities);
        
        //UINT_MAX is a special value indicating the extent of surface is determined by the extent of swapchain
        if(capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            //TODO: This need to be handled in a better way
            const Bolt::IWindow_Impl_Win* window = Bolt::RenderModule::get().getBoltRenderer()->getWindow().getWindowsWindow();
            RECT rect;
            GetWindowRect(window->getWindowHandle(), &rect);
            uint32_t width = rect.right - rect.left;
            uint32_t height = rect.bottom - rect.top;

            VkExtent2D actualExtent = { width, height };
            actualExtent.width = width < capabilities.minImageExtent.width 
            ? capabilities.minImageExtent.width : width;
            actualExtent.width = width > capabilities.maxImageExtent.width
            ? capabilities.maxImageExtent.width : width;

            actualExtent.height = height < capabilities.minImageExtent.height 
            ? capabilities.minImageExtent.height : height;
            actualExtent.height = height > capabilities.maxImageExtent.height
            ? capabilities.maxImageExtent.height : height;
            
            return actualExtent;
        }
    }

    VkPresentModeKHR getPresentMode()
    {
        return VK_PRESENT_MODE_FIFO_KHR;
        //TODO: Experiment with different presentation modes
    }


    VulkanSwapchain::VulkanSwapchain(Quaint::IMemoryContext* context)
    : m_swapchainViews(context)
    {}

    VulkanSwapchain::~VulkanSwapchain()
    {
        destroy();
    }

    void VulkanSwapchain::construct()
    {
        if(m_valid)
        {
            QLOG_E(VK_SWAPCHAIN, "Swapchain already constructed. Use rebuild or destroy old swapchain before trying to create another one");
            return;
        }

        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        VkSurfaceKHR surface = VulkanRenderer::get()->getSurface();

        VkSurfaceFormatKHR format = getSwapchainFormat();

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.pNext = nullptr;
        info.flags = 0;
        info.surface = surface;
        info.minImageCount = getMinSwapchainImageCount();
        info.imageFormat = format.format;
        info.imageColorSpace = format.colorSpace;
        info.imageExtent = getSwapchainExtent();
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        //TODO: These should be handled if the queues are exclusive for multiple operations 
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;

        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // TODO: Check if this is giving any issues
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //This specifies if alpha channel should be used for blending with other windows
        info.presentMode = getPresentMode();
        info.clipped = VK_TRUE;
        info.oldSwapchain = m_oldSwapchain; //This will eventually have a null handle 
        VkResult res = vkCreateSwapchainKHR(device, &info, callbacks, &m_swapchain);
        ASSERT_SUCCESS(res, "Failed to create swapchain");

        //Access swapchain to create it's image views
        uint32_t swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(device, m_swapchain, &swapchainImageCount, nullptr);
        assert(swapchainImageCount > 0 && "No valid swapchain images available for presentation");
        Quaint::QArray<VkImage> swapchainImages(m_context);
        m_swapchainViews.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(device, m_swapchain, &swapchainImageCount, swapchainImages.getBuffer_NonConst());

        //TODO: See if it makes sense to use VulkanTexture here
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = nullptr;
        viewInfo.flags = 0;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        //subresource range selects mipmap levels and array layers to be accessible to the view
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;

        for(size_t i = 0; i < swapchainImageCount; ++i)
        {
            viewInfo.image = swapchainImages[i];
            res = vkCreateImageView(device, &viewInfo, callbacks, m_swapchainViews.getBuffer_NonConst() + i);
            ASSERT_SUCCESS(res, "Could not create swapchain image view");
        }

        m_extent = info.imageExtent;
        m_valid = true;
        m_format = format.format;
    }

    void VulkanSwapchain::destroy()
    {
        if(!hasValidSwapchain())
        {
            return;
        }

        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();
        for(auto& view : m_swapchainViews)
        {
            vkDestroyImageView(device, view, callbacks);
        }
        vkDestroySwapchainKHR(device, m_swapchain, callbacks);
        m_swapchain = VK_NULL_HANDLE;
        m_valid = false;
    }

    void VulkanSwapchain::rebuildSwapchain()
    {
        if(!m_valid)
        {
            QLOG_E(VK_SWAPCHAIN, "Need a valid swapchain first before trying to rebuild it");
            return;
        }
        m_valid = false;
        VkDevice device = VulkanRenderer::get()->getDevice();
        VkAllocationCallbacks* callbacks = VulkanRenderer::get()->getAllocationCallbacks();

        vkDeviceWaitIdle(device);
        m_oldSwapchain = m_swapchain;
        construct();
        if(m_oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, m_oldSwapchain, callbacks);
            m_oldSwapchain = VK_NULL_HANDLE;
        }
    }
}}