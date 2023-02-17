#ifndef _H_MEMORY_MANAGER
#define _H_MEMORY_MANAGER

//TODO: Change to custom type once ready
#include <string>
#include <map>
#include <set>
#include <memory>
#include <Singleton.h>
#include "MemCore/Techniques/DefaultAllocTechnique.h"
#include "MemoryContext.h"
#include "MemoryConstants.h"

using namespace std;

namespace Quaint
{
    class MemoryModule;
    /*Memory Manager lives outside the custom allocated memory blocks. This should be the last thing to be destroyed*/
    //TODO: Find a way to add to one of the memory contexts
    class MemoryManager
    {
        friend class MemoryModule;
    public:
        /*Extremely critical function! There should not be any calls to new or delete. Will result in a deadlock otherwise*/
        bool initialize();
        bool shutdown();
        /*Registers a new memory partition*/
        static constexpr void registerMemoryPartition(uint32_t index, const char* partitionName, size_t size, bool dynamic = false, EAllocationTechnique technique = EAllocationTechnique::Default)
        {
            m_MemoryContexts[index] = MemoryContext(partitionName, size, dynamic);
            ++m_validContexts;
        }
        int              getValidContexts() { return m_validContexts; }
        MemoryContext*   getMemoryContexts() { return m_MemoryContexts; }

        inline void*    defaultAlloc(size_t allocSize) 
        { 
            return m_MemoryContexts[0].Alloc(allocSize); 
        }
        inline void     defaultFree(void* mem) 
        { 
            m_MemoryContexts[0].Free(mem); 
        }

        MemoryContext*          getMemoryContenxtByIndex(uint32_t index);
        MemoryContext*          getMemoryContextByName(const char* name);

    private:
        /*When heap allocation is first requested, boot allocation technique is constructed*/
        MemoryManager() = default;
        ~MemoryManager()
        {
            shutdown();
        };
        
        static int                             m_validContexts;
        static MemoryContext                   m_MemoryContexts[MAX_MEMORY_CONTEXTS];

        bool                                    m_initialized = false;
    };

#define REGISTER_MEMORY_PARTITION(INDEX, PARTITION_NAME, SIZE, DYNAMIC, TYPE) \
            MemoryManager::registerMemoryPartition(INDEX, PARTITION_NAME, SIZE, DYNAMIC, TYPE);
}
#endif //_H_MEMORY_MANAGER