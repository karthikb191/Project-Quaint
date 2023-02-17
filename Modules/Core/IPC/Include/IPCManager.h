#ifndef _H_IPC_MANAGER
#define _H_IPC_MANAGER

#include <vector>
#include "SharedMemoryTypes.h"
#include "Mapper/MemoryMapper_Win.h" //TODO: CROSS-PLATFORM

namespace Quaint
{
    class IPCModule;

    class IPCManager
    {
        friend class IPCModule;
    public:
        void initialize();
        void shutdown();

        const SharedMemoryHandle* requestSharedMemory(const char* name, const ESharedMemoryType type, const size_t size);
        void releaseSharedMemory(const SharedMemoryHandle& handle);

    private:
        SharedMemoryHandle& getFreeHandle();

        IPCManager() = default;
        virtual ~IPCManager() = default;
        IPCManager(const IPCManager&) = delete;
        IPCManager(const IPCManager&&) = delete;
        IPCManager& operator=(const IPCManager&) = delete;
        IPCManager& operator=(const IPCManager&&) = delete;

        std::vector<SharedMemoryHandle>     m_memoryHandles;
        MemoryMapper_Win                    m_winMemoryMapper;  //TODO CROSS-PLATFORM
    };
}

#endif