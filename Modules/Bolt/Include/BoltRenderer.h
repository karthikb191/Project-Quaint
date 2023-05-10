#ifndef _H_BOLT
#define _H_BOLT
#include <memory>
#include <iostream>
#include <GFX/Interface/IRenderer.h>
#include <Interface/IMemoryContext.h>

namespace Bolt
{
    class BoltRenderer
    {
    public:
        static BoltRenderer* get()
        {
            if(m_renderer == nullptr)
            {
                m_renderer = std::make_unique<BoltRenderer>();
                std::cout << "Constructed Bolt Renderer" << "\n";
            }
            return m_renderer.get();
        }
        ~BoltRenderer();

        void startEngine(Quaint::IMemoryContext* context);
        void shutdown();

    private:
        IRenderer*                  m_renderer_impl     = nullptr;
        Quaint::IMemoryContext*     m_context           = nullptr;

        static std::unique_ptr<BoltRenderer> m_renderer;
        bool m_engineRunning;
    };
}
#endif //_H_BOLT