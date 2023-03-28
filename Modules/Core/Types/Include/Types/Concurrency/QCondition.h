#ifndef _H_Q_CONDITION
#define _H_Q_CONDITION

#include <Types/Concurrency/Win/QPlatformCondition_Win.h> 

namespace Quaint
{
    class QCondition
    {
    public:

        void wait(uint32_t timeout = 0xFFFFFFFF);
        void signal();
        void* getPlatformHandle() { return m_platformCondition.getPlatformHandle(); }

    private:
        QPlatformCondition      m_platformCondition;
    };
}

#endif //_H_Q_CONDITION