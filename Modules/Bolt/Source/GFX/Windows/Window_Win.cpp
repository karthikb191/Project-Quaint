#include <Windows.h>
#include <assert.h>
#include <GFX/Window.h>
#include <Types/QStaticString_W.h>
#include <Types/QStaticString.h>
#include <QuaintLogger.h>

namespace Bolt
{
    using namespace Quaint;

    DECLARE_LOG_CATEGORY(Window_Win_Logger);
    DEFINE_LOG_CATEGORY(Window_Win_Logger);

    class Window_Impl : public Window::IWindow_Impl
    {
    public:
        Window_Impl(){}
        bool createWindow(const WindowCreationParams& params)
        {
            //NOTE: If running from a DLL, retrieving a HInstance might fail
            m_instance = GetModuleHandle(nullptr);
            WNDCLASS wc = {};
            wc.lpszClassName = params.className;
            wc.hInstance = m_instance;
            wc.lpfnWndProc = params.callback;
            wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

            RegisterClass(&wc);

            m_windowsHandle = CreateWindowEx
            (0, params.className, params.windowname
            , WS_OVERLAPPEDWINDOW, params.left, params.top
            , params.width, params.height, NULL, NULL, m_instance, NULL
            );
            
            if(m_windowsHandle == NULL)
            {
                QLOG_E(Window_Win_Logger, "Could not create window");
                return false;
            }

            QLOG_I(Window_Win_Logger, "Window created successfully");
            return true;
        }
        void showWindow()
        {
            //TODO: Allow multiple variations of cmdShow
            if(m_windowsHandle == NULL)
            {
                QLOG_W(Window_Win_Logger, "Trying to show window with an invalid handle");
                return;
            }
            ShowWindow(m_windowsHandle, SW_SHOWNORMAL);
        }
        void hideWindow()
        {
            if(m_windowsHandle == NULL)
            {
                QLOG_W(Window_Win_Logger, "Trying to hide window with an invalid handle");
                return;
            }
            ShowWindow(m_windowsHandle, SW_HIDE);
        }

    private:
        //TODO: Store window params
        HWND                        m_windowsHandle;
        HINSTANCE                   m_instance;
    };

    Window Window::createWindow(const WindowCreationParams& params)
    {
        Window window;

        window.m_impl = new Window_Impl();
        window.m_impl->createWindow(params);
        return window;
    }
}