#include <Types/Concurrency/Win/QPlatformCriticalSection_Win.h>

namespace Quaint
{
    QPlatformCriticalSection::QPlatformCriticalSection()
    {
        InitializeCriticalSection(&m_criticalSection);
    }

    QPlatformCriticalSection::~QPlatformCriticalSection()
    {
        if(m_locked.load())
        {
            m_locked.store(false);
            LeaveCriticalSection(&m_criticalSection);
        }
    }

    void QPlatformCriticalSection::enter()
    {
        // Enter cirtical section first, if a second thread tries to enter, it will be blocked and
        // wont be able to modify m_locked
        EnterCriticalSection(&m_criticalSection);
        m_locked.store(true);
    }

    void QPlatformCriticalSection::leave()
    {
        // m_locked is set to false first as the thread currently running is the only one that can possibly modify it
        // Then, leave critical section for other threads to run
        m_locked.store(false);
        LeaveCriticalSection(&m_criticalSection);
    }
}