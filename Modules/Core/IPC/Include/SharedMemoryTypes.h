#ifndef _H_SHARED_MEMORY_TYPES
#define _H_SHARED_MEMORY_TYPES

#include <string>

namespace Quaint
{
    enum class ESharedMemoryType
    {
        Pipe,
        DoublePipe,
        SharedFile,
        SharedMemory,
        SocketCOM,
        Invalid
    };

    struct SharedMemoryHandle
    {
        SharedMemoryHandle(const char* name)
        : m_name(name)
        {}
        const char*                     m_name;
        void*                           m_memHandle = nullptr;
        ESharedMemoryType               m_type = ESharedMemoryType::SharedMemory;
        size_t                          m_size = 0;
        bool                            m_valid = false;

        //TODO: Convert this to a ID once there's ID generation system in place
        bool operator==(const SharedMemoryHandle& other)
        {
            return strcmp(other.m_name, m_name) == 0;
        }
        bool operator!=(const SharedMemoryHandle& other)
        {
            return !(*this == other);
        }
    };
}

#endif //_H_SHARED_MEMORY_TYPES