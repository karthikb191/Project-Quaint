#ifndef _H_WINDOW
#define _H_WINDOW

namespace Bolt
{
    class Window_Impl;
    class Window
    {
    public:
        static Window createWindow();



    private:
        
        Window_Impl*    m_impl = nullptr;
    };
}

#endif