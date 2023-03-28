#include <Types/Concurrency/Win/QThread_Win.h>
#include <string>
namespace Quaint
{
    QPlatformThread::~QPlatformThread()
    {
        //TODO: Move this to shutdown instead
        TerminateThread(m_threadHandle, 0);
    }

    DWORD WINAPI QPlatformThread::jobCallback(LPVOID lpParam)
    {
        QPlatformThread* winThread = static_cast<QPlatformThread*>(lpParam);
        if(winThread->m_threadParams.m_job == nullptr)
        {
            return -1;
        }

        //Calls Job function
        //TODO: Implement a proper shutdown. Right now, thread completes the job and just returns
        //while(!m_shutdown)
        {
            winThread->m_running = true;
            winThread->m_threadParams.m_job(winThread->m_threadParams.m_jobParam);
            winThread->m_running = false;
            //Thread finished working on this JOB. Go to sleep or exit
        }
        
        return 0;
    }

    bool QPlatformThread::initialize(const ThreadParams& params)
    {
        m_threadParams = params;
        DWORD flags = params.m_threadInitState == EThreadInitState::Started ? 0
                                                : CREATE_SUSPENDED;
        m_threadHandle = CreateThread(NULL, params.m_stackSize, jobCallback, this, flags, &m_threadId);

        if(m_threadHandle == NULL)
        {
            //TODO Log an error message
            return false;
        }
        
        m_running = params.m_threadInitState == EThreadInitState::Started;

        //m_threadHandle = OpenThread(THREAD_ALL_ACCESS, false, m_threadId);
        //if(m_threadHandle == NULL)
        //{
        //    DWORD errorMessageID = ::GetLastError();
        //    if(errorMessageID == 0) {
        //        return false; //No error message has been recorded
        //    }

        //    LPSTR messageBuffer = nullptr;

        //    //Ask Win32 to give us the string version of that message ID.
        //    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        //    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        //                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        //    //Copy the error message into a std::string.
        //    std::string message(messageBuffer, size);

        //    //Free the Win32's string's buffer.
        //    LocalFree(messageBuffer);
        //    return false;
        //}
        
        return true;
    }

    void QPlatformThread::run()
    {
        if(m_threadHandle == NULL)
        {
            return;
        }
        if(m_running)
        {
            return;
        }
        
        ResumeThread(m_threadHandle);
    }

    void QPlatformThread::wait()
    {
        //TODO
    }

    void QPlatformThread::join()
    {
        WaitForSingleObject(m_threadHandle, INFINITE);
    }

    void QPlatformThread::waitOnPredicate(std::function<bool()> predicate)
    {
        
    }

    void QPlatformThread::shutdown()
    {
        m_shutdown = true;
    }
}