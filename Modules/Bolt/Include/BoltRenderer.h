#ifndef _H_BOLT
#define _H_BOLT
#include <memory>
#include <iostream>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <MemCore/GlobalMemoryOverrides.h>

//TODO: This should be moved to Application Module
#include <GFX/Window.h>

namespace Bolt
{
    class RenderModule;

    class BoltRenderer
    {
        friend class RenderModule;
        
        template<typename T, typename ...ARGS>
        friend T* ::allocFromContext(Quaint::IMemoryContext* context, ARGS...);
        template<typename T>
        friend void ::deleteFromContext(Quaint::IMemoryContext* context, T* mem);

    public:

        void startEngine(Quaint::IMemoryContext* context);
        void update();
        void shutdown();

        const Window& getWindow() { return m_window; }
        IRenderer* GetRenderer() { return m_renderer_impl; }
    private:
        BoltRenderer();
        ~BoltRenderer();

        IRenderer*                          m_renderer_impl     = nullptr;
        Quaint::IMemoryContext*             m_context           = nullptr;

        Window                              m_window;

        bool m_engineRunning;
    };
}
#endif //_H_BOLT