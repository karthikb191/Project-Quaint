#include <Types/Concurrency/QThread.h>

namespace Quaint
{
    void QThread::initializeThread(const ThreadParams& params )
    {
        m_platformThread.initialize(params);
    }
}