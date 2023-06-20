#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QFastArray.h>
#include <Types/QStaticString.h>
#include <Types/QSet.h>
#include <QuaintLogger.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <fstream>
#include <chrono>

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
        VK_DYNAMIC_STATE_VIEWPORT
        , VK_DYNAMIC_STATE_SCISSOR
    );

    auto vertices = Quaint::createFastArray<QVertex>(
    {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},    //vertex 0
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},     //vertex 1
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    //vertex 2
        {{0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}    //vertex 3
    }
    );

    auto indices = Quaint::createFastArray<uint16_t>(
        {0, 1, 2, 0, 3, 1}
    );

    
    #define VALIDATION_LAYER_TYPE decltype(validationLayers)

    VulkanRenderer::~VulkanRenderer()
    {
    }
    VulkanRenderer::VulkanRenderer(Quaint::IMemoryContext* context)
    : m_context(context)
    , m_defGraphicsAllocator {VK_NULL_HANDLE}
    , m_allocationPtr(nullptr)
    , m_instance {VK_NULL_HANDLE}
    , m_swapchainImages(context)
    , m_swapchainImageViews(context)
    , m_frameBuffers(context)
    , m_commandBuffers(context)
    , m_imageAvailableSemaphores(context)
    , m_renderFinishedSemaphores(context)
    , m_inFlightFences(context)
    , m_uniformBuffers(context)
    , m_uniformBufferGpuMemory(context)
    , m_mappedUniformBuffers(context)
    , m_descriptorSets(context)
    {
    }

    void VulkanRenderer::init()
    {
        CameraInitInfo info{};
        info.fov = 90.0f;
        info.rotation = Quaint::QVec3(0);
        info.translation = Quaint::QVec4(0);
        info.nearClipDist = 0.1f;
        info.farClipDist = 10000.0f;
        m_camera.init(info);

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
        createRenderPass();
        createDescriptorSetLayout();
        createRenderPipeline();
        createFrameBuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();

        createCommandBuffer();
        createSyncObjects();
        QLOG_V(VULKAN_RENDERER_LOGGER, "Vulkan Renderer Running");

        m_running = true;
    }

    void VulkanRenderer::shutdown()
    {
        vkDeviceWaitIdle(m_device);

        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], m_allocationPtr);
            vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], m_allocationPtr);
            vkDestroyFence(m_device, m_inFlightFences[i], m_allocationPtr);
        }

        vkDestroyCommandPool(m_device, m_graphicsCommandPool, m_allocationPtr);
        vkDestroyCommandPool(m_device, m_transferCommandPool, m_allocationPtr);
        for(const VkFramebuffer& buffer : m_frameBuffers)
        {
            vkDestroyFramebuffer(m_device, buffer, m_allocationPtr);
        }

        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(m_device, m_uniformBuffers[i], m_allocationPtr);
            vkFreeMemory(m_device, m_uniformBufferGpuMemory[i], m_allocationPtr);
        }

        vkDestroyBuffer(m_device, m_indexBuffer, m_allocationPtr);
        vkFreeMemory(m_device, m_indexBufferGpuMemory, m_allocationPtr);
        vkDestroyBuffer(m_device, m_vertexBuffer, m_allocationPtr);
        //Memory in device can be freed once the bound buffer is no longer used
        vkFreeMemory(m_device, m_vertexBufferGpuMemory, m_allocationPtr);

        vkDestroyDescriptorPool(m_device, m_descriptorPool, m_allocationPtr);
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, m_allocationPtr);
        vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocationPtr);
        vkDestroyRenderPass(m_device, m_renderPass, m_allocationPtr);
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocationPtr);
        for(const VkImageView& view : m_swapchainImageViews)
        {
            vkDestroyImageView(m_device, view, m_allocationPtr);
        }
        vkDestroySwapchainKHR(m_device, m_swapchain, m_allocationPtr);
        vkDestroyDevice(m_device, m_allocationPtr);
        vkDestroySurfaceKHR(m_instance, m_surface, m_allocationPtr);
#ifdef DEBUG_BUILD
        destroyDebugMessenger();
#endif
        vkDestroyInstance(m_instance, m_allocationPtr);
    }

    void VulkanRenderer::render()
    {
        drawFrame();
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
        m_allocationPtr = &m_defGraphicsAllocator;
    }
    //............................................................................

    bool areAllValidationLayersAvailable(Quaint::IMemoryContext* context, const VALIDATION_LAYER_TYPE& validationLayers)
    {
        uint32_t availableLayers = 0;
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
        appInfo.apiVersion = VK_API_VERSION_1_0;

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
            , "VK_KHR_external_semaphore_capabilities" //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” semaphore handles an implementation supports for a given set of use cases
            , "VK_KHR_get_physical_device_properties2" //REQUIRED! This extension provides new entry points to query device features, device properties, and format properties in a way that can be easily extended by other extensions, without introducing any further entry points
            , "VK_KHR_get_surface_capabilities2" //REQUIRED! Provides an entry point to query device surface capabilities
            , "VK_KHR_surface" //REQUIRED! Abstracts native platform surfaces for use with Vulkan. Provides a way to determine whether queue family in a device supports presenting to a surface   
            //, "VK_KHR_surface_protected_capabilities" //??? This extension extends VkSurfaceCapabilities2KHR, providing applications a way to query whether swapchains can be created with the VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR flag set
            , "VK_KHR_win32_surface"  //REQUIRED! Required for rendering to windows. Provided mechanism to create "VkSurfaceKHR" object 
        #ifdef DEBUG_BUILD
            , "VK_EXT_debug_report" //OPTIONAL! Enabled detailed debug reports
            , VK_EXT_DEBUG_UTILS_EXTENSION_NAME //OPTIONAL! Enables support of passing a callback to handle debug messages and much more
        #endif
            , "VK_EXT_swapchain_colorspace", //??? Might be needed. Not much information available
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

        VkResult result = vkCreateInstance(&instanceInfo, m_allocationPtr, &m_instance);
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
    
        VkResult res = vkCreateWin32SurfaceKHR(m_instance, &winSurfaceInfo, m_allocationPtr, &m_surface);
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
            else if(properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                families.transfer.set(i);
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
        VkResult res = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionProperties.getBuffer_NonConst());

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
            if(!found)
            {
                allExtensionsAvailable = false;
                break;  
            } 
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
        Quaint::QSet<uint32_t> queueIndices(m_context);
        
        queueIndices = { queueFamilies.graphics.get(), queueFamilies.presentation.get(), queueFamilies.transfer.get() };

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

        VkResult res = vkCreateDevice(m_physicalDevice, &deviceInfo, m_allocationPtr, &m_device);
        assert(res == VK_SUCCESS && "Logical device creation failed!");
        
        //Once Logical device is created, retrieve queues to interface with
        vkGetDeviceQueue(m_device, queueFamilies.graphics.get(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, queueFamilies.presentation.get(), 0, &m_presentQueue);
        vkGetDeviceQueue(m_device, queueFamilies.transfer.get(), 0, &m_transferQueue);
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
        //for (const auto& availablePresentMode : supportInfo.presentMode) {
        //    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        //        return availablePresentMode;
        //    }
        //}
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

        Quaint::QSet<uint32_t> uniqueIndices(m_context);
        uniqueIndices = { families.graphics.get(), families.presentation.get() };
        
        if(uniqueIndices.getSize() > 1)
        {
            Quaint::QArray<uint32_t> indexArray(m_context);
            for(auto index : uniqueIndices)
            {
                indexArray.pushBack(index);
            }
            // If queue indices are different, enalble concurrent access to swapchain.
            // Images can be used without explicit transfer of ownership from one queue to another
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = indexArray.getSize();
            createInfo.pQueueFamilyIndices = indexArray.getBuffer();
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
        createInfo.oldSwapchain = m_outOfDateSwapchain; //This will eventually have a null handle 
        createInfo.presentMode = presentMode;

        VkResult res = vkCreateSwapchainKHR(m_device, &createInfo, m_allocationPtr, &m_swapchain);
        assert(res == VK_SUCCESS && "Swapchain creation failed! Application will terminate");

        //retrieve images from swapchain
        uint32_t swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, nullptr);
        assert(swapchainImageCount != 0 && "No images were retrieved from swapchain.");
        m_swapchainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, m_swapchainImages.getBuffer_NonConst());
    
        m_swapchainFormat = surfaceFormat.format;
        m_swapchainExtent = swapExtent;

        //This builds a proper camera projection matrix
        m_camera.setAspectRatio((float)m_swapchainExtent.width / (float)m_swapchainExtent.height);
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

            VkResult res = vkCreateImageView(m_device, &createInfo, m_allocationPtr, &m_swapchainImageViews[i]);
            assert(res == VK_SUCCESS && "Failed to create Image view for a swapchain image");
        }

    }
//--------------------------------------------------

    void VulkanRenderer::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding info{};
        info.binding = 0;
        // It's possible for shader variable to represent an array of Uniform objects.
        // In that case, we pass the appropriate descriptor count
        info.descriptorCount = 1;
        info.descriptorType =VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        //Specifies in which shader stage the descriptor is going to be referenced
        info.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


        VkDescriptorSetLayoutCreateInfo descriptorLayout{};
        descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.bindingCount = 1;
        descriptorLayout.pBindings = &info;
        
        VkResult res = vkCreateDescriptorSetLayout(m_device, &descriptorLayout, m_allocationPtr, &m_descriptorSetLayout);
        assert(res == VK_SUCCESS && "Failed to create descriptor set layout");
    }

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
    VkShaderModule createShaderModule(const VkDevice& device, const VkAllocationCallbacks* allocationCallbacks, const Quaint::QArray<char>& shaderCode)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.getSize();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.getBuffer());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VkResult res = vkCreateShaderModule(device, &createInfo, allocationCallbacks, &shaderModule);
        assert(res == VK_SUCCESS && "Could not create a shader module");
        return shaderModule;
    }

    void VulkanRenderer::setupFixedFunctions(FixedStageInfo& fixedStageInfo)
    {
        //create Dynamic states. Specifying these ignores the configuration of these values and these must be specified at drawing time
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = dynamicStates.getSize();
        dynamicStateInfo.pDynamicStates = dynamicStates.getBuffer();

        fixedStageInfo.dynamicStateInfo = dynamicStateInfo;


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

        fixedStageInfo.vertexInputStateInfo = vertexInputStateInfo;


        //Input Assembly: We specify what kind of geometry to be drawn an if primitiveRestart should be enabled. Not sure what this is yet
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        fixedStageInfo.pipelineInputAssemblyStateInfo = inputAssemblyInfo;


        //Setting up Viewport and scissor
        VkPipelineViewportStateCreateInfo viewportStateInfo{};
        viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        //Viewport and scissor are marked as dynamic states. It's enough to only mention their count here
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.viewportCount = 1;

        fixedStageInfo.viewportStateInfo = viewportStateInfo;


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
        rasterizationInfo.cullMode = VK_CULL_MODE_NONE;// VK_CULL_MODE_BACK_BIT; //Cull back faces
        rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //Vertices are processed in clock-wise direction

        rasterizationInfo.depthBiasEnable = VK_FALSE;

        fixedStageInfo.rasterizationStateInfo = rasterizationInfo;


        //Multi-sampling: helps with alnti aliasing. More on this later
        VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
        multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingInfo.sampleShadingEnable = VK_FALSE;
        multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        fixedStageInfo.multisampleSateInfo = multisamplingInfo;


        //TODO: Depth and Stencil testing: We have to set up a structure for depth and stencil testing here late


        //Color Blending: 
        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.blendEnable = VK_FALSE;

        fixedStageInfo.blendAttachmentState = blendAttachmentState;


        VkPipelineColorBlendStateCreateInfo blendInfo{};
        blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendInfo.attachmentCount = 1;
        blendInfo.pAttachments = &fixedStageInfo.blendAttachmentState;
        blendInfo.logicOpEnable = VK_FALSE;
        blendInfo.logicOp = VK_LOGIC_OP_COPY;
        blendInfo.blendConstants[0] = 0.0f;
        blendInfo.blendConstants[1] = 0.0f;
        blendInfo.blendConstants[2] = 0.0f;
        blendInfo.blendConstants[3] = 0.0f;

        fixedStageInfo.colorBlendInfo = blendInfo;


        // Pipeline Layout: Specifies the uniform values that will be passed to shaders.
        // TODO: We create an empty pipeline for now.
        VkPipelineLayoutCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineInfo.setLayoutCount = 1;
        pipelineInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineInfo.pushConstantRangeCount = 0;
        pipelineInfo.pPushConstantRanges = nullptr;

        VkResult res = vkCreatePipelineLayout(m_device, &pipelineInfo, m_allocationPtr, &m_pipelineLayout);
        assert(res == VK_SUCCESS && "Failed to create a pipeline layout");

    }
    void VulkanRenderer::createRenderPass()
    {
        // We need to tell Vulkan about frame buffer attachments that we'll be using for rendering.
        // We need to specify how many color and depth buffer there will be, how many samples to use for each of them
        // and how their contents are handled throughout rendering operations

        VkAttachmentDescription colorDesc{};
        colorDesc.format = m_swapchainFormat;
        colorDesc.samples = VK_SAMPLE_COUNT_1_BIT; //We are not doing anything with multi-sampling. So, setting this to 1 bit
        
        //loadOp and storeOp determines what to do with data before and after rendering.
        colorDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // Textures and framebuffers in Vulkan are represented by VkImage with a certain pixel format. 
        // Depending on what we are trying to do with the image, their layout in the memory can change
        // Images need to be transitioned to specific layouts that are suitable for operations that will be performed on them next
        colorDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


        // A single renderpass can consist of multiple subpasses. Subpasses are subsequent render operations that depends on
        // content of framebuffer from previous passes.

        //Every subpass references one or more attachments we have described before
        VkAttachmentReference attachmentRef{};
        // Attchment parameter specifies which attachment decription to reference by index
        // Layout specifies which layout that attchment should have when running this subpass
        // attachment is either an integer value identifying an attachment at the corresponding index in VkRenderPassCreateInfo::pAttachments, 
        // or VK_ATTACHMENT_UNUSED to signify that this attachment is not used
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // Each element of pColorAttachments correspond to output location in the shader
        // In the simple triangle, this attachmentReference is accessed by fragment shader through "layout(location = 0) out vec3 fragColor"

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorDesc;

        
        //TODO: READ ALOT MORE ON THIS PART
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult res = vkCreateRenderPass(m_device, &renderPassInfo, m_allocationPtr, &m_renderPass);
        assert(res == VK_SUCCESS && "Render pass creation failed");
    }
    void VulkanRenderer::createRenderPipeline()
    {
        Quaint::QArray<char> vertexShaderCode(m_context);
        Quaint::QArray<char> fragmentShaderCode(m_context);
        getShaderCode(m_context, "Shaders\\TestTriangle\\simpleTri.vert.spv", vertexShaderCode);
        getShaderCode(m_context, "Shaders\\TestTriangle\\simpleTri.frag.spv", fragmentShaderCode);

        VkShaderModule vertexModule = createShaderModule(m_device, m_allocationPtr, vertexShaderCode);
        VkShaderModule fragmentModule = createShaderModule(m_device, m_allocationPtr, fragmentShaderCode);

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

        FixedStageInfo fixedStageInfo;
        setupFixedFunctions(fixedStageInfo);

        VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
        graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        //Shader Modules setup
        graphicsPipelineInfo.stageCount = 2;
        graphicsPipelineInfo.pStages = shaderStages;
        
        //Fixed stage setup
        graphicsPipelineInfo.pDynamicState = &fixedStageInfo.dynamicStateInfo;


//-------------Vertex Shader input binding----------------------
        auto inputBindingDesc = QVertex::getBindingDescription();
        Quaint::QFastArray<VkVertexInputAttributeDescription, 2> attributeDesc;
        QVertex::getAttributeDescription(attributeDesc);
        fixedStageInfo.vertexInputStateInfo.vertexBindingDescriptionCount = 1;
        fixedStageInfo.vertexInputStateInfo.pVertexBindingDescriptions = &inputBindingDesc;

        fixedStageInfo.vertexInputStateInfo.vertexAttributeDescriptionCount = attributeDesc.getSize();
        fixedStageInfo.vertexInputStateInfo.pVertexAttributeDescriptions = attributeDesc.getBuffer();

        graphicsPipelineInfo.pVertexInputState = &fixedStageInfo.vertexInputStateInfo;
//---------------------------------------------------------------


        graphicsPipelineInfo.pInputAssemblyState = &fixedStageInfo.pipelineInputAssemblyStateInfo;
        graphicsPipelineInfo.pViewportState = &fixedStageInfo.viewportStateInfo;
        graphicsPipelineInfo.pRasterizationState = &fixedStageInfo.rasterizationStateInfo;
        graphicsPipelineInfo.pMultisampleState = &fixedStageInfo.multisampleSateInfo;
        graphicsPipelineInfo.pColorBlendState = &fixedStageInfo.colorBlendInfo;
        graphicsPipelineInfo.pDepthStencilState = nullptr;

        graphicsPipelineInfo.layout = m_pipelineLayout;

        graphicsPipelineInfo.renderPass = m_renderPass;
        graphicsPipelineInfo.subpass = 0;

        //Vulkan lets you create new pipelines by deriving from existing ones. More on this later
        graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipelineInfo.basePipelineIndex = 0;

        VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, m_allocationPtr, &m_graphicsPipeline);

        assert(res==VK_SUCCESS && "Failed to create graphics pipeline");

        vkDestroyShaderModule(m_device, vertexModule, m_allocationPtr);
        vkDestroyShaderModule(m_device, fragmentModule, m_allocationPtr);
    }

    void VulkanRenderer::createFrameBuffers()
    {
        //m_frameBuffers.resize(m_swapchainImageViews.getSize());
        m_frameBuffers.resize(m_swapchainImageViews.getSize());
        for(size_t i = 0; i < m_frameBuffers.getSize(); i++)
        {
            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.width = m_swapchainExtent.width;
            frameBufferInfo.height = m_swapchainExtent.height;
            frameBufferInfo.renderPass = m_renderPass;
            frameBufferInfo.attachmentCount = 1;
            frameBufferInfo.pAttachments = &m_swapchainImageViews[i];
            frameBufferInfo.layers = 1;

            VkResult res = vkCreateFramebuffer(m_device, &frameBufferInfo, m_allocationPtr, &m_frameBuffers[i]);
            assert(res==VK_SUCCESS && "Failed to create a frame buffer");
        }
    }

//---------------Command Pool Creation--------------------------------------
    void VulkanRenderer::createCommandPool()
    {
        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);
        
        createGraphicsCommandPool();
        //Create a new command pool if we have different graphics and transfer queues set.
        if(families.graphics.get() != families.transfer.get())
        {
            createTransferCommandPool();
        }
        else
        {
            m_transferCommandPool = m_graphicsCommandPool;
        }
    }
    void VulkanRenderer::createGraphicsCommandPool()
    {
        //Command pools manage memory that is used to allocate command buffers from
        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows command buffers to be reset individually
        commandPoolInfo.queueFamilyIndex = families.graphics.get();

        VkResult res = vkCreateCommandPool(m_device, &commandPoolInfo, m_allocationPtr, &m_graphicsCommandPool);
        assert(res==VK_SUCCESS && "Could not create command pool");
    }
    void VulkanRenderer::createTransferCommandPool()
    {
        //Command pools manage memory that is used to allocate command buffers from
        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows command buffers to be reset individually
        commandPoolInfo.queueFamilyIndex = families.transfer.get();

        VkResult res = vkCreateCommandPool(m_device, &commandPoolInfo, m_allocationPtr, &m_transferCommandPool);
        assert(res==VK_SUCCESS && "Could not create command pool");
    }
//---------------------------------------------------------------------------------------

    void VulkanRenderer::createBuffer(size_t bufferSize, VkBufferUsageFlags usageFlags, 
    const Quaint::QArray<uint32_t>& queueFamilies, 
    VkMemoryPropertyFlags propertyFlags,
    VkDeviceMemory& deviceMemory,
    VkBuffer& buffer)
    {
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = bufferSize;
        info.usage = usageFlags; //It is possible to specify multiple usages using bitwise OR
        //Just like swapchain images, buffers can also be shared across queues or have unique ownership
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if(queueFamilies.getSize() > 1)
        {
            info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = queueFamilies.getSize();
            info.pQueueFamilyIndices = queueFamilies.getBuffer();
        }

        VkResult res = vkCreateBuffer(m_device, &info, m_allocationPtr, &buffer);
        assert(res == VK_SUCCESS && "Could not create a vertex buffer");

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(m_device, buffer, &requirements);
        
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

        uint32_t memoryTypeIndex;
        bool found = false;
        for(size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            // memoryTypeBits is bit field of memory types that are suitable for buffer
            // Also, we are not just interested in memory type that's suitable to the buffer. We should also be able to write to it
            // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT properties indicate that we can write to it from CPU
            
            if((requirements.memoryTypeBits & (1 << i)) && 
            (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
            {
                memoryTypeIndex = i;
                found = true;
                break;
            }
        }
        assert(found && "Could not find suitable a memory type for device memory allocation");

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        allocInfo.allocationSize = requirements.size;
        
        res = vkAllocateMemory(m_device, &allocInfo, m_allocationPtr, &deviceMemory);
        assert(res == VK_SUCCESS && "Failed to allocate vertex buffer on device");

        //Offset should be a multiple of alignment if it's not 0. 0 works since we are specifically binding data for a vertex
        vkBindBufferMemory(m_device, buffer, deviceMemory, 0);
        
    }

    void copyBuffer(VkDevice device, VkQueue transferQueue, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool srcCommandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = srcCommandPool;


        VkCommandBuffer copyBuffer;
        VkResult res = vkAllocateCommandBuffers(device, &allocInfo, &copyBuffer);
        assert(res == VK_SUCCESS && "Could not create copy buffer");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        res = vkBeginCommandBuffer(copyBuffer, &beginInfo);
        assert(res == VK_SUCCESS && "Could not begin copy command buffer");

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(copyBuffer, src, dst, 1, &copyRegion);

        vkEndCommandBuffer(copyBuffer);

        //Submit to GPU immediately
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &copyBuffer;

        res = vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
        assert(res == VK_SUCCESS && "Failed to submit copy command to transfer queue");
        vkQueueWaitIdle(transferQueue);

        vkFreeCommandBuffers(device, srcCommandPool, 1, &copyBuffer);
    }

    void VulkanRenderer::createVertexBuffer()
    {
        size_t bufferSize = sizeof(QVertex) * vertices.getSize();
        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);

        Quaint::QArray<uint32_t> queueFamilies(m_context);
        queueFamilies.pushBack(families.graphics.get());
        if(families.graphics.get() != families.transfer.get())
        {
            queueFamilies.pushBack(families.transfer.get());
        }
        
        //Staging buffer creation
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferGpuMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, queueFamilies, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferGpuMemory, stagingBuffer);

        //Move vertex data to staging buffer
        void* data;
        vkMapMemory(m_device, stagingBufferGpuMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.getBuffer(), bufferSize);
        vkUnmapMemory(m_device, stagingBufferGpuMemory);

        //Vertex Buffer creation
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, queueFamilies,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBufferGpuMemory, m_vertexBuffer);
        
        //Immediately copy from staging buffer to vertex buffer
        copyBuffer(m_device, m_transferQueue, stagingBuffer, m_vertexBuffer, bufferSize, m_transferCommandPool);

        vkDestroyBuffer(m_device, stagingBuffer, m_allocationPtr);
        vkFreeMemory(m_device, stagingBufferGpuMemory, m_allocationPtr);
    }

    void VulkanRenderer::createIndexBuffer()
    {
        size_t bufferSize = sizeof(decltype(indices)::value_type) * indices.getSize();
        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);

        Quaint::QArray<uint32_t> queueFamilies(m_context);
        queueFamilies.pushBack(families.graphics.get());
        if(families.graphics.get() != families.transfer.get())
        {
            queueFamilies.pushBack(families.transfer.get());
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferGpuMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, queueFamilies,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferGpuMemory, stagingBuffer);

        //Move index data to staging buffer
        void* data;
        vkMapMemory(m_device, stagingBufferGpuMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.getBuffer(), bufferSize);
        vkUnmapMemory(m_device, stagingBufferGpuMemory);

        //Index Buffer creation
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queueFamilies,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBufferGpuMemory, m_indexBuffer);
        
        //Immediately copy from staging buffer to vertex buffer
        copyBuffer(m_device, m_transferQueue, stagingBuffer, m_indexBuffer, bufferSize, m_transferCommandPool);

        vkDestroyBuffer(m_device, stagingBuffer, m_allocationPtr);
        vkFreeMemory(m_device, stagingBufferGpuMemory, m_allocationPtr);

    }

    void VulkanRenderer::createUniformBuffers()
    {
        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBufferGpuMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_mappedUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        size_t bufferSize = sizeof(UniformBufferObject);

        QueueFamilies families;
        getQueueFamilies(m_context, m_physicalDevice, m_surface, families);
        Quaint::QArray<uint32_t> queueFamilies(m_context);
        queueFamilies.pushBack(families.graphics.get());
        if(families.graphics.get() != families.transfer.get())
        {
            queueFamilies.pushBack(families.transfer.get());
        }

        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, queueFamilies,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_uniformBufferGpuMemory[i], m_uniformBuffers[i]);

            vkMapMemory(m_device, m_uniformBufferGpuMemory[i], 0, bufferSize, 0, &m_mappedUniformBuffers[i]);
        }

    }

    void VulkanRenderer::createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;

        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        VkResult res = vkCreateDescriptorPool(m_device, &poolInfo, m_allocationPtr, &m_descriptorPool);
        assert(res == VK_SUCCESS && "Failed to create descriptor pool");
    }
    void VulkanRenderer::createDescriptorSets()
    {
        m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        Quaint::QArray<VkDescriptorSetLayout> layouts(m_context, MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.getBuffer();

        VkResult res = vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.getBuffer_NonConst());
        assert(res == VK_SUCCESS && "Could not allocate descriptor sets");

        //Descriptor sets are allocated, but they arent filled with approproate values
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
        
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.dstSet = m_descriptorSets[i]; //This specifies the descriptor set to update
            write.dstBinding = 0;
            write.dstArrayElement = 0;

            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
        }

    }

    void VulkanRenderer::createCommandBuffer()
    {
        //Command buffers will be automatically cleaned when their pool is destroyed
        VkCommandBufferAllocateInfo bufferAllocateInfo{};
        bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocateInfo.commandPool = m_graphicsCommandPool;
        //PRIMARY buffers can be submitted to queues, but cannot be called from other command buffers
        //SECONDARY buffers can be called from primary command buffers, but cannot directly submitted to queue
        bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        bufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

        m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkResult res = vkAllocateCommandBuffers(m_device, &bufferAllocateInfo, m_commandBuffers.getBuffer_NonConst());
        assert(res==VK_SUCCESS && "Could not allocate command buffer");
    }

    void VulkanRenderer::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        VkResult res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        assert(res==VK_SUCCESS && "Could not being command buffer");

        //Now we begin a renderpass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_frameBuffers[imageIndex];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchainExtent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.5f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //Binder renderpass to a point in the pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchainExtent.width);
        viewport.height = static_cast<float>(m_swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        
        //vkCmdDraw(commandBuffer, vertices.getSize(), 1, 0, 0);

        vkCmdBindDescriptorSets(commandBuffer,
         VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, indices.getSize(), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        res = vkEndCommandBuffer(commandBuffer);
        assert(res==VK_SUCCESS && "Could not end command buffer");
    }

    void VulkanRenderer::createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;
        
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Init fence in signalled state

        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkCreateSemaphore(m_device, &semaphoreInfo, m_allocationPtr, &m_imageAvailableSemaphores[i]);
            vkCreateSemaphore(m_device, &semaphoreInfo, m_allocationPtr, &m_renderFinishedSemaphores[i]);
            vkCreateFence(m_device, &fenceInfo, m_allocationPtr, &m_inFlightFences[i]);
        }
    }

    void VulkanRenderer::updateUniformBuffer(size_t index)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = Quaint::buildRotationMatrixYZX(Quaint::QVec3( 0.f, time * 10.0f, 0.0f ));

        //glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //ubo.view =  glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
        
        m_camera.lookAt( Quaint::QVec4(0.0f, 0.0f, 0.0f, 1.0f), Quaint::QVec4(2.0f, 2.0f, 2.0f, 1.0f), Quaint::QVec3(0.0f, 1.0f, 0.0f));
        ubo.view = m_camera.getViewMatrix();
        ubo.proj = m_camera.getProjectionMatrix();

        memcpy(m_mappedUniformBuffers[index], &ubo, sizeof(ubo));
    }

    void VulkanRenderer::drawFrame()
    {
        vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        
        //Semaphore is signalled when presentation engine finishes using the image
        VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

        if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            recreateSwapchain();
            return;
        }
        assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR && "Failed to acquire swapchain image");
        vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);


        vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

        recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

        updateUniformBuffer(m_currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        //Wait for image to become available
        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
        
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        //Wait for image to be available from vkAcquireImage. Then singal renderFinishedSemaphore once render is finished
        res = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]);
        assert(res == VK_SUCCESS && "Could not submit to queue");
        //assert(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS && "Could not submit to queue");

        //READ ALOT MORE ON THIS PART
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];


        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &imageIndex;

        res = vkQueuePresentKHR(m_presentQueue, &presentInfo);
        
        if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            recreateSwapchain();
            return;
        }
        assert(res == VK_SUCCESS && "Failed to acquire swapchain image");

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        assert(res==VK_SUCCESS && "Could not present");
    }

    void VulkanRenderer::cleanupOutofDateSwapchain()
    {
        for(const VkFramebuffer& buffer : m_frameBuffers)
        {
            vkDestroyFramebuffer(m_device, buffer, m_allocationPtr);
        }

        for(const VkImageView& view : m_swapchainImageViews)
        {
            vkDestroyImageView(m_device, view, m_allocationPtr);
        }
        vkDestroySwapchainKHR(m_device, m_outOfDateSwapchain, m_allocationPtr);
    }
    void VulkanRenderer::recreateSwapchain()
    {
        // We need to cleanup images, imageviews, old swapchain extent, format and other info.
        // Framebuffers also depend on swapchain. They must also be recreated

        //Here we are assuming swapchain format doesnt change. If it changes, we must also have to recreate RenderPass

        vkDeviceWaitIdle(m_device);

        m_outOfDateSwapchain = m_swapchain;
        createSwapchain();

        cleanupOutofDateSwapchain();

        createImageViews();
        createFrameBuffers();
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

        if(func(m_instance, &createInfo, m_allocationPtr, &m_debugMessenger) != VK_SUCCESS)
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
        func(m_instance, m_debugMessenger, m_allocationPtr);
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
        else
        {
            QLOG_V(VULKAN_RENDERER_LOGGER, pCallbackData->pMessage);
        }
        return VK_FALSE;
    }
#endif
}