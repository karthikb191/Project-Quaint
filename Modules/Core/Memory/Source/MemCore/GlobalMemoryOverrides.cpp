#include <stdlib.h>
#include <MemoryModule.h>

//TODO: Later Move this to a precompiled header
//TODO: Add operators to handle arrays

void* operator new(size_t size)
{
    return Quaint::MemoryModule::get().getMemoryManager().defaultAlloc(size);
}

void* operator new[](size_t size)
{
    return Quaint::MemoryModule::get().getMemoryManager().defaultAlloc(size);
}

void operator delete(void* mem)
{
    Quaint::MemoryModule::get().getMemoryManager().defaultFree(mem);
}

void operator delete[](void* mem)
{
    Quaint::MemoryModule::get().getMemoryManager().defaultFree(mem);
}

void* operator new(size_t size, Quaint::IMemoryContext* context)
{
    return context->Alloc(size);
}
void* operator new[](size_t size, Quaint::IMemoryContext* context)
{
    return context->Alloc(size);
}

void* operator new(size_t size, int contextId)
{
    //TODO: Fill this up
    return nullptr;
}
void operator delete(void* mem, int contextId)
{
    //TODO: Fill this up
}

//Potentially slow operations
void* operator new(size_t size, const char* contextName)
{
    //TODO: Fill this up
    return nullptr;
}
void operator delete(void* mem, const char* contextName)
{
    //TODO: Fill this up
}
