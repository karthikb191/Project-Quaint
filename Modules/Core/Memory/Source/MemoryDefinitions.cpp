#include <MemoryDefinitions.h>
#include <MemoryConstants.h>
#include <MemoryManager.h>

namespace Quaint
{
    void RegisterMemoryPartitions()
    {   
        //Best fit pool alloc technique currently uses a global RB-Tree. This needs to be refactored!
        REGISTER_MEMORY_PARTITION(0, BOOT_MEMORY_NAME, BOOT_MEMORY_SIZE, false, EAllocationTechnique::BestFitPoolAllocTechnique);
        REGISTER_MEMORY_PARTITION(MEDIA_MEMORY_INDEX, MEDIA_MEMORY_NAME, MEDIA_MEMORY_SIZE, false, EAllocationTechnique::Default);
        //REGISTER_MEMORY_PARTITION(0, BOOT_MEMORY_NAME, BOOT_MEMORY_SIZE, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(2, "Test2", 40 * 1024, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(3, "Test3", 50 * 1024, false, EAllocationTechnique::Default);
        REGISTER_MEMORY_PARTITION(4, "Test4", 60 * 1024, false, EAllocationTechnique::Default);
    }
}