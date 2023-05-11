#include <BoltRenderer.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Bolt
{
    IRenderer*                  BoltRenderer::m_renderer_impl = nullptr;
    std::unique_ptr<BoltRenderer> BoltRenderer::m_renderer = nullptr;
    BoltRenderer::~BoltRenderer()
    {
        std::cout << "Renderer destroyed!!!!" << "\n";
    }

    void BoltRenderer::startEngine(Quaint::IMemoryContext* context)
    {
        std::cout << "Renderer Started!!!" << "\n";
        m_engineRunning = true;
        
        int* arr = QUAINT_NEW_ARRAY(context, int, 100, 200);

        m_renderer_impl = QUAINT_NEW(VulkanRenderer, context);
        m_renderer_impl->init(context);
        QUAINT_DELETE(m_renderer_impl, context);

        int i = 100;
        i -= 10;
    }

    void BoltRenderer::shutdown()
    {
        std::cout << "Renderer shutdown!!!!" << "\n";
        m_engineRunning = false;

        m_renderer_impl->shutdown();
        delete m_renderer_impl;
    }
}