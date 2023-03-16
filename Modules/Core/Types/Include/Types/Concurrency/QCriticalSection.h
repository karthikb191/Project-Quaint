#ifndef _H_Q_CRITICAL_SECTION
#define _H_Q_CRITICAL_SECTION

namespace Quaint
{
    class QCriticalSection
    {
    public:
        QCriticalSection() = default;
        ~QCriticalSection() = default;
        
    private:
        QCriticalSection(const QCriticalSection&) = delete;
        QCriticalSection(QCriticalSection&&) = delete;
        QCriticalSection& operator=(const CriticalSection&) = delete;
        QCriticalSection& operator=(CriticalSection&&) = delete;
    };
}

#endif