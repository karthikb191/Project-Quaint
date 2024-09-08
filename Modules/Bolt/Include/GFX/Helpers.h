#ifndef _H_HELPERS
#define _H_HELPERS

#include <MemCore/GlobalMemoryOverrides.h>

namespace Quaint
{
    class IMemoryContext;
}

namespace Bolt
{
    template<typename T>
    struct Deleter
    {
        Deleter(Quaint::IMemoryContext* pContext)
        : context(pContext)
        {}
        void operator()(T* ptr)
        {
            QUAINT_DELETE(context, ptr);
        }
        Quaint::IMemoryContext* context = nullptr;
    };

}
#endif