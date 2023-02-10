#include <stdlib.h>
#include <MemoryManager.h>

//TODO: Later Move this to a precompiled header
//TODO: Add operators to handle arrays

void* operator new(size_t size)
{
    return Quaint::MemoryManager::Get().defaultAlloc(size);
}

void operator delete(void* mem)
{
    Quaint::MemoryManager::Get().defaultFree(mem);
}

void* operator new(size_t size, const char* contextName)
{
    //TODO: Fill this up
    return nullptr;
}
void operator delete(void* mem, const char* contextName)
{
    //TODO: Fill this up
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