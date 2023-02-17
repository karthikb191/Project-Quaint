#ifndef _H_MEMORY_MAPPER_WIN
#define _H_MEMORY_MAPPER_WIN

#include "../SharedMemoryTypes.h"

namespace Quaint
{
    class MemoryMapper_Win
    {
    public:
        /*
        * These functions read the name from shared handle and update its shared memory and valdity
        */
        bool createMemory(SharedMemoryHandle& handle);
        bool freeMemory(SharedMemoryHandle& handle);
        
        MemoryMapper_Win() = default;
        ~MemoryMapper_Win() = default;
    private:
        MemoryMapper_Win(const MemoryMapper_Win&) = delete;
        MemoryMapper_Win(const MemoryMapper_Win&&) = delete;
        MemoryMapper_Win& operator=(const MemoryMapper_Win&) = delete;
        MemoryMapper_Win& operator=(const MemoryMapper_Win&&) = delete;
    };
}

#endif //_H_MEMORY_MAPPER_WIN