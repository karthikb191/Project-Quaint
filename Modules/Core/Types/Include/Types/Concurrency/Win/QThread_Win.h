#ifndef _H_QTHREAD_WIN
#define _H_QTHREAD_WIN

//TODO: Include a windows compile check

#include "../IQPlatformThread.h"
#include <windows.h>

namespace Quaint
{
    class QThread_Win : public IQPlatformThread 
    {
    public:
        bool initialize(const ThreadParams& params) override;
        void run() override;
        void wait() override;
        void WaitOnExternalSyncPrimitive() override {};
        void waitOnPredicate(std::function<bool()> predicate) override;
        void signal() override;
        void shutdown() override;

    private:
        static DWORD WINAPI jobCallback(LPVOID lpParam);
        
        LPDWORD         m_threadId;
        HANDLE          m_threadHandle;
        //TODO: Add a synchroniztion primitive
    };
}

#endif //_H_QTHREAD_WIN