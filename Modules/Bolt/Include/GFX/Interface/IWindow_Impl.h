#ifndef _I_WINDOW_IMPL_H
#define _I_WINDOW_IMPL_H
#include <Windows.h>
//TODO: Move this to application module under Quaint namespace
namespace Bolt
{
    struct WindowCreationParams
    {
        char*       windowname  = "Default Window";
        char*       className   = "Default Class";
        //TODO: Surround this with a Plat-Spec macro
        WNDPROC     callback    = nullptr;
        size_t      width       = 400;
        size_t      height      = 400;
        size_t      top         = 200;
        size_t      left        = 200;
        //TODO: Add option to have multiple window styles
    };

    class IWindow_Impl
    {
    public:
        virtual bool createWindow(const WindowCreationParams& params) = 0;
        virtual void showWindow() = 0;
        virtual void hideWindow() = 0;
    };

    //TODO: Surround with plat-spec macro
    class IWindow_Impl_Win : public IWindow_Impl
    {
    public:
        virtual HWND getWindowHandle() const = 0;
        virtual HINSTANCE getHInstance() const = 0;
    };
}

#endif