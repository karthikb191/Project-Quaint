#ifndef _H_Q_PLAFORM_CRITICAL_SECTION_WIN
#define _H_Q_PLAFORM_CRITICAL_SECTION_WIN
#include "../Interface/IQPlatformCriticalSection.h"
#include <Windows.h>
#include <atomic>

namespace Quaint
{
    class QPlatformCriticalSection : public IQPlatformCriticalSection
    {
    public:
        QPlatformCriticalSection();
        virtual ~QPlatformCriticalSection();
        virtual void enter() override;
        virtual void leave() override;
        virtual void* getPlatformHandle() override { return &m_criticalSection; }

    private:
        CRITICAL_SECTION        m_criticalSection;
        std::atomic<bool>       m_locked = false;
    };
}

#endif //_H_Q_PLAFORM_CRITICAL_SECTION_WIN