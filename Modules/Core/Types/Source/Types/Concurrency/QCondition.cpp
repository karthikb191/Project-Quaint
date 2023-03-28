#include <Types/Concurrency/QCondition.h>

namespace Quaint
{
    void QCondition::wait(uint32_t timeout)
    {
        m_platformCondition.wait();
    }

    void QCondition::signal()
    {
        m_platformCondition.signal();
    }
}