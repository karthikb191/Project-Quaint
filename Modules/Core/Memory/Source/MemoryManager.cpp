#include <MemoryManager.h>
#include <QuaintLogger.h>
#include <MemoryDefinitions.h>

namespace Quaint
{

    DECLARE_LOG_CATEGORY(MemoryManagerLogger);
    DEFINE_LOG_CATEGORY(MemoryManagerLogger);

    MemoryContext MemoryManager::m_MemoryContexts[] = {};
    int MemoryManager::m_validContexts = 0;

    bool MemoryManager::initialize()
    {
        //WARN: DONOT create logs before
        if(m_initialized)
        {
            return true;
        }

        RegisterMemoryPartitions();
        //Create Memory Context map that maps a string to MemoryContext once the custom allocator is working
        if(m_validContexts == 0)
        {
            QLOG_E(MemoryManagerLogger, "No memory contexts registered. We need atleast 1. Populate them in MemoryDefinitions.h");
            return false;
        }
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

    bool MemoryManager::shutdown()
    {
        for(int i = 0; i < m_validContexts; i++)
        {
            m_MemoryContexts[i].Shutdown();
        }
        return true;
    }

    MemoryContext* MemoryManager::getMemoryContenxtByIndex(uint32_t index)
    {
        assert((int)index < m_validContexts && "Invalid index passed for memory context retrieval");
        return &m_MemoryContexts[index];
    }

    MemoryContext* MemoryManager::getMemoryContextByName(const char* name)
    {
        for(int i = 0; i< m_validContexts; ++i)
        {
            if(strcmp(m_MemoryContexts[i].getContextName(), name) == 0)
            {
                return &m_MemoryContexts[i]; 
            }
        }
        
        assert(false && "No valid context found");
        return nullptr;
    }

#ifdef _DEBUG
    void MemoryManager::populateTrackerInformation(void* sharedMemory)
    {
        //TODO: Populating only the first context for now. Add support to populate all contexts later
        m_MemoryContexts[0].writeMemoryTrackInfo(sharedMemory, 0, sizeof(ContextHeader));
    }
#endif
}