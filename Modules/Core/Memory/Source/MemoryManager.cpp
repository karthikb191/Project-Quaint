#include <MemoryManager.h>
#include <QuaintLogger.h>
#include <MemoryDefinitions.h>

namespace Quaint
{

    DECLARE_LOG_CATEGORY(MemoryManagerLogger);
    DEFINE_LOG_CATEGORY(MemoryManagerLogger);

    MemoryContext MemoryManager::m_MemoryContexts[] = {};
    int MemoryManager::m_validContexts = 0;
    //DefaultAllocTechnique MemoryManager::m_bootAllocTechnique = DefaultAllocTechnique();
    //bool MemoryManager::m_initialized = false;

    bool MemoryManager::initialize()
    {
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
        
        //m_bootAllocTechnique.shutdown();
        //m_MemoryContexts[0].Invalidate();
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