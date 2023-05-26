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
