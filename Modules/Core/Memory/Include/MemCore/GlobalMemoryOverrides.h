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
    size_t numElems = context->GetBlockSize(mem) / sizeof(T);
    //Loops through and calls destructor
    for(size_t i = 0; i < numElems; i++)
    {
        (mem + i)->~T();
    }
    context->Free(mem);
}

//#define QUAINT_NEW(TYPE, CONTEXT, ...) new (allocFromContext<TYPE>(CONTEXT)) TYPE (__VA_ARGS__);
#define QUAINT_NEW(CONTEXT, TYPE, ...) allocFromContext<TYPE>(CONTEXT, __VA_ARGS__)
#define QUAINT_NEW_ARRAY(CONTEXT, TYPE, ELEMENTS, ...) allocArrayFromContext<TYPE>(CONTEXT, ELEMENTS, __VA_ARGS__)

#define QUAINT_DELETE(CONTEXT, MEMORY) deleteFromContext(CONTEXT, MEMORY)
#define QUAINT_DELETE_ARRAY(CONTEXT, MEMORY) deleteArrayFromContext(CONTEXT, MEMORY)

#define QUAINT_ALLOC_MEMORY(CONTEXT, SIZE) context->Alloc(SIZE)
#define QUAINT_DEALLOC_MEMORY(CONTEXT, MEMORY) context->Free(MEMORY)

#endif //_H_GLOBAL_MEMORY_OVERRIDES