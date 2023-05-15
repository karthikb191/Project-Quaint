#ifndef _H_VULKAN_RENDERER
#define _H_VULKAN_RENDERER

#include <vulkan/vulkan.h>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <QuaintLogger.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Bolt
{
    class BoltRenderer;
    class VulkanRenderer : public IRenderer
    {
        friend class BoltRenderer;
        
        template<typename T, typename ...ARGS>
        friend T* ::allocFromContext(Quaint::IMemoryContext* context, ARGS...);

    public:
        void init(Quaint::IMemoryContext* context) override;
        void shutdown() override;
        
    private:
        static void* VKAPI_PTR allocationFunction(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR freeFunction(void* pUserData, void* pMemory);
        static void* VKAPI_PTR reallocFunction(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR internalAllocationFunction
        (void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
        static void VKAPI_PTR internalFreeFunction
        (void* pUserData, void* pMemory, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);

        void createInstance();
        void createAllocationCallbacks();

        VulkanRenderer();
        virtual ~VulkanRenderer();
        
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer(VulkanRenderer&&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(VulkanRenderer&&) = delete;

        bool                        m_running = false;
        Quaint::IMemoryContext*     m_context;
        
        VkAllocationCallbacks       m_defGraphicsAllocator;
        VkInstance                  m_instance;
    };
}

#endif