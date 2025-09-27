#include <stdlib.h>
#include <MemoryModule.h>
#include <EASTL/internal/config.h>
#include <EABase/config/eacompilertraits.h>

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
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
     return Quaint::MemoryModule::get().getMemoryManager().defaultAlloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return Quaint::MemoryModule::get().getMemoryManager().defaultAllocAligned(size, alignment);
    
    //TODO: Add proper bypasses to use system allocated memory
    //EA_UNUSED(alignmentOffset); EA_UNUSED(flags);
//
    //size_t adjustedAlignment = (alignment > EA_PLATFORM_PTR_SIZE) ? alignment : EA_PLATFORM_PTR_SIZE;
//
    //void* p = new char[size + adjustedAlignment + EA_PLATFORM_PTR_SIZE];
    //void* pPlusPointerSize = (void*)((uintptr_t)p + EA_PLATFORM_PTR_SIZE);
    //void* pAligned = (void*)(((uintptr_t)pPlusPointerSize + adjustedAlignment - 1) & ~(adjustedAlignment - 1));
//
    //void** pStoredPtr = (void**)pAligned - 1;
    //EASTL_ASSERT(pStoredPtr >= p);
    //*(pStoredPtr) = p;
//
    //EASTL_ASSERT(((size_t)pAligned & ~(alignment - 1)) == (size_t)pAligned);
//
    //return pAligned;
}

void operator delete(void* mem)
{
    Quaint::MemoryModule::get().getMemoryManager().defaultFree(mem);
}

void operator delete[](void* mem)
{
    Quaint::MemoryModule::get().getMemoryManager().defaultFree(mem);
}
