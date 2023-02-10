#include <MemoryDefinitions.h>
#include <MemoryConstants.h>
#include <MemoryManager.h>

namespace Quaint
{
    void RegisterMemoryPartitions()
    {
        REGISTER_MEMORY_PARTITION(0, "Test0", 1000 * 1024, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(1, "Test1", 30 * 1024, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(2, "Test2", 40 * 1024, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(3, "Test3", 50 * 1024, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(4, "Test4", 60 * 1024, false, EAllocationTechnique::Default);
    }
}