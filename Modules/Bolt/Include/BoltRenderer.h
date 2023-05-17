#ifndef _H_BOLT
#define _H_BOLT
#include <memory>
#include <iostream>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>
#include <MemCore/GlobalMemoryOverrides.h>

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
        void shutdown();

    private:
        BoltRenderer();
        ~BoltRenderer();

        IRenderer*                          m_renderer_impl     = nullptr;
        Quaint::IMemoryContext*             m_context           = nullptr;

        bool m_engineRunning;
    };
}
#endif //_H_BOLT