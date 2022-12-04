#include <MemoryManager.h>

#define DEFAULT_MEMORY_SIZE 1024 * 1024 * 100 // 100 MiB
#define DEFAULT_MEMORY_NAME "DEFAULT"

namespace Quaint
{
    MemoryManager *MemoryManager::m_MemoryManager = nullptr;
    MemoryContext MemoryManager::m_MemoryContexts[] = {};
    int MemoryManager::m_validContexts = 0;

    MemoryManager::MemoryManager()
        : m_initialized(false)
    {
    }

    bool MemoryManager::initialize()
    {
        m_MemoryManager = new MemoryManager();
        return true;
    }

}