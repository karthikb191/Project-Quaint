#include <MemoryManager.h>

#define DEFAULT_MEMORY_SIZE 1024 * 1024 * 100 // 100 MiB
#define DEFAULT_MEMORY_NAME "DEFAULT"

namespace Quaint
{
    DEFINE_LOG_CATEGORY(MemoryManagerLogger);
    DEFINE_SINGLETON(MemoryManager);

    MemoryContext MemoryManager::m_MemoryContexts[] = {};
    DefaultAllocTechnique MemoryManager::m_bootAllocTechnique = DefaultAllocTechnique();
    int MemoryManager::m_validContexts = 0;
    bool MemoryManager::m_initialized = false;

    bool MemoryManager::initialize()
    {
        //Create Memory Context map that maps a string to MemoryContext once the custom allocator is working

        if(m_validContexts == 0)
        {
            QLOG_E(MemoryManagerLogger, "No memory contexts registered. We need atleast 1. Populate them in MemoryDefinitions.h");
            return false;
        }
        //initializes valid memory contexts
        m_MemoryContexts[0].InitializeContextAndTechnique(&m_bootAllocTechnique);
        for(int i = 1; i < m_validContexts; i++)
        {
            bool res = m_MemoryContexts[i].Initialize();
            if(!res)
            {
                QLOG_E(MemoryManagerLogger, "Memory conext initialization failed! Bailing on MemoryManager Initialization");
                return false;
            }
        }

        m_initialized = true;

        QLOG_V(MemoryManagerLogger, "MemoryManager Initialization successful");
        return true;
    }

    bool MemoryManager::shutdown()
    {
        for(int i = 1; i < m_validContexts; i++)
        {
            m_MemoryContexts[i].Shutdown();
        }
        
        m_bootAllocTechnique.shutdown();
        m_MemoryContexts[0].Invalidate();
        return true;
    }

    void* MemoryManager::defaultAlloc(size_t allocSize)
    {
        return m_MemoryContexts[0].Alloc(allocSize);
    }

    void MemoryManager::defaultFree(void* mem)
    {
        m_MemoryContexts[0].Free(mem);
    }

    MemoryContext* MemoryManager::getMemoryContenxtByIndex(uint32_t index)
    {
        return nullptr;
    }

    MemoryContext* MemoryManager::getMemoryContextByName(const char* name)
    {
        return nullptr;
    }

}