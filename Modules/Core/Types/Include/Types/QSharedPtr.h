#ifndef _H_Q_SHARED_PTR
#define _H_Q_SHARED_PTR
#include <memory>

#ifndef USE_CUSTOM_MEMORY_PTRS
 #define USE_CUSTOM_MEMORY_PTRS 0
#endif
#if !USE_CUSTOM_MEMORY_PTRS
    #include <memory>
    #include "Deleter.h"
#endif

namespace Quaint
{
#if USE_CUSTOM_MEMORY_PTRS
    class QSharedPtr
    {

    };
#else
    template<class _T> 
    using QSharedPtr = std::shared_ptr<_T>;

    template<typename _T>
    QSharedPtr<_T> makeShared(_T* t, Deleter<_T> deleter)
    {
        std::shared_ptr<_T> sharedPtr = std::shared_ptr<_T>(t, deleter)
        return sharedPtr;
    }
    
    template<typename _T>
    QSharedPtr<_T> makeShared(Quaint::IMemoryContext* context)
    {
        std::shared_ptr<_T> sharedPtr = std::shared_ptr<_T>(nullptr, Deleter<_T>(context));
        return sharedPtr;
    }

#endif
}

#endif //_H_Q_UNIQUE_PTR