#ifndef _H_IPC_MODULE
#define _H_IPC_MODULE
#include <Module.h>
#include "IPCManager.h"

namespace Quaint
{
    class IPCModule : public Module<IPCModule>
    {
        BEFRIEND_MODULE(IPCModule)
        public:
            IPCManager* getIPCManager() { return &m_IPCManager; } 
            
        protected:
            void initModule_impl() override;
            void shutdown_impl() override;

        private:
            IPCModule() = default;
            virtual ~IPCModule() = default;
            IPCModule(const IPCModule&) = delete;
            IPCModule(const IPCModule&&) = delete;
            IPCModule& operator=(const IPCModule&) = delete;
            IPCModule& operator=(const IPCModule&&) = delete;

            
            IPCManager      m_IPCManager;
    };
}

#endif //_H_IPC_MODULE