#include <MemoryManager.h>

#define DEFAULT_MEMORY_SIZE 1024 * 1024 * 100 // 100 MiB
#define DEFAULT_MEMORY_NAME "DEFAULT"

namespace Quaint
{
    DEFINE_LOG_CATEGORY(MemoryManagerLogger);

    MemoryManager *MemoryManager::m_MemoryManager = nullptr;
    MemoryContext MemoryManager::m_MemoryContexts[] = {};
    int MemoryManager::m_validContexts = 0;

    MemoryManager::MemoryManager()
        : m_initialized(false)
    {
    }

    bool MemoryManager::initialize()
    {
        //Create Memory Context map that maps a string to MemoryContext once the custom allocator is working

        //initializes valid memory contexts
        for(int i = 0; i < m_validContexts; i++)
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


    MemoryContext* MemoryManager::getMemoryContenxtByIndex(uint32_t index)
    {
        return nullptr;
    }
    MemoryContext* MemoryManager::getMemoryContextByName(const char* name)
    {
        return nullptr;
    }

}