#include "MemCore/AllocationTechniqueFactory.h"
#include "MemCore/Techniques/DefaultAllocTechnique.h"

namespace Quaint
{
    IAllocationTechnique* AllocationTechniqueFactory::createAllocationTechique(const EAllocationTechnique technique, void* memoryPointer, size_t& techniqueSize)
    {
        //Technique ptrs use default Memory chunk
        switch (technique)
        {
        case EAllocationTechnique::Default:
            DefaultAllocTechnique* allocTechnique = nullptr;//(DefaultAllocTechnique*)memoryPointer;
            allocTechnique = new (memoryPointer) DefaultAllocTechnique();
            techniqueSize = sizeof(DefaultAllocTechnique);
            return allocTechnique;
            break;
        }
        return nullptr;
    }   
}