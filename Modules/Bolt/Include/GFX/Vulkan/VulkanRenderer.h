#ifndef _H_VULKAN_RENDERER
#define _H_VULKAN_RENDERER

#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <QuaintLogger.h>
#include <MemCore/GlobalMemoryOverrides.h>

//TODO: Surround with plat-spec macro
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace Bolt
{
    class BoltRenderer;
    class VulkanRenderer : public IRenderer
    {
        friend class BoltRenderer;
        
        template<typename T, typename ...ARGS>
        friend T* ::allocFromContext(Quaint::IMemoryContext* context, ARGS...);

    public:
        struct QueueFamily
        {
        public:
            void set(uint32_t idx) {index = idx; valid = true;}
            void unset() {index = 0; valid = false;}
            uint32_t get() { return index; }
            bool isSet(){ return valid; }

        private:
            uint32_t        index  = 0;
            bool            valid       = false;
        };
        struct QueueFamilies
        {
            QueueFamily     graphics{};
            QueueFamily     presentation{};

            bool allSet() { return graphics.isSet() && presentation.isSet(); }
        };

    public:
        void init(Quaint::IMemoryContext* context) override;
        void shutdown() override;
        
    private:
    //------ Static Allocation Functions
        static void* VKAPI_PTR allocationFunction(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR freeFunction(void* pUserData, void* pMemory);
        static void* VKAPI_PTR reallocFunction(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR internalAllocationFunction
        (void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR internalFreeFunction
        (void* pUserData, void* pMemory, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
    //----------------------------------

    //------ Static Message Handlers

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunction(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

    //------------------------------

        void createAllocationCallbacks();
        void createInstance();
        void createSurface();
        void selectPhysicalDevice();
        void createLogicalDevice();

        //TODO: Surround this with platform spec macro
        void createWindowsSurface(); //Surface creation might affect physical device selection
    #ifdef DEBUG_BUILD
        void setupDebugMessenger();
        void destroyDebugMessenger();        
    #endif

        VulkanRenderer();
        virtual ~VulkanRenderer();
        
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer(VulkanRenderer&&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(VulkanRenderer&&) = delete;

        bool                        m_running = false;
        Quaint::IMemoryContext*     m_context;
        
        VkAllocationCallbacks       m_defGraphicsAllocator;
        VkInstance                  m_instance = VK_NULL_HANDLE;
        VkPhysicalDevice            m_physicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR                m_surface = VK_NULL_HANDLE;
        VkDevice                    m_device = VK_NULL_HANDLE;

        VkQueue                     m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue                     m_presentQueue = VK_NULL_HANDLE;
        
    #ifdef DEBUG_BUILD
        VkDebugUtilsMessengerEXT    m_debugMessenger = VK_NULL_HANDLE;
    #endif
    };
}

#endif