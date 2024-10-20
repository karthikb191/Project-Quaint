#include <BoltRenderer.h>
#include <GFX/Vulkan/VulkanRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <RenderModule.h>

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
        m_renderObjectBuilder = vulkanRenderer;

        m_renderer_impl->init();

        m_engineRunning = true;
    }

    void BoltRenderer::update()
    {
        //TODO: Move this to Application Module
        MSG msg = { };
        while (true)
        {
            bool quitApplication = false;

            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                switch (msg.message)
                {
                case WM_DESTROY:
                    PostQuitMessage(0);
                    break;
                case WM_QUIT:
                    quitApplication = true;
                    break;
                case WM_PAINT:
                    std::cout << "Painting!!!!\n"; 
                    break; 
                
                default:
                    break;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if(quitApplication) break;

            m_renderer_impl->render();
        }
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