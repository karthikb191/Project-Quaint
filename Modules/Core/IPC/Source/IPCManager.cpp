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
        for(SharedMemoryHandle& handle : m_memoryHandles)
        {
            if(handle.m_valid)
            {
                releaseSharedMemory(handle); 
            }
        }

        m_memoryHandles.clear();
    }

    SharedMemoryHandle& IPCManager::getFreeHandle()
    {
        for(SharedMemoryHandle& handle : m_memoryHandles)
        {
            if(!handle.m_valid)
            {
                return handle;
            }
        }
        m_memoryHandles.emplace_back(SharedMemoryHandle());
        return m_memoryHandles[m_memoryHandles.size() - 1];
    }

    const SharedMemoryHandle* IPCManager::requestSharedMemory(const char* name, const ESharedMemoryType type, const size_t size)
    {
        auto it = std::find(m_memoryHandles.begin(), m_memoryHandles.end(), name);

        if(it != m_memoryHandles.end())
        {
            //TODO: Convert this to ASSERT and return.
            QLOG_E(IPCManagerLogger, "Requested shared memory already exists!");
            return nullptr;
        }

        SharedMemoryHandle& handle = getFreeHandle();
        
        handle.init(name, type, size);
        if(m_winMemoryMapper.createMemory(handle))
        {
            QLOG_I(IPCManagerLogger, "Shared Memory Creation Successful");
        }
        else
        {
            QLOG_E(IPCManagerLogger, "Shared Memory Creation failed!");
            return nullptr;
        }

        return &handle;
    }

    void IPCManager::releaseSharedMemory(const SharedMemoryHandle& handle)
    {
        auto it = std::find(m_memoryHandles.begin(), m_memoryHandles.end(), handle);
        
        if(it != m_memoryHandles.end())
        {
            //TODO: Add logic here to actually free memory
            char buffer[256];
            if(m_winMemoryMapper.freeMemory(*it))
            {
                sprintf_s(buffer, "Could not free release shared memory %s!!", it->m_name);
                QLOG_E(IPCManagerLogger, buffer);
            }
            else
            {
                sprintf_s(buffer, "Released shared memory %s!!", it->m_name);
                QLOG_E(IPCManagerLogger, buffer);
            }
        }
    }
}