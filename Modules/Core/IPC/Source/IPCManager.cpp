#include <IPCManager.h>
#include <algorithm>
#include <QuaintLogger.h>

namespace Quaint
{
    DECLARE_LOG_CATEGORY(IPCManagerLogger);
    DEFINE_LOG_CATEGORY(IPCManagerLogger);

    void IPCManager::initialize()
    {
        m_memoryHandles.reserve(10);
    }

    void IPCManager::shutdown()
    {

    }

    const SharedMemoryHandle& IPCManager::requestSharedMemory(const char* name, const size_t size)
    {
        SharedMemoryHandle handle(name);
        
        auto it = std::find(m_memoryHandles.begin(), m_memoryHandles.end(), handle);

        if(it != m_memoryHandles.end())
        {
            //TODO: ASSERT and return. There's already an active handle
            QLOG_E(IPCManagerLogger, "Requested shared memory already exists!");
            return handle;
        }

        //TODO: Add logic here to actually request shared memory

        m_memoryHandles.emplace_back(handle);
        return handle;
    }

    void IPCManager::releaseSharedMemory(const SharedMemoryHandle& handle)
    {
        auto it = std::find(m_memoryHandles.begin(), m_memoryHandles.end(), handle);
        
        if(it != m_memoryHandles.end())
        {
            //TODO: Add logic here to actually free memory

            m_memoryHandles.erase(it);
        }
    }
}