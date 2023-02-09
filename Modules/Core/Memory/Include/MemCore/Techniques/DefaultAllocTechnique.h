#ifndef _H_DEFAULT_ALLOC_TECHNIQUE
#define _H_DEFAULT_ALLOC_TECHNIQUE

#include "IAllocationTechnique.h"
#include "../MemoryData.h"

namespace Quaint
{
    class DefaultAllocTechnique : public IAllocationTechnique
    {
    public:
        void boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic = false) override;
        void reboot(size_t size, void* rawMemory) override;
        void* alloc(size_t allocSize) override;
        void free(void* mem) override;
        void shutdown() override;

    protected:
        MemoryChunk* createFreeChunk(void* memLocation, size_t availableSize);
        void mergeUnusedChunks();
        /*Tres to get the first free chunk encountered. Must check for whether returned chunk is being used or not*/
        MemoryChunk* getFirstFitChunk(size_t allocSize);

        bool            m_dynamic = false;
        MemoryChunk*    m_rootChunk = nullptr;
        MemoryChunk*    m_currentFree = nullptr;
    };
}

#endif //_H_DEFAULT_ALLOC_TECHNIQUE