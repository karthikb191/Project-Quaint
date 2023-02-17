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
        SharedOSMemory,
        SocketCOM,
        Invalid
    };
    
    struct SharedMemoryHandle
    {
        void init(const char* name, const ESharedMemoryType type, const size_t size)
        {
            sprintf_s(m_name, name);
            m_type = type;
            m_size = size;
        }
        char                            m_name[256] = {0};
        void*                           m_memHandle = nullptr;
        void*                           m_dataBuffer = nullptr;
        ESharedMemoryType               m_type = ESharedMemoryType::SharedOSMemory;
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
        bool operator==(const char* name)
        {
            return strcmp(m_name, name) == 0;
        }
        bool operator!=(const char* name)
        {
            return !(*this == name);
        }
    };
}

#endif //_H_SHARED_MEMORY_TYPES