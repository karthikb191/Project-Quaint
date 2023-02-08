#include <stdlib.h>
#include <MemoryManager.h>
#include <MemCore/MemoryData.h>

void* operator new(size_t size)
{
    return Quaint::MemoryManager::defaultAlloc(size);
}

void operator delete(void* mem)
{
    Quaint::MemoryManager::defaultFree();
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

void* operator new(size_t size, size_t contextId)
{
    //TODO: Fill this up
    return nullptr;
}
void operator delete(void* mem, size_t contextId)
{
    //TODO: Fill this up
}