#include <BoltRenderer.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Bolt
{
    IRenderer*                  BoltRenderer::m_renderer_impl = nullptr;
    std::unique_ptr<BoltRenderer> BoltRenderer::m_renderer = nullptr;
    BoltRenderer::~BoltRenderer()
    {
        std::cout << "Renderer destroyed!!!!\n";
    }

    void BoltRenderer::startEngine(Quaint::IMemoryContext* context)
    {
        std::cout << "Renderer Started Successfully!!!\n";
        m_context = context;

        int* arr = QUAINT_NEW_ARRAY(context, int, 100, 200);

        QUAINT_DELETE_ARRAY(context, arr);

        m_renderer_impl = QUAINT_NEW(context, VulkanRenderer);
        m_renderer_impl->init(context);
        
        m_engineRunning = true;
    }

    void BoltRenderer::shutdown()
    {
        m_engineRunning = false;
        
        m_renderer_impl->shutdown();
        QUAINT_DELETE(m_context, m_renderer_impl);
        
        m_context = nullptr;
        std::cout << "Renderer shutdown successful!\n";
    }
}