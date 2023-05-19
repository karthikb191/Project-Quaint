#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QFastArray.h>
#include <Types/QStaticString.h>
#include <QuaintLogger.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <fstream>

//TODO: Remove this once there's a custom version available
#include <set>

namespace Bolt
{
    #define VULKAN_RENDERER_LOGGER
    DECLARE_LOG_CATEGORY(VULKAN_RENDERER_LOGGER);
    DEFINE_LOG_CATEGORY(VULKAN_RENDERER_LOGGER);

    auto validationLayers = Quaint::createFastArray<const char*>(
        "VK_LAYER_KHRONOS_validation"
    );

    auto deviceExtensions = Quaint::createFastArray<const char*>(
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    );

    auto dynamicStates = Quaint::createFastArray<VkDynamicState>(
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    );
    
    #define VALIDATION_LAYER_TYPE decltype(validationLayers)

    VulkanRenderer::~VulkanRenderer()
    {
    }
    VulkanRenderer::VulkanRenderer(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_defGraphicsAllocator {VK_NULL_HANDLE}
    , m_instance {VK_NULL_HANDLE}
    , m_swapchainImages(context)
    , m_swapchainImageViews(context)
    {
    }

    void VulkanRenderer::init()
    {
        if(m_context != nullptr)
        {
            createAllocationCallbacks();
        }
        //Vulkan Instance creation
        createInstance();
#ifdef DEBUG_BUILD
        setupDebugMessenger();
#endif
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        createRenderPipeline();

        m_running = true;
    }

    void VulkanRenderer::shutdown()
    {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, &m_defGraphicsAllocator);
        for(const VkImageView& view : m_swapchainImageViews)
        {
            vkDestroyImageView(m_device, view, &m_defGraphicsAllocator);
        }
        vkDestroySwapchainKHR(m_device, m_swapchain, &m_defGraphicsAllocator);
        vkDestroyDevice(m_device, &m_defGraphicsAllocator);
        vkDestroySurfaceKHR(m_instance, m_surface, &m_defGraphicsAllocator);
#ifdef DEBUG_BUILD
        destroyDebugMessenger();
#endif
        vkDestroyInstance(m_instance, &m_defGraphicsAllocator);
    }
    
    //Custom Allocation code uses memory context ....................................
    //These are just memory allocation functions. These don't have to call constructors and destructors.
    void* VKAPI_PTR VulkanRenderer::allocationFunction(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        Quaint::IMemoryContext* context = static_cast<Quaint::IMemoryContext*>(pUserData);
        assert(context && "Invalid Context received in allocation function");

        return QUAINT_ALLOC_MEMORY_ALIGNED(context, size, alignment);
    }
    void VKAPI_PTR VulkanRenderer::freeFunction(void* pUserData, void* pMemory)
    {
        Quaint::IMemoryContext* context = static_cast<Quaint::IMemoryContext*>(pUserData);
        assert(context && "Invalid Context received in free function");

        QUAINT_DEALLOC_MEMORY(context, pMemory);
    }
    void* VKAPI_PTR VulkanRenderer::reallocFunction(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        Quaint::IMemoryContext* context = static_cast<Quaint::IMemoryContext*>(pUserData);
        assert(context && "Invalid Context received in realloc function");

        QUAINT_DEALLOC_MEMORY(context, pOriginal);
        return QUAINT_ALLOC_MEMORY_ALIGNED(context, size, alignment);
    }

    void VKAPI_PTR VulkanRenderer::internalAllocationFunction(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
    {
        Quaint::IMemoryContext* context = static_cast<Quaint::IMemoryContext*>(pUserData);
        assert(context && "Invalid Context received in allocation function");

        //return QUAINT_ALLOC_MEMORY(context, size);
    }
    void VKAPI_PTR VulkanRenderer::internalFreeFunction(void* pUserData, void* pMemory, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
    {
        Quaint::IMemoryContext* context = static_cast<Quaint::IMemoryContext*>(pUserData);
        assert(context && "Invalid Context received in free function");

        QUAINT_DEALLOC_MEMORY(context, pMemory);
    }

    void VulkanRenderer::createAllocationCallbacks()
    {
        if(m_context == nullptr)
        {
            //Not fatal. Application just doesn't use custom memory management if it's available
            QLOG_I(VULKAN_RENDERER_LOGGER, "Null context passed. Allocation callback structure creation failed! Using defaults");
            return;
        }
        m_defGraphicsAllocator.pUserData = m_context;
        m_defGraphicsAllocator.pfnAllocation = VulkanRenderer::allocationFunction;
        m_defGraphicsAllocator.pfnFree = VulkanRenderer::freeFunction;
        m_defGraphicsAllocator.pfnReallocation = VulkanRenderer::reallocFunction;
    }
    //............................................................................

    bool areAllValidationLayersAvailable(Quaint::IMemoryContext* context, const VALIDATION_LAYER_TYPE& validationLayers)
    {
        size_t availableLayers = 0;
        vkEnumerateInstanceLayerProperties(&availableLayers, nullptr);
        Quaint::QArray<VkLayerProperties> layerProperties(context, availableLayers);
        vkEnumerateInstanceLayerProperties(&availableLayers, layerProperties.getBuffer_NonConst());

        bool allFound = true;
        for(const char* layerName : validationLayers)
        {
            bool found = false;
            for(const auto& property : layerProperties)
            {
                if(strcmp(layerName, property.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                allFound = false;
                break;
            }
        }

        return allFound;
    }

    void VulkanRenderer::createInstance()
    {
        //TODO: Probably get this info from an xml or json
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Bolt Renderer";
        appInfo.pEngineName = "Quaint Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;

        //Possible Windows Extensions.
        //TODO: Create a platform agnostic implementation
        auto instanceExtensions = Quaint::createFastArray(
        {
            "VK_KHR_device_group_creation"// REQUIRED! This extension provides instance-level commands to enumerate groups of physical devices, and to create a logical device from a subset of one of those groups
            , "VK_KHR_external_fence_capabilities" //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” fence handles an implementation supports for a given set of use cases
            , "VK_KHR_external_memory_capabilities" //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” memory handles an implementation supports for a given set of use cases
            //, "VK_KHR_external_semaphore_capabilities", //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” semaphore handles an implementation supports for a given set of use cases
            , "VK_KHR_get_physical_device_properties2" //REQUIRED! This extension provides new entry points to query device features, device properties, and format properties in a way that can be easily extended by other extensions, without introducing any further entry points
            , "VK_KHR_get_surface_capabilities2" //REQUIRED! Provides an entry point to query device surface capabilities
            , "VK_KHR_surface" //REQUIRED! Abstracts native platform surfaces for use with Vulkan. Provides a way to determine whether queue family in a device supports presenting to a surface   
            //, "VK_KHR_surface_protected_capabilities", //??? This extension extends VkSurfaceCapabilities2KHR, providing applications a way to query whether swapchains can be created with the VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR flag set
            , "VK_KHR_win32_surface"  //REQUIRED! Required for rendering to windows. Provided mechanism to create "VkSurfaceKHR" object 
        #ifdef DEBUG_BUILD
            , "VK_EXT_debug_report" //OPTIONAL! Enabled detailed debug reports
            , VK_EXT_DEBUG_UTILS_EXTENSION_NAME //OPTIONAL! Enables support of passing a callback to handle debug messages and much more
        #endif
            //, "VK_EXT_swapchain_colorspace", //??? Might be needed. Not much information available
            //, "VK_NV_external_memory_capabilities", //??? Applications may wish to import memory from the Direct 3D API, or export memory to other Vulkan instances. This extension provides a set of capability queries that allow applications determine what types of win32 memory handles an implementation supports for a given set of use cases.
            //, "VK_KHR_portability_enumeration", //OPTIONAL! This extension allows applications to control whether devices that expose the VK_KHR_portability_subset extension are included in the results of physical device enumeration.
        });

        instanceInfo.enabledExtensionCount = instanceExtensions.getSize();
        instanceInfo.ppEnabledExtensionNames = instanceExtensions.getBuffer();

        //Uncomment to print available instances
        //uint32_t extensionsCount = 0;
        //vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        //Quaint::QArray<VkExtensionProperties> extensions(m_context, extensionsCount);
        //vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.getBuffer_NonConst());
        //for(size_t i = 0; i < extensionsCount; i++)
        //{
        //    QLOG_I(VULKAN_RENDERER_LOGGER, extensions[i].extensionName);
        //}

        //Validation layers are debug only feature
#ifdef DEBUG_BUILD
        
        if(!areAllValidationLayersAvailable(m_context, validationLayers))
        {
            QLOG_E(VULKAN_RENDERER_LOGGER, "[-] Some/All of validation layers provided are not available. This will fail isntance creation. Bailing out");
            return;
        }
        instanceInfo.enabledLayerCount = validationLayers.getSize();
        instanceInfo.ppEnabledLayerNames = validationLayers.getBuffer();
#else
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = nullptr;
#endif

        VkResult result = vkCreateInstance(&instanceInfo, &m_defGraphicsAllocator, &m_instance);
        if(result != VK_SUCCESS)
        {
            QLOG_E(VULKAN_RENDERER_LOGGER, "FATAL ERROR! Failed to create a Vulkan Instance");
        }
        else
        {
            QLOG_I(VULKAN_RENDERER_LOGGER, "[+] Vulkan Instance creation successful");
        }
    }

    void VulkanRenderer::createSurface()
    {
        //TODO: Surround with plat-spec macro
        createWindowsSurface();
    }
    void VulkanRenderer::createWindowsSurface()
    {
        const IWindow_Impl_Win* window = RenderModule::get().getBoltRenderer()->getWindow().getWindowsWindow();
        assert(window != nullptr && "Invalid window retrieved");

         VkWin32SurfaceCreateInfoKHR winSurfaceInfo{};
         winSurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
         winSurfaceInfo.hwnd = window->getWindowHandle();
         winSurfaceInfo.hinstance = window->getHInstance();

         VkResult res = vkCreateWin32SurfaceKHR(m_instance, &winSurfaceInfo, &m_defGraphicsAllocator, &m_surface);
         assert(res == VK_SUCCESS && "Failed to create a windows surface");
         QLOG_I(VULKAN_RENDERER_LOGGER, "[+] Windows Surface Creation successful");
    }

//Physical and Logical device creation and corresponding queries -------------------------------
    void getQueueFamilies(Quaint::IMemoryContext* context, const VkPhysicalDevice& device, const VkSurfaceKHR& surface, VulkanRenderer::QueueFamilies& families)
    {
        uint32_t propCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &propCount, nullptr);

        Quaint::QArray<VkQueueFamilyProperties> properties(context, propCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &propCount, properties.getBuffer_NonConst());
        
        for(size_t i = 0; i < propCount; i++)
        {
            if(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                families.graphics.set(i);
            }

            //Check if a
            VkBool32 supported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
            if(supported)
            {
                families.presentation.set(i);
            }

            if(families.allSet()) break;
        }
    }

    VulkanRenderer::SwapchainSupportInfo querySwapchainSupport(Quaint::IMemoryContext* context, const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
    {
        VulkanRenderer::SwapchainSupportInfo supportInfo(context);

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &supportInfo.surfaceCapabilities);

        uint32_t surfaceFormatsCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatsCount, nullptr);
        if(surfaceFormatsCount != 0)
        {
            supportInfo.surfaceFormat.resize(surfaceFormatsCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatsCount, supportInfo.surfaceFormat.getBuffer_NonConst());
        }

        uint32_t presentModesCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
        if(presentModesCount != 0)
        {
            supportInfo.presentMode.resize(presentModesCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, supportInfo.presentMode.getBuffer_NonConst());
        }

        return supportInfo;
    }

    size_t getDeviceSuitability(Quaint::IMemoryContext* context, const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
    {
        size_t score = 0;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceProperties(device, &properties);

        //Check if device extensions are supported on current device
        uint32_t extensionsCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
        Quaint::QArray<VkExtensionProperties> extensionProperties(context, extensionsCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.getBuffer_NonConst());

        bool allExtensionsAvailable = true;
        for(const char* extension : deviceExtensions)
        {
            bool found = false;
            for(const VkExtensionProperties& property : extensionProperties)
            {
                if(strcmp(extension, property.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if(!found) allExtensionsAvailable = false;
        }

        //Application can't function if all device extensions are not available on selected device
        if(!allExtensionsAvailable) return 0;

        //Check if device contains a suitable swapchain
        //Application can't function without an adequate swapchain
        bool containsSuitableSwapchain = false;
        VulkanRenderer::SwapchainSupportInfo supportInfo = querySwapchainSupport(context, device, surface);
        containsSuitableSwapchain = !supportInfo.surfaceFormat.isEmpty() && !supportInfo.presentMode.isEmpty();
        if(!containsSuitableSwapchain) return 0;

        VulkanRenderer::QueueFamilies families;
        getQueueFamilies(context, device, surface, families);
        
        //Application can't function without all the supported queues
        if(!families.allSet()) return 0;
        //Application can't function without geometry shader
        if(!features.geometryShader) return 0;

        if(features.tessellationShader) score += 100;
        if(features.imageCubeArray) score += 100;
        if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 100;

        //score += properties.limits.maxImageDimension2D;
        //score += properties.limits.maxImageDimension3D;

        return score;
    }

    void VulkanRenderer::selectPhysicalDevice()
    {
        uint32_t numDevices = 0;
        vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);
        assert(numDevices > 0 && "Vulkan could not retrieve any physical devices");

        Quaint::QArray<VkPhysicalDevice> physicalDevices(m_context, numDevices);
        vkEnumeratePhysicalDevices(m_instance, &numDevices, physicalDevices.getBuffer_NonConst());

        size_t score = 0;
        for(const VkPhysicalDevice& device : physicalDevices)
        {
            size_t deviceScore = getDeviceSuitability(m_context, device, m_surface);
            if(deviceScore && deviceScore > score)
            {
                score = deviceScore;
                m_physicalDevice = device;
            }
        }

        assert(m_physicalDevice != VK_NULL_HANDLE && "Could not retrieve a valid physical device");
    }

    void VulkanRenderer::createLogicalDevice()
    {
        QueueFamilies queueFamilies;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, queueFamilies);

        //TODO: Replace with a custom set
        std::set<uint32_t> queueIndices { queueFamilies.graphics.get(), queueFamilies.presentation.get() };

        float priority = 1.0f;
        Quaint::QArray<VkDeviceQueueCreateInfo> queues(m_context);

        for(uint32_t index : queueIndices)
        {
            //Device queue create structure. Describes the number of queues we want for a single queue family        
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = index;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            
            queues.pushBack(queueInfo);
        }

        //TODO: TO Be filled later
        VkPhysicalDeviceFeatures deviceFeatures{};


        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = queues.getSize();
        deviceInfo.pQueueCreateInfos = queues.getBuffer();
        deviceInfo.pEnabledFeatures = &deviceFeatures;
        //Enables use of swapchain extension on the device
        deviceInfo.enabledExtensionCount = deviceExtensions.getSize();
        deviceInfo.ppEnabledExtensionNames = deviceExtensions.getBuffer();

#ifdef DEBUG_BUILD
        deviceInfo.enabledLayerCount = 1;
        deviceInfo.ppEnabledLayerNames = validationLayers.getBuffer();
#else
        deviceInfo.enabledLayerCount = 0;
        deviceInfo.ppEnabledLayerNames = nullptr;
#endif

        VkResult res = vkCreateDevice(m_physicalDevice, &deviceInfo, &m_defGraphicsAllocator, &m_device);
        assert(res == VK_SUCCESS && "Logical device creation failed!");
        
        //Once Logical device is created, retrieve queues to interface with
        vkGetDeviceQueue(m_device, queueFamilies.graphics.get(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, queueFamilies.presentation.get(), 0, &m_presentQueue);
    }
//--------------------------------------------------------------------

//----------------------------Swapchain Creation
    VkSurfaceFormatKHR chooseSurfaceFormat(Quaint::IMemoryContext* context, VulkanRenderer::SwapchainSupportInfo& supportInfo)
    {
        for (const VkSurfaceFormatKHR& availableFormat : supportInfo.surfaceFormat) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return supportInfo.surfaceFormat[0];
    }
    VkPresentModeKHR choosePresentationMode(Quaint::IMemoryContext* context, VulkanRenderer::SwapchainSupportInfo& supportInfo)
    {
        //TODO: use VK_PRESENT_MODE_MAILBOX_KHR on windows if available
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D chooseSwapExtent(Quaint::IMemoryContext* context, VulkanRenderer::SwapchainSupportInfo& supportInfo)
    {
        if(supportInfo.surfaceCapabilities.currentExtent.width != UINT32_MAX)
        {
            return supportInfo.surfaceCapabilities.currentExtent;
        }
        else
        {
            //TODO: find a better way to retrieve pixel width/height
            const Bolt::IWindow_Impl_Win* window = Bolt::RenderModule::get().getBoltRenderer()->getWindow().getWindowsWindow();
            RECT rect;
            GetWindowRect(window->getWindowHandle(), &rect);
            uint32_t width = rect.right - rect.left;
            uint32_t height = rect.bottom - rect.top;

            VkExtent2D actualExtent = { width, height };
            actualExtent.width = width < supportInfo.surfaceCapabilities.minImageExtent.width 
            ? supportInfo.surfaceCapabilities.minImageExtent.width : width;
            actualExtent.width = width > supportInfo.surfaceCapabilities.maxImageExtent.width
            ? supportInfo.surfaceCapabilities.maxImageExtent.width : width;

            actualExtent.height = height < supportInfo.surfaceCapabilities.minImageExtent.height 
            ? supportInfo.surfaceCapabilities.minImageExtent.height : height;
            actualExtent.height = height > supportInfo.surfaceCapabilities.maxImageExtent.height
            ? supportInfo.surfaceCapabilities.maxImageExtent.height : height;
            
            return actualExtent;
        }
    }

    void VulkanRenderer::createSwapchain()
    {
        VulkanRenderer::SwapchainSupportInfo supportInfo = querySwapchainSupport(m_context, m_physicalDevice, m_surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(m_context, supportInfo);
        VkPresentModeKHR presentMode = choosePresentationMode(m_context, supportInfo);
        VkExtent2D swapExtent = chooseSwapExtent(m_context, supportInfo);
        
        uint32_t imageCount = supportInfo.surfaceCapabilities.minImageCount + 1;
        if (supportInfo.surfaceCapabilities.maxImageCount > 0 && imageCount > supportInfo.surfaceCapabilities.maxImageCount) {
            imageCount = supportInfo.surfaceCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = swapExtent;
        createInfo.imageArrayLayers = 1; //This is always 1 unless you are developing stereoscopic 3D app
        //TODO: Change this later if we are using a memory command to transfer images to swapchain
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        //Here, we specify how swapchain should be shared across multiple queue families
        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);
        uint32_t indices[] = { families.graphics.get(), families.presentation.get() };

        if(families.graphics.get() != families.presentation.get())
        {
            // If queue indices are different, enalble concurrent access to swapchain.
            // Images can be used without explicit transfer of ownership from one queue to another
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = indices;
        }
        else
        {
            // Image is owned by one queue at a time.
            // Ownership must be exclusively transferred if we plan to use this image in another queue
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        //Change this if you want the image to have a different transform 
        createInfo.preTransform = supportInfo.surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //This specifies if alpha channel should be used for blending with other windows
        createInfo.clipped = VK_TRUE;
        createInfo.presentMode = presentMode;

        VkResult res = vkCreateSwapchainKHR(m_device, &createInfo, &m_defGraphicsAllocator, &m_swapchain);
        assert(res == VK_SUCCESS && "Swapchain creation failed! Application will terminate");

        //retrieve images from swapchain
        uint32_t swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, nullptr);
        assert(swapchainImageCount != 0 && "No images were retrieved from swapchain.");
        m_swapchainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, m_swapchainImages.getBuffer_NonConst());
    
        m_swapchainFormat = surfaceFormat.format;
        m_swapchainExtent = swapExtent;
    }

    void VulkanRenderer::createImageViews()
    {
        m_swapchainImageViews.resize(m_swapchainImages.getSize());

        for(size_t i = 0; i < m_swapchainImages.getSize(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapchainFormat;
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

            VkResult res = vkCreateImageView(m_device, &createInfo, &m_defGraphicsAllocator, &m_swapchainImageViews[i]);
            assert(res == VK_SUCCESS && "Failed to create Image view for a swapchain image");
        }

    }
//--------------------------------------------------

//Graphics Pipeline Creation------------------------

//TODO: Access this through application module

    #define DATA_PATH "D:\\Works\\Project-Quaint\\Data\\"
    void getShaderCode(Quaint::IMemoryContext* context, const char* relFilePath, Quaint::QArray<char>& outCode)
    {
        Quaint::QPath filePath(DATA_PATH);
        filePath.append(relFilePath);
        //std::cout << filePath.getBuffer() << "\n";
        //open file as binary and read from end
        std::ifstream stream(filePath.getBuffer(), std::ios::ate | std::ios::binary);
        assert(stream.is_open() && "Given file could not be opened");
        size_t fileSize = (size_t)stream.tellg();
        outCode.resize(fileSize);
        stream.seekg(0);    //Go to beginning of file
        stream.read(outCode.getBuffer_NonConst(), fileSize);
        stream.close();
    }
    VkShaderModule createShaderModule(const VkDevice& device, const VkAllocationCallbacks& allocationCallbacks, const Quaint::QArray<char>& shaderCode)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.getSize();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.getBuffer());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VkResult res = vkCreateShaderModule(device, &createInfo, &allocationCallbacks, &shaderModule);
        assert(res == VK_SUCCESS && "Could not create a shader module");
        return shaderModule;
    }
    void VulkanRenderer::setupFixedFunctions()
    {
        //create Dynamic states. Specifying these ignores the configuration of these values and these must be specified at drawing time
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = dynamicStates.getSize();
        dynamicStateInfo.pDynamicStates = dynamicStates.getBuffer();


        //Pass in vertex input. This structure describes the format of vertex data that will be passed to the vertex shader
        //TODO: Since we are hardcoding vertices in shader, this will be specify that there's no vertex data to load for now
        VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
        vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        //Spacing between data and whether data is per-vertex or per-instance
        vertexInputStateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateInfo.pVertexBindingDescriptions = nullptr;
        //Types of attributes passed to vertex shader, which binding to load them from and at which offset
        vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateInfo.pVertexAttributeDescriptions = nullptr;


        //Input Assembly: We specify what kind of geometry to be drawn an if primitiveRestart should be enabled. Not sure what this is yet
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;


        //Setting up Viewport and scissor
        VkPipelineViewportStateCreateInfo viewportStateInfo{};
        viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        //Viewport and scissor are marked as dynamic states. It's enough to only mention their count here
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.viewportCount = 1;


        //Rasterizer: Takes geometry shaped by the vertices and turns it into fragments
        //Also performs depth testing, face culling
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        //If this is set to true, fragments that are beyond near and far planes are clamped as opposed to discarded.
        //This is useful in shadow maps;
        rasterizationInfo.depthClampEnable = VK_FALSE;
        //If this is true, geometry never passes through rasterization stage and nothing gets drawn to framebuffer
        rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.lineWidth = 1.0f;
        rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; //Cull back faces
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; //Vertices are processed in clock-wise direction

        rasterizationInfo.depthBiasEnable = VK_FALSE;
        rasterizationInfo.depthBiasConstantFactor = 0;
        rasterizationInfo.depthBiasClamp = 0;
        rasterizationInfo.depthBiasSlopeFactor = 0;


        //Multi-sampling: helps with alnti aliasing. More on this later
        VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
        multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingInfo.sampleShadingEnable = VK_FALSE;
        multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisamplingInfo.minSampleShading = 1.0f; // Optional
        multisamplingInfo.pSampleMask = nullptr; // Optional
        multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional


        //TODO: Depth and Stencil testing: We have to set up a structure for depth and stencil testing here late


        //Color Blending: 
        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;

        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo blendInfo{};
        blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendInfo.attachmentCount = 1;
        blendInfo.pAttachments = &blendAttachmentState;
        blendInfo.logicOpEnable = VK_FALSE;
        blendInfo.logicOp = VK_LOGIC_OP_COPY;
        blendInfo.blendConstants[0] = 0.0f;
        blendInfo.blendConstants[1] = 0.0f;
        blendInfo.blendConstants[2] = 0.0f;
        blendInfo.blendConstants[3] = 0.0f;


        // Pipeline Layout: Specifies the uniform values that will be passed to shaders.
        // TODO: We create an empty pipeline for now.
        VkPipelineLayoutCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineInfo.setLayoutCount = 0;
        pipelineInfo.pSetLayouts = nullptr;
        pipelineInfo.pushConstantRangeCount = 0;
        pipelineInfo.pPushConstantRanges = nullptr;

        VkResult res = vkCreatePipelineLayout(m_device, &pipelineInfo, &m_defGraphicsAllocator, &m_pipelineLayout);
        assert(res == VK_SUCCESS && "Failed to create a pipeline layout");

    }
    void VulkanRenderer::createRenderPipeline()
    {
        Quaint::QArray<char> vertexShaderCode(m_context);
        Quaint::QArray<char> fragmentShaderCode(m_context);
        getShaderCode(m_context, "Shaders\\TestTriangle\\simpleTri.vert.spv", vertexShaderCode);
        getShaderCode(m_context, "Shaders\\TestTriangle\\simpleTri.frag.spv", fragmentShaderCode);

        VkShaderModule vertexModule = createShaderModule(m_device, m_defGraphicsAllocator, vertexShaderCode);
        VkShaderModule fragmentModule = createShaderModule(m_device, m_defGraphicsAllocator, fragmentShaderCode);

        //To actually use the shaders, we need to assign them to a specific pipeline stage
        VkPipelineShaderStageCreateInfo vertStageInfo{};
        vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageInfo.module = vertexModule;
        vertStageInfo.pName = "main";
        //vertStageInfo.pSpecializationInfo = //This lets us specify values for shader constants. TODO: Read more on this later 

        VkPipelineShaderStageCreateInfo fragStageInfo{};
        fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStageInfo.module = fragmentModule;
        fragStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};

        setupFixedFunctions();


        vkDestroyShaderModule(m_device, vertexModule, &m_defGraphicsAllocator);
        vkDestroyShaderModule(m_device, fragmentModule, &m_defGraphicsAllocator);
    }

//----------------------------------------------------
#ifdef DEBUG_BUILD
//TODO: Currently Instance Creation and Destruction is not handled by our debug messenger. Will address this later

    void VulkanRenderer::setupDebugMessenger()
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if(func == nullptr)
        {
            QLOG_W(VULKAN_RENDERER_LOGGER, "Could not retrieve 'vkCreateDebugUtilsMessengerEXT'. Failed to create debug messenger");
            return;
        }
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallbackFunction;
        createInfo.pUserData = nullptr;

        if(func(m_instance, &createInfo, &m_defGraphicsAllocator, &m_debugMessenger) != VK_SUCCESS)
        {
            m_debugMessenger = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::destroyDebugMessenger()
    {
        if(m_debugMessenger == VK_NULL_HANDLE) return;

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if(func == nullptr)
        {
            QLOG_W(VULKAN_RENDERER_LOGGER, "Could not retrieve 'vkCreateDebugUtilsMessengerEXT'. Failed to create debug messenger");
            return;
        }
        func(m_instance, m_debugMessenger, &m_defGraphicsAllocator);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallbackFunction(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
    {
        //TODO: Add a much richers set of message filtering
        
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            QLOG_W(VULKAN_RENDERER_LOGGER, pCallbackData->pMessage);
        }
        else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            QLOG_W(VULKAN_RENDERER_LOGGER, pCallbackData->pMessage);
        }
        else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            QLOG_I(VULKAN_RENDERER_LOGGER, pCallbackData->pMessage);
        }

        return VK_FALSE;
    }
#endif
}