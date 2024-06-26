#ifndef _H_RENDER_MODULE
#define _H_RENDER_MODULE

#include <Module.h>
#include <Interface/IMemoryContext.h>

namespace Bolt
{
    class BoltRenderer;
    class SomeRandonClass;
    class RenderModule : public Quaint::Module<::Bolt::RenderModule>
    {
        BEFRIEND_MODULE(::Bolt::RenderModule);

    public:
        void start(Quaint::IMemoryContext* context);
        void stop();
        BoltRenderer* getBoltRenderer() { return m_boltRenderer; }
        
        void SomeRandomFunction(const SomeRandonClass& rc);
        Quaint::IMemoryContext* getMemoryContext() { return m_context; }
    protected:
        void initModule_impl() override;
        void shutdown_impl() override;

    private:
        Quaint::IMemoryContext*     m_context       = nullptr;
        BoltRenderer*               m_boltRenderer  = nullptr;

    };
}

#endif