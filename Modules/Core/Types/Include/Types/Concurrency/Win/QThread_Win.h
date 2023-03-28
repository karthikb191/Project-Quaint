#ifndef _H_QTHREAD_WIN
#define _H_QTHREAD_WIN

//TODO: Include a windows compile check

#include "../Interface/IQPlatformThread.h"
#include <windows.h>

namespace Quaint
{
    class QPlatformThread : public IQPlatformThread 
    {
    public:
        virtual ~QPlatformThread();
        virtual bool initialize(const ThreadParams& params) override;
        virtual void run() override;
        virtual void wait() override;
        virtual void join() override;
        virtual void WaitOnExternalSyncPrimitive() override {};
        virtual void waitOnPredicate(std::function<bool()> predicate) override;
        virtual void shutdown() override;

    private:
        static DWORD WINAPI jobCallback(LPVOID lpParam);
        
        DWORD           m_threadId;
        HANDLE          m_threadHandle;
        bool            m_running = false;
        bool            m_shutdown = false;
        //TODO: Add a synchroniztion primitive
    };
}

#endif //_H_QTHREAD_WIN