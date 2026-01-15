#ifndef _H_HELPERS
#define _H_HELPERS

#include <MemCore/GlobalMemoryOverrides.h>
#include <type_traits>

namespace Quaint
{
    class IMemoryContext;
}

namespace Quaint
{
    template<typename T>
    struct Deleter
    {
        Deleter(Quaint::IMemoryContext* pContext)
        : context(pContext)
        {}

        template<typename P, typename std::enable_if<
                            std::is_base_of<T, P>::value
                            , bool>::type = true
                >
        Deleter(const Deleter<P>& other)
        {
            context = other.context;
        }
        
        template<typename P, typename std::enable_if<
                            std::is_base_of<T, P>::value
                            , bool>::type = true
                >
        Deleter(Deleter<P>&& other)
        {
            context = other.context;
        }
        
        void operator()(T* ptr)
        {
            if(ptr == nullptr)
            {
                return;
            }
            
            QUAINT_DELETE(context, ptr);
        }
        Quaint::IMemoryContext* context = nullptr;
    };

}
#endif