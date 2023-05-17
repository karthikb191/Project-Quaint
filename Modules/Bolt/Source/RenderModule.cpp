#include <RenderModule.h>
#include <BoltRenderer.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Bolt
{
    void RenderModule::initModule_impl()
    {
        Module<RenderModule>::initModule_impl();
    }

    void RenderModule::shutdown_impl()
    {
        Module<RenderModule>::shutdown_impl();
        if(m_boltRenderer != nullptr) QUAINT_DELETE(m_context, m_boltRenderer);
    }

    void RenderModule::start(Quaint::IMemoryContext* context)
    {
        m_context = context;
        m_boltRenderer = QUAINT_NEW(context, BoltRenderer);
        m_boltRenderer->startEngine(context);
        //TODO: Create a window here for now and expose it
    }
    void RenderModule::stop()
    {
        m_boltRenderer->shutdown();
    }
}