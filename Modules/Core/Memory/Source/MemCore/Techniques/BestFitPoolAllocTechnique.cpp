#include <MemCore/Techniques/BestFitPoolAllocTechnique.h>
namespace Quaint
{
    void BestFitPoolAllocTechnique::boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic)
    {

    }
    void BestFitPoolAllocTechnique::reboot(size_t size, void* rawMemory)
    {

    }
    void* BestFitPoolAllocTechnique::alloc(size_t allocSize)
    {
        return nullptr;
    }
    void BestFitPoolAllocTechnique::free(void* mem)
    {

    }
    void BestFitPoolAllocTechnique::shutdown()
    {

    }
}