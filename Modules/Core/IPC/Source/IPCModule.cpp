#include<IPCModule.h>

namespace Quaint
{

    void IPCModule::initModule_impl()
    {
        m_IPCManager.initialize();
    }

    void IPCModule::shutdown_impl()
    {
        m_IPCManager.shutdown();
    }

}