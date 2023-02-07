#ifndef _H_MEMORY_MANAGER
#define _H_MEMORY_MANAGER

//TODO: Change to custom type once ready
#include <string>
#include <map>
#include <set>
#include <memory>
#include <QuaintLogger.h>
#include "MemoryContext.h"
#include "MemoryConstants.h"

using namespace std;

namespace Quaint
{
    DECLARE_LOG_CATEGORY(MemoryManagerLogger);

    struct MemoryMap
    {
        public:

        static constexpr MemoryMap* next = nullptr;
    };

    /*Memory Manager lives outside the custom allocated memory blocks. This should be the last thing to be destroyed*/
    //TODO: Find a way to move this to a memory context 
    class MemoryManager
    {
    public:
        static MemoryManager* get() 
        {
            if(m_MemoryManager == nullptr)
            {
                //TODO: New should be using default memory context. Check if that's happening correctly
                m_MemoryManager = new MemoryManager();
            }
            return m_MemoryManager;
        }
        bool initialize();

        /*Registers a new memory partition*/
        static constexpr void registerMemoryPartition(uint32_t index, const char* partitionName, size_t size, bool dynamic = false)
        {
            m_MemoryContexts[index] = MemoryContext(partitionName, size, dynamic);
            ++m_validContexts;
        }
        static int getValidContexts() { return m_validContexts; }
        static MemoryContext* getMemoryContexts() { return m_MemoryContexts; }

        MemoryContext*          getMemoryContenxtByIndex(uint32_t index);
        MemoryContext*          getMemoryContextByName(const char* name);

    private:
        MemoryManager();
        static MemoryManager*       m_MemoryManager;
        static MemoryContext        m_MemoryContexts[MAX_MEMORY_CONTEXTS];
        static int                  m_validContexts;

        bool                        m_initialized;
    };

    #define REGISTER_MEMORY_PARTITION(INDEX, PARTITION_NAME, SIZE) \
            MemoryManager::registerMemoryPartition(INDEX, PARTITION_NAME, SIZE);
}
#endif //_H_MEMORY_MANAGER