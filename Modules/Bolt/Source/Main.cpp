#include <iostream>
#include <vulkan/vulkan.h>
#include <BoltRenderer.h>
#include <Types/QFastArray.h>
#include <Types/QStaticString.h>
#include <Types/QStaticString_W.h>
#include <GFX/Window.h>

#include <MemoryModule.h>
#include <LoggerModule.h>
namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}

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

int main()
{
    std::cout << "Hello Renderer!" << std::endl;
    Bolt::BoltRenderer::get()->startEngine(Quaint::MemoryModule::get().getMemoryManager().getDefaultMemoryContext());

    Quaint::QStaticString_W<64> testStr(L"Test Test");
    testStr.length();

    Bolt::WindowCreationParams params;
    params.className = "Main Render Class";
    params.windowname = "Main Render Window";
    params.callback = &msgHandleLoop;
    Bolt::Window window = Bolt::Window::createWindow(params);
    window.showWindow();

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Bolt::BoltRenderer::get()->shutdown();
    
    return 0;
}