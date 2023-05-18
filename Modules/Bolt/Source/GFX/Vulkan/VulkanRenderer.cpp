#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QArray.h>
#include <Types/QFastArray.h>
#include <QuaintLogger.h>
#include <RenderModule.h>
#include <BoltRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>

//TODO: Remove this once there's a custom version available
#include <set>

namespace Bolt
{
    #define VULKAN_RENDERER_LOGGER
    DECLARE_LOG_CATEGORY(VULKAN_RENDERER_LOGGER);
    DEFINE_LOG_CATEGORY(VULKAN_RENDERER_LOGGER);

    Quaint::QFastArray<const char*, 1> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };
    
    #define VALIDATION_LAYER_TYPE decltype(validationLayers)

    VulkanRenderer::~VulkanRenderer()
    {
    }
    VulkanRenderer::VulkanRenderer()
    : m_defGraphicsAllocator {}
    , m_instance {}
    {

    }
    void VulkanRenderer::init(Quaint::IMemoryContext* context)
    {
        m_context = context;
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

        m_running = true;
    }

    void VulkanRenderer::shutdown()
    {
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
        //m_defGraphicsAllocator.pfnInternalAllocation = VulkanRenderer::internalAllocationFunction;
        //m_defGraphicsAllocator.pfnInternalFree = VulkanRenderer::internalFreeFunction;
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

    size_t getDeviceSuitability(Quaint::IMemoryContext* context, const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
    {
        size_t score = 0;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceProperties(device, &properties);

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
        //TODO: To be filled later
        deviceInfo.enabledExtensionCount = 0;

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