#include "MemCore/Techniques/DefaultAllocTechnique.h"

namespace Quaint
{
    void DefaultAllocTechnique::boot(size_t size, void* rawMemory, bool dynamic)
    {

    }
    
    void DefaultAllocTechnique::reboot(size_t size, void* rawMemory)
    {

    }
    
    void* DefaultAllocTechnique::alloc(size_t allocSize)
    {
        return nullptr;
    }

    void DefaultAllocTechnique::free()
    {
        
    }
}