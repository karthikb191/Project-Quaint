#ifndef _H_Q_SHARED_PTR
#define _H_Q_SHARED_PTR
#include <memory>

#ifndef USE_CUSTOM_MEMORY_PTRS
 #define USE_CUSTOM_MEMORY_PTRS 0
#endif
#if USE_CUSTOM_MEMORY_PTRS
    #include <memory>
#endif

namespace Bolt
{
#if USE_CUSTOM_MEMORY_PTRS
    class QSharedPtr
    {

    };
#else
    typedef std::shared_ptr QUniquePtr;
#endif
}

#endif //_H_Q_UNIQUE_PTR