#ifndef _H_VULKAN_RENDERER
#define _H_VULKAN_RENDERER

#include <vulkan/vulkan.h>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <QuaintLogger.h>

namespace Bolt
{
    class VulkanRenderer : public IRenderer
    {
    public:
        void init(Quaint::IMemoryContext* context) override;
        
    private:
        Quaint::IMemoryContext*     m_context;
        VkAllocationCallbacks       m_defGraphicsAllocator;
    };
}

#endif