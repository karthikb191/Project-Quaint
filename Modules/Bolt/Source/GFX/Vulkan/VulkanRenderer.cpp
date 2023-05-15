#include <GFX/Vulkan/VulkanRenderer.h>
#include <Types/QArray.h>
#include <Types/QFastArray.h>
#include <QuaintLogger.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Bolt
{
    #define VULKAN_RENDERER_LOGGER
    DECLARE_LOG_CATEGORY(VULKAN_RENDERER_LOGGER);
    DEFINE_LOG_CATEGORY(VULKAN_RENDERER_LOGGER);

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

        m_running = true;
    }

    void VulkanRenderer::shutdown()
    {

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
            "VK_KHR_device_group_creation",// REQUIRED! This extension provides instance-level commands to enumerate groups of physical devices, and to create a logical device from a subset of one of those groups
            "VK_KHR_external_fence_capabilities", //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” fence handles an implementation supports for a given set of use cases
            "VK_KHR_external_memory_capabilities", //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” memory handles an implementation supports for a given set of use cases
            //"VK_KHR_external_semaphore_capabilities", //??? This extension provides a set of capability queries and handle definitions that allow an application to determine what types of “external” semaphore handles an implementation supports for a given set of use cases
            "VK_KHR_get_physical_device_properties2", //REQUIRED! This extension provides new entry points to query device features, device properties, and format properties in a way that can be easily extended by other extensions, without introducing any further entry points
            "VK_KHR_get_surface_capabilities2", //REQUIRED! Provides an entry point to query device surface capabilities
            "VK_KHR_surface", //REQUIRED! Abstracts native platform surfaces for use with Vulkan. Provides a way to determine whether queue family in a device supports presenting to a surface   
            //"VK_KHR_surface_protected_capabilities", //??? This extension extends VkSurfaceCapabilities2KHR, providing applications a way to query whether swapchains can be created with the VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR flag set
            "VK_KHR_win32_surface",  //REQUIRED! Required for rendering to windows. Provided mechanism to create "VkSurfaceKHR" object 
            "VK_EXT_debug_report", //OPTIONAL! Enabled detailed debug reports
            "VK_EXT_debug_utils\0", //OPTIONAL! Enables support of passing a callback to handle debug messages and much more
            //"VK_EXT_swapchain_colorspace", //??? Might be needed. Not much information available
            //"VK_NV_external_memory_capabilities", //??? Applications may wish to import memory from the Direct 3D API, or export memory to other Vulkan instances. This extension provides a set of capability queries that allow applications determine what types of win32 memory handles an implementation supports for a given set of use cases.
            //"VK_KHR_portability_enumeration", //OPTIONAL! This extension allows applications to control whether devices that expose the VK_KHR_portability_subset extension are included in the results of physical device enumeration.
        });

        instanceInfo.enabledExtensionCount = instanceExtensions.getSize();
        instanceInfo.ppEnabledExtensionNames = instanceExtensions.getRawData();

        //Uncomment to print available instances
        //uint32_t extensionsCount = 0;
        //vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        //Quaint::QArray<VkExtensionProperties> extensions(m_context, extensionsCount);
        //vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.getBuffer_NonConst());
        //for(size_t i = 0; i < extensionsCount; i++)
        //{
        //    QLOG_I(VULKAN_RENDERER_LOGGER, extensions[i].extensionName);
        //}

        VkResult result = vkCreateInstance(&instanceInfo, &m_defGraphicsAllocator, &m_instance);
        if(result != VK_SUCCESS)
        {
            QLOG_E(VULKAN_RENDERER_LOGGER, "FATAL ERROR! Failed to create a Vulkan Instance");
        }
    }
}