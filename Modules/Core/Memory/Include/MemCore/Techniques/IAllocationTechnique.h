#ifndef _H_I_ALLOCATION_Technique
#define _H_I_ALLOCATION_Technique
#include <vector>
#include <Tracker/MemoryTrackerData.h>
#include <MemoryConstants.h>

namespace Quaint
{
    class IAllocationTechnique
    {
    public:
        virtual void boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic = false) = 0;
        virtual void reboot(size_t size, void* rawMemory) = 0;
        virtual void* alloc(size_t allocSize) = 0;
        virtual void* allocAligned(size_t allocSize, size_t alignment = DEFAULT_ALIGNMENT) = 0;
        virtual void free(void* mem) = 0;
        virtual void shutdown() = 0;
        virtual size_t getHeaderSize() = 0;
        virtual size_t getBlockSize(void* mem) = 0;
    #ifdef _DEBUG
        virtual size_t getTrackerBlocks(std::vector<TrackerBlock>& trackerBlocks) = 0;
    #endif
        size_t getAvailableSize() { return m_availableSize; }
        virtual ~IAllocationTechnique() {};
    protected:
        size_t          m_availableSize = 0;
        bool            m_isRunning = false;
    };
}
#endif //_H_I_ALLOCATION_Technique