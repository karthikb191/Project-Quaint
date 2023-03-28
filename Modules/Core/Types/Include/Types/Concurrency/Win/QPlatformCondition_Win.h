#ifndef _Q_PLATFORM_CONDITION_WIN
#define _Q_PLATFORM_CONDITION_WIN

#include "../Interface/IQPlatformCondition.h"
#include "QPlatformCriticalSection_Win.h"
#include <windows.h>

namespace Quaint
{
    class QPlatformCondition : public IQPlatformCondition
    {
    public:
        QPlatformCondition(Type type = Type::Binary);
        virtual ~QPlatformCondition();

        void wait(uint32_t timeout = 0xFFFFFFFF);
        void signal();
        virtual void* getPlatformHandle() override { return &m_condition; }

    private:
        CONDITION_VARIABLE          m_condition;
        QPlatformCriticalSection    m_criticalSection;
        Type                        m_type;
    };
}

#endif