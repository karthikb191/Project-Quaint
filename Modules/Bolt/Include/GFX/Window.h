#ifndef _H_WINDOW
#define _H_WINDOW

//TODO: Surround with plat-spec macro
#include <Windows.h>

namespace Bolt
{
    struct WindowCreationParams
    {
        char*    windowname     = "Default Window";
        char*       className   = "Default Class";
        //TODO: Surround this with a Plat-Spec macro
        WNDPROC     callback    = nullptr;
        size_t      width       = 400;
        size_t      height      = 400;
        size_t      top         = 200;
        size_t      left        = 200;
        //TODO: Add option to have multiple window styles
    };

    class Window_Impl;
    class Window
    {
        class IWindow_Impl
        {
        public:
            virtual bool createWindow(const WindowCreationParams& params) = 0;
            virtual void showWindow() = 0;
            virtual void hideWindow() = 0;
        };
        friend class Window_Impl;
    public:
        static Window createWindow(const WindowCreationParams& params);

        void showWindow() { m_impl->showWindow(); }
        void hideWindow() { m_impl->hideWindow(); }

    private:
        
        IWindow_Impl*    m_impl = nullptr;
    };
}

#endif