#ifndef _H_I_Q_PLATFORM_CRITICAL_SECTION
#define _H_I_Q_PLATFORM_CRITICAL_SECTION

namespace Quaint
{
    class IQPlatformCriticalSection
    {
    public:
        virtual ~IQPlatformCriticalSection(){}
        virtual void enter() = 0;
        virtual void leave() = 0;
        virtual void* getPlatformHandle() = 0;
    protected:
    };
}

#endif //_H_I_Q_PLATFORM_CRITICAL_SECTION