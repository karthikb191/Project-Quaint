#ifndef _H_Q_CRITICAL_SECTION
#define _H_Q_CRITICAL_SECTION

#include "Win/QPlatformCriticalSection_Win.h"

namespace Quaint
{
    class QCriticalSection
    {
    public:
        QCriticalSection();
        ~QCriticalSection();
        
        void enter();
        void leave();

    private:
        QCriticalSection(const QCriticalSection&) = delete;
        QCriticalSection(QCriticalSection&&) = delete;
        QCriticalSection& operator=(const QCriticalSection&) = delete;
        QCriticalSection& operator=(QCriticalSection&&) = delete;

        QPlatformCriticalSection        m_platformCriticalSection;    
    };
}

#endif