#include "MemCore/AllocationTechniqueFactory.h"
#include "MemCore/Techniques/DefaultAllocTechnique.h"

namespace Quaint
{
    IAllocationTechnique* AllocationTechniqueFactory::createAllocationTechique(EAllocationTechnique technique)
    {
        //Technique ptrs use default Memory chunk
        switch (technique)
        {
        case EAllocationTechnique::Default:
            return new DefaultAllocTechnique();
            break;
        }
        return nullptr;
    }   
}