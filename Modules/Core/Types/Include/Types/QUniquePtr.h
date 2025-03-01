#ifndef _H_Q_UNIQUE_PTR
#define _H_Q_UNIQUE_PTR

#ifndef USE_CUSTOM_MEMORY_PTRS
 #define USE_CUSTOM_MEMORY_PTRS 0
#endif

#if !USE_CUSTOM_MEMORY_PTRS
    #include <memory>
#endif

namespace Quaint
{
#if USE_CUSTOM_MEMORY_PTRS
    class QUniquePtr
    {

    };
#else
    template<class _T, class _DELETER = std::default_delete<_T>> 
    using QUniquePtr = std::unique_ptr<_T, _DELETER>;
#endif
}

#endif //_H_Q_UNIQUE_PTR