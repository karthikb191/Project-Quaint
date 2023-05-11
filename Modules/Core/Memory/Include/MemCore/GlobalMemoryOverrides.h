#ifndef _H_GLOBAL_MEMORY_OVERRIDES
#define _H_GLOBAL_MEMORY_OVERRIDES
#include <new.h>
namespace Quaint
{
    class IMemoryContext;
}

//TODO: Later move this to a precompiled header

void* operator new(size_t size, const char* contextName);
void operator delete(void* mem, const char* contextName);

void* operator new(size_t size, int contextId);
void operator delete(void* mem, int contextId);

void* operator new(size_t size, Quaint::IMemoryContext* context);
void* operator new[](size_t size, Quaint::IMemoryContext* context);


template<typename T, typename ...ARGS>
T* allocFromContext(Quaint::IMemoryContext* context, ARGS... args)
{
    T* allocPtr = (T*)context->Alloc(sizeof(T));
    new(allocPtr)T(args...);
    return allocPtr;
}

//TODO: Array allocation is a little broken. Address this
template<typename T, typename ...ARGS>
T* allocArrayFromContext(Quaint::IMemoryContext* context, size_t elements, ARGS... args)
{
    T* allocPtr = (T*)context->Alloc(elements * sizeof(T));

    for(size_t i = 0; i < elements; i++)
    {
        new(allocPtr + i)T(args...);
    }

    return allocPtr;
}

template<typename T>
void deleteFromContext(T* mem, Quaint::IMemoryContext* context)
{
    mem->~T();
    context->Free(mem);
}

//TODO: Implement array deallocation

//#define QUAINT_NEW(TYPE, CONTEXT, ...) new (allocFromContext<TYPE>(CONTEXT)) TYPE (__VA_ARGS__);
#define QUAINT_NEW(TYPE, CONTEXT, ...) allocFromContext<TYPE>(CONTEXT, __VA_ARGS__);

#define QUAINT_NEW_ARRAY(CONTEXT, TYPE, ELEMENTS, ...) allocArrayFromContext<TYPE>(CONTEXT, ELEMENTS, __VA_ARGS__);

#define QUAINT_DELETE(MEMORY, CONTEXT) deleteFromContext(MEMORY, CONTEXT);

#endif //_H_GLOBAL_MEMORY_OVERRIDES