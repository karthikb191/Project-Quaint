#ifndef _H_I_MEMORY_CONTEXT
#define _H_I_MEMORY_CONTEXT

namespace Quaint
{
    class IMemoryContext
    {
    public:
        inline virtual void* Alloc(size_t allocSize) = 0;
        inline virtual void* AllocAligned(size_t allocSize, size_t alignment) = 0;
        inline virtual void Free(void* mem) = 0;
        inline virtual size_t GetBlockSize(void* mem) = 0;
    };
}

#endif //_H_I_MEMORY_CONTEXT