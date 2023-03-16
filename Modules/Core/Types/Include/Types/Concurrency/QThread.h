#ifndef _H_QTHREAD
#define _H_QTHREAD

//TODO: 
#include "Win/QThread_Win.h"
namespace Quaint
{
    class QThread
    {
    public:
        QThread() = default;
        ~QThread()
        {
            delete m_platformThread;
        }
        QThread(const QThread&) = delete;
        QThread& operator=(const QThread&) = delete;
        //QThread supports moving. There can only ever be a single owner for the thread
        QThread(QThread&&) = default;
        QThread& operator=(QThread&&) = default;

        void initializeThread(const ThreadParams& params );
        void run() { m_platformThread->run(); }
        void wait() { m_platformThread->wait(); }
        void waitOnPredicate(std::function<bool()> predicate) { m_platformThread->waitOnPredicate(predicate); }
        void signal() { m_platformThread->signal(); }
        void shutdown() { m_platformThread->shutdown(); }

            
    private:

        IQPlatformThread* m_platformThread = nullptr;
    };
}

#endif