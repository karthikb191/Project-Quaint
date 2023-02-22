#include "MemCore/AllocationTechniqueFactory.h"
#include "MemCore/Techniques/DefaultAllocTechnique.h"
#include "MemCore/Techniques/BestFitPoolAllocTechnique.h"

namespace Quaint
{
    IAllocationTechnique* AllocationTechniqueFactory::createAllocationTechique(const EAllocationTechnique technique, void* memoryPointer, size_t& techniqueSize)
    {
        //Technique ptrs use default Memory chunk
        switch (technique)
        {
        case EAllocationTechnique::Default:
        {
            DefaultAllocTechnique* allocTechnique = nullptr;
            allocTechnique = new (memoryPointer) DefaultAllocTechnique();
            techniqueSize = sizeof(DefaultAllocTechnique);
            return allocTechnique;
        }
        break;
        case EAllocationTechnique::BestFitPoolAllocTechnique:
        {
            BestFitPoolAllocTechnique* allocTechnique = nullptr;
            allocTechnique = new (memoryPointer) BestFitPoolAllocTechnique();
            techniqueSize = sizeof(BestFitPoolAllocTechnique);
            return allocTechnique;
        }
        break;
        }
        return nullptr;
    }   
}