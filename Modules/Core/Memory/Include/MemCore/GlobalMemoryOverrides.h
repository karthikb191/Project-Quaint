#ifndef _H_GLOBAL_MEMORY_OVERRIDES
#define _H_GLOBAL_MEMORY_OVERRIDES
#include <new.h>

//#define Q_DISABLE_CUSTOM_MEMORY_ALLOCATION

namespace Quaint
{
    class IMemoryContext;
}


template<typename T, typename ...ARGS>
T* allocFromContext(Quaint::IMemoryContext* context, ARGS... args)
{
    T* allocPtr = (T*)context->AllocAligned(sizeof(T), alignof(T));
    new(allocPtr)T(args...);
    return allocPtr;
}
template<typename T, typename ...ARGS>
T* allocArrayFromContext(Quaint::IMemoryContext* context, size_t elements, ARGS... args)
{
    T* allocPtr = (T*)context->Alloc(elements * sizeof(T));

    //Calls Constructor with provided args
    for(size_t i = 0; i < elements; i++)
    {
        new(allocPtr + i)T(args...);
    }

    return allocPtr;
}

template<typename T>
void deleteFromContext(Quaint::IMemoryContext* context, T* mem)
{
    mem->~T();
    context->Free(mem);
}
template<typename T>
void deleteArrayFromContext(Quaint::IMemoryContext* context, T* mem)
{
#ifndef Q_DISABLE_CUSTOM_MEMORY_ALLOCATION
    size_t numElems = context->GetBlockSize(mem) / sizeof(T);
    //Loops through and calls destructor
    for(size_t i = 0; i < numElems; i++)
    {
        (mem + i)->~T();
    }
#endif
    context->Free(mem);
}

//#define QUAINT_NEW(TYPE, CONTEXT, ...) new (allocFromContext<TYPE>(CONTEXT)) TYPE (__VA_ARGS__);
#define QUAINT_NEW(CONTEXT, TYPE, ...) allocFromContext<TYPE>(CONTEXT, __VA_ARGS__)
#define QUAINT_NEW_ARRAY(CONTEXT, TYPE, ELEMENTS, ...) allocArrayFromContext<TYPE>(CONTEXT, ELEMENTS, __VA_ARGS__)

#define QUAINT_DELETE(CONTEXT, MEMORY) deleteFromContext(CONTEXT, MEMORY)
#define QUAINT_DELETE_ARRAY(CONTEXT, MEMORY) deleteArrayFromContext(CONTEXT, MEMORY)

#define QUAINT_ALLOC_MEMORY(CONTEXT, SIZE) CONTEXT->Alloc(SIZE)
#define QUAINT_ALLOC_MEMORY_ALIGNED(CONTEXT, SIZE, ALIGNMENT) CONTEXT->AllocAligned(SIZE, ALIGNMENT);
#define QUAINT_DEALLOC_MEMORY(CONTEXT, MEMORY) CONTEXT->Free(MEMORY)

#endif //_H_GLOBAL_MEMORY_OVERRIDES