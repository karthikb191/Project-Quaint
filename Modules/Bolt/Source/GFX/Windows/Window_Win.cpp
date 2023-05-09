#include <GFX/Window.h>
#include <Windows.h>

namespace Bolt
{
    class Window_Impl
    {
    public:
        void createWindow()
        {
            
        }
    private:

    };

    Window Window::createWindow()
    {
        Window window;

        window.m_impl = new Window_Impl();
        window.m_impl->createWindow();
        return window;
    }
}