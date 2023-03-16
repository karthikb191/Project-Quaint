#ifndef _H_QPLATFORM_THREAD
#define _H_QPLATFORM_THREAD

#include <functional>

namespace Quaint
{
    enum class EThreadInitState
    {
        Started,
        Suspended
    };
    struct ThreadParams
    {
        using JobType = std::function<void(void*)>;
        typename JobType    m_job = nullptr;
        void*               m_jobParam = nullptr;
        size_t              m_stackSize = 4096; //setting stack size is 4MB
        EThreadInitState    m_threadInitState = EThreadInitState::Started; 
    };
    
    class IQPlatformThread
    {
    public:
        //TODO: Use custom Delegates instead of std::function
        virtual bool initialize(const ThreadParams& params) = 0;
        virtual void run() = 0;
        /*Waits on internal Synchronization primitive. Responsibility of thread to code this implementation*/
        virtual void wait() = 0;
        /*Waits on external Synchronization primitive*/
        virtual void WaitOnExternalSyncPrimitive() = 0;
        virtual void waitOnPredicate(std::function<bool()> predicate) = 0;
        virtual void signal() = 0;
        virtual void shutdown() = 0;

        IQPlatformThread() = default;
        virtual ~IQPlatformThread() = default;
    protected:
        //Copy and Move operations are not valid for platform threads. The Engine thread should complete own these
        IQPlatformThread(const IQPlatformThread&) = delete;
        IQPlatformThread(IQPlatformThread&&) = delete;
        IQPlatformThread& operator=(const IQPlatformThread&) = delete;
        IQPlatformThread& operator=(IQPlatformThread&&) = delete;

        ThreadParams m_threadParams;
    };
}

#endif // _H_QPLATFORM_THREAD