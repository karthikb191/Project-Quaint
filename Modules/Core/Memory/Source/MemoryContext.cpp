#include <MemoryContext.h>
#include <QuaintLogger.h>
#include <MemCore/AllocationTechniqueFactory.h>
#include <cstdlib>

namespace Quaint
{
    DECLARE_LOG_CATEGORY(MemoryContextLogger);
    DEFINE_LOG_CATEGORY(MemoryContextLogger);
    
    bool MemoryContext::Initialize()
    {
        //TODO: Only add asserts here
        if(!m_valid)
        {
            QLOG_E(MemoryContextLogger, "Memory context is in invalid state during initialization");
            return false;
        }

        //TODO: Add an assert here
        if(!RequestMemoryFromOS())
        {
            return false;
        }
        
        char buffer[1024];
        sprintf_s(buffer, "Memory Context %s initialization successful! Allocated %zu bytes from OS", m_name, m_size);
        QLOG_I(MemoryContextLogger, buffer);
        
        //TODO: Assert check here
        //TODO: Instead of using new to create allocator technique, use the memory given by OS
        size_t techniqueSize = 0;
        m_technique = AllocationTechniqueFactory::createAllocationTechique(EAllocationTechnique::Default, m_rawMemory, techniqueSize);
        void* targetMemory = (char*)m_rawMemory + techniqueSize;
        m_technique->boot(m_name, m_size - techniqueSize, targetMemory, m_dynamic);

        return true;
    }

    void MemoryContext::Invalidate()
    {
        if(m_rawMemory != nullptr)
        {
            free(m_rawMemory);
        }
        m_valid = false;
    }

    bool MemoryContext::Shutdown()
    {
        if(m_technique != nullptr)
        {
            m_technique->shutdown();
            // No need to call delete on technique here. managed memory will be cleared in free
        }
        Invalidate();
        return true;
    }

    bool MemoryContext::InitializeContextAndTechnique(IAllocationTechnique* technique)
    {
        if(!m_valid)
        {
            QLOG_E(MemoryContextLogger, "Memory context is in invalid state during initialization");
            return false;
        }

        //TODO: Add an assert here
        if(!RequestMemoryFromOS())
        {
            return false;
        }
        
        char buffer[1024];
        sprintf_s(buffer, "Memory Context %s initialization successful! Allocated %zu bytes from OS", m_name, m_size);
        QLOG_I(MemoryContextLogger, buffer);
        
        //TODO: null check technique
        m_technique = technique;
        m_technique->boot(m_name, m_size, m_rawMemory, m_dynamic);
        return true;
    }

    bool MemoryContext::RequestMemoryFromOS()
    {
        if(!m_valid)
        {
            QLOG_E(MemoryContextLogger, "Memory context is in invalid state during initialization");
            return false;
        }

        //Requests Memory from OS
        m_rawMemory = malloc(m_size);
        if(m_rawMemory == nullptr)
        {
            char buffer[1024];
            sprintf_s(buffer, "Memory Context %s initialzation failed!", m_name);
            QLOG_E(MemoryContextLogger, buffer);
            return false;
        }
        return true;
    }

#ifdef _DEBUG
    void* MemoryContext::writeMemoryTrackInfo(void* memoryPoiter, size_t contextHeaderoffset, size_t trackerBlocksOffset)
    {
        void* headerAddress = (char*)memoryPoiter + contextHeaderoffset;
        void* blocksAddress = (char*)memoryPoiter + trackerBlocksOffset;

        std::vector<TrackerBlock> trackerBlocks;
        size_t numBlocks = m_technique->getTrackerBlocks(trackerBlocks);

        //Fill Context header
        ContextHeader* header = new (headerAddress) ContextHeader();
        header->m_size = m_size;
        header->m_headerSize = sizeof(m_technique->getHeaderSize());
        header->m_numTrackerBlocks = numBlocks;
        header->m_startAddress = (size_t)m_rawMemory;

        size_t blockOffset = sizeof(TrackerBlock);
        void* targetAddress = blocksAddress;
        for(size_t i = 0; i < numBlocks; i++)
        {
            TrackerBlock* block = nullptr;
            block = new (targetAddress) TrackerBlock();
            *block = trackerBlocks[i];
            targetAddress = (char*) targetAddress + blockOffset;
        }

        return targetAddress;
    }
    
#endif

}