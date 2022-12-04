#ifndef _H_MEMORY_DEFINITIONS
#define _H_MEMORY_DEFINITIONS

#include "MemoryConstants.h"
#include "MemoryManager.h"

namespace Quaint
{
    void RegisterMemoryPartitions()
    {
        REGISTER_MEMORY_PARTITION(0, "Default", 20 * 1024);
        REGISTER_MEMORY_PARTITION(1, "Test1", 30 * 1024);
        REGISTER_MEMORY_PARTITION(2, "Test2", 40 * 1024);
        REGISTER_MEMORY_PARTITION(3, "Test3", 50 * 1024);
        REGISTER_MEMORY_PARTITION(4, "Test4", 60 * 1024);
    }
}

#endif //_H_MEMORY_DEFINITIONS