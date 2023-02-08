#ifndef _H_DEFAULT_ALLOC_TECHNIQUE
#define _H_DEFAULT_ALLOC_TECHNIQUE

#include "IAllocationTechnique.h"

namespace Quaint
{
    class DefaultAllocTechnique : public IAllocationTechnique
    {
    public:
        void boot(size_t size, void* rawMemory, bool dynamic = false) override;
        void reboot(size_t size, void* rawMemory) override;
        void* alloc(size_t allocSize) override;
        void free() override;

    protected:
        bool m_dynamic = false;
    };
}

#endif //_H_DEFAULT_ALLOC_TECHNIQUE