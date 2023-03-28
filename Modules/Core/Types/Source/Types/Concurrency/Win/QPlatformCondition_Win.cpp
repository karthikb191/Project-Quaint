
#include <Types/Concurrency/Win/QPlatformCondition_Win.h>

namespace Quaint
{
    QPlatformCondition::QPlatformCondition(Type type)
    : m_type(type)
    {
        InitializeConditionVariable(&m_condition);
    }
    
    QPlatformCondition::~QPlatformCondition()
    {
    }

    void QPlatformCondition::wait(uint32_t timeout)
    {
        m_criticalSection.enter();
        {
            m_waiting = true;
            PCRITICAL_SECTION handle = static_cast<PCRITICAL_SECTION>(m_criticalSection.getPlatformHandle());
            //Waits on specified condition and unlocks the Critical section. CS must be locked prior to calling this
            SleepConditionVariableCS(&m_condition, handle, timeout);
            m_criticalSection.leave();
            return;
        }
        m_criticalSection.leave();
    }

    void QPlatformCondition::signal()
    {
        m_criticalSection.enter();
        
        WakeConditionVariable(&m_condition);
        
        m_criticalSection.leave();
    }
}