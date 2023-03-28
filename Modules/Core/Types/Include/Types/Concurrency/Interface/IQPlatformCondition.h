#ifndef _H_Q_PLATFORM_CONDITION
#define _H_Q_PLATFORM_CONDITION
#include <stdint.h>

namespace Quaint
{
    class IQPlatformCondition
    {
    public:
        enum Type
        {
            Binary,
            Counting
        };
        virtual ~IQPlatformCondition(){}
        virtual void wait(uint32_t timeout) = 0;
        virtual void signal() = 0;
        virtual void* getPlatformHandle() = 0;
    
    protected:
        bool        m_waiting = false;
        int         m_count = 0;
    };
}

#endif