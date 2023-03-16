#include <Types/Concurrency/Win/QThread_Win.h>

namespace Quaint
{
    DWORD WINAPI QThread_Win::jobCallback(LPVOID lpParam)
    {
        QThread_Win* winThread = static_cast<QThread_Win*>(lpParam);
        if(winThread->m_threadParams.m_job == nullptr)
        {
            return -1;
        }

        //Calls Job function
        while(true)
        {
            winThread->m_threadParams.m_job(winThread->m_threadParams.m_jobParam);
            
            //Thread finished working on this JOB. Go to sleep or exit
        }
        
        return 0;
    }

    bool QThread_Win::initialize(const ThreadParams& params)
    {
        m_threadParams = params;
        DWORD state = params.m_threadInitState == EThreadInitState::Started ? 0
                                                : CREATE_SUSPENDED;
        m_threadHandle = CreateThread(NULL, params.m_stackSize, jobCallback, this, state, m_threadId);

        if(m_threadHandle == NULL)
        {
            return false;
        }
        OpenThread(NULL, false, *m_threadId);
        return true;
    }

    void QThread_Win::run()
    {
        if(m_threadHandle == NULL)
        {
            return;
        }
        LPDWORD exitCode = nullptr;
        GetExitCodeThread(m_threadHandle, exitCode);
        if(*exitCode == STILL_ACTIVE)
        {
            return;
        }

        ResumeThread(m_threadHandle);
    }

    void QThread_Win::wait()
    {
        //TODO
    }

    void QThread_Win::waitOnPredicate(std::function<bool()> predicate)
    {
        
    }

    void QThread_Win::signal()
    {

    }

    void QThread_Win::shutdown()
    {

    }
}