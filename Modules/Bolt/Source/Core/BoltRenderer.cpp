#include <BoltRenderer.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <RenderModule.h>
#include <chrono>
#include <GFX/Entities/Painters.h>

namespace Bolt
{

    BoltRenderer::BoltRenderer()
    {
        std::cout << "Constructed Bolt Renderer\n";
    }
    BoltRenderer::~BoltRenderer()
    {
        std::cout << "Renderer destroyed!!!!\n";
    }

    //TODO: This should be moved to Application Module
    LRESULT CALLBACK msgHandleLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
            case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void BoltRenderer::startEngine(Quaint::IMemoryContext* context)
    {
        std::cout << "Renderer Started Successfully!!!\n";
        m_context = context;
        m_painters = Quaint::QArray<Painter*>(context);

        //TODO: Move window creation to Application Module
        Bolt::WindowCreationParams params;
        params.className = "Main Render Class";
        params.windowname = "Main Render Window";
        params.callback = &msgHandleLoop;
        params.width = 700;
        params.height = 700;
        m_window = Bolt::Window::createWindow(params);
        m_window.showWindow();
        
        //Initialize renderer
        VulkanRenderer* vulkanRenderer = QUAINT_NEW(context, VulkanRenderer, context);
        m_renderer_impl = vulkanRenderer;

        initCamera();
        m_renderer_impl->init();

        m_engineRunning = true;
    }

    void BoltRenderer::initCamera()
    {
        CameraInitInfo info{};
        info.translation = Quaint::QVec4(0, 0, 3, 1);
        info.rotation = Quaint::QVec3(0, 0, 0);
        info.fov = 90.0f;
        info.nearClipDist = 0.1f;
        info.farClipDist = 10000.0f;
        m_camera.init(info);
    }

    void BoltRenderer::addPainter(Painter* painter)
    {
        m_painters.pushBack(painter);
    }

    void BoltRenderer::update()
    {
        updateUniformBufferProxy();
        m_renderer_impl->render();
    }

    void BoltRenderer::updateUniformBufferProxy()
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        m_ubo.model = Quaint::buildRotationMatrixYZX(Quaint::QVec3( 0.f, time * 10.0f, 0.0f ));

        uint64_t timeNow = std::chrono::system_clock::now().time_since_epoch().count();
        float x = 5 * (float)std::sin(timeNow * 0.00000005);
        
        m_camera.lookAt( Quaint::QVec4(0.0f, 0.0f, 0.0f, 1.0f), 
        Quaint::QVec4(x, 1.0f, 2.0f, 1.0f),
        Quaint::QVec3(0.0f, 1.0f, 0.0f));
        m_ubo.view = m_camera.getViewMatrix();
        m_ubo.proj = m_camera.getProjectionMatrix();
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