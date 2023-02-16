#ifndef _H_ALLOCATION_TECHNIQUE_FACTORY
#define _H_ALLOCATION_TECHNIQUE_FACTORY

#include <memory>
#include "Techniques/IAllocationTechnique.h"
#include "MemoryData.h"

namespace Quaint
{
    class AllocationTechniqueFactory
    {
    public:
        static IAllocationTechnique* createAllocationTechique(const EAllocationTechnique technique, void* memoryPointer, size_t& techniqueSize);

    private:
        AllocationTechniqueFactory() = delete;
        ~AllocationTechniqueFactory() = delete;
        AllocationTechniqueFactory(const AllocationTechniqueFactory&) = delete;
        AllocationTechniqueFactory(const AllocationTechniqueFactory&&) = delete;
        AllocationTechniqueFactory& operator =(const AllocationTechniqueFactory&) = delete;
        AllocationTechniqueFactory& operator =(const AllocationTechniqueFactory&&) = delete;
    };
}
#endif //_H_ALLOCATION_TECHNIQUE_FACTORY