#ifndef _H_MEMORY_MANAGER
#define _H_MEMORY_MANAGER

//TODO: Change to custom type once ready
#include <string>
#include <map>
#include <set>
#include <memory>
#include <Singleton.h>
#include <QuaintLogger.h>
#include "MemCore/Techniques/DefaultAllocTechnique.h"
#include "MemoryContext.h"
#include "MemoryConstants.h"

using namespace std;

namespace Quaint
{
    DECLARE_LOG_CATEGORY(MemoryManagerLogger);

    /*Memory Manager lives outside the custom allocated memory blocks. This should be the last thing to be destroyed*/
    //TODO: Find a way to add to one of the memory contexts
    class MemoryManager
    {
        DECLARE_SINGLETON(MemoryManager);
        
    public:
        static bool initialize();
        static bool shutdown();
        /*Registers a new memory partition*/
        static constexpr void registerMemoryPartition(uint32_t index, const char* partitionName, size_t size, bool dynamic = false, EAllocationTechnique technique = EAllocationTechnique::Default)
        {
            m_MemoryContexts[index] = MemoryContext(partitionName, size, dynamic);
            ++m_validContexts;
        }
        static int              getValidContexts() { return m_validContexts; }
        static MemoryContext*   getMemoryContexts() { return m_MemoryContexts; }

        static void*    defaultAlloc(size_t allocSize);
        static void     defaultFree(void* mem);

        static MemoryContext*          getMemoryContenxtByIndex(uint32_t index);
        static MemoryContext*          getMemoryContextByName(const char* name);

    private:
        MemoryManager() = delete;
        ~MemoryManager() = delete;
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager(const MemoryManager&&) = delete;
        
        static DefaultAllocTechnique    m_bootAllocTechnique;
        static MemoryContext            m_MemoryContexts[MAX_MEMORY_CONTEXTS];
        static int                      m_validContexts;

        static bool                     m_initialized;
    };

    #define REGISTER_MEMORY_PARTITION(INDEX, PARTITION_NAME, SIZE, DYNAMIC, TYPE) \
            MemoryManager::registerMemoryPartition(INDEX, PARTITION_NAME, SIZE, DYNAMIC, TYPE);
}
#endif //_H_MEMORY_MANAGER