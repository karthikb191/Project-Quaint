#include <Mapper/MemoryMapper_Win.h>
#include <Windows.h>

namespace Quaint
{
    bool MemoryMapper_Win::createMemory(SharedMemoryHandle& handle)
    {
        if(handle.m_type == ESharedMemoryType::SharedOSMemory)
        {
            DWORD size = (DWORD)handle.m_size;
            handle.m_memHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, handle.m_name);
            if(GetLastError() != S_OK)
            {
                return false;
            }

            //Map entirety of the size for now
            handle.m_dataBuffer = MapViewOfFile(handle.m_memHandle, FILE_MAP_ALL_ACCESS, 0, 0, handle.m_size);
            if(handle.m_dataBuffer == NULL)
            {
                return false;
            }
            handle.m_valid = true;
            return true;
        }
        return false;
    }

    bool MemoryMapper_Win::freeMemory(SharedMemoryHandle& handle)
    {
        if(handle.m_dataBuffer != NULL && !UnmapViewOfFile(handle.m_dataBuffer))
        {
            return false;
        }
        if(handle.m_memHandle != NULL && !CloseHandle(handle.m_memHandle))
        {
            return false;
        }
        handle.m_memHandle = nullptr;
        handle.m_dataBuffer = nullptr;
        handle.m_valid = false;
        return true;
    }
}