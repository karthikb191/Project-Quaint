#ifndef _H_WINDOW
#define _H_WINDOW

#include <GFX/Interface/IWindow_Impl.h>

namespace Bolt
{
    class Window_Impl;
    class Window
    {
        friend class Window_Impl;
    public:
        static Window createWindow(const WindowCreationParams& params);

        void showWindow() { m_impl->showWindow(); }
        void hideWindow() { m_impl->hideWindow(); }

        //TODO: Surround with plat-spec macro
        const IWindow_Impl_Win* getWindowsWindow() const { return static_cast<IWindow_Impl_Win*>(m_impl); }
    
    private:
        IWindow_Impl*    m_impl = nullptr;
    };
}

#endif