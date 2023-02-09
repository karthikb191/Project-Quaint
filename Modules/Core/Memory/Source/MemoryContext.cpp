#include <MemoryContext.h>
#include <MemCore/AllocationTechniqueFactory.h>
#include <cstdlib>

namespace Quaint
{
    DEFINE_LOG_CATEGORY(MemoryContextLogger);

    bool MemoryContext::Initialize()
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
        sprintf_s(buffer, "Memory Context %s initialization successful! Allocated %lu bytes from OS", m_name, m_size);
        QLOG_I(MemoryContextLogger, buffer);
        
        //TODO: Assert check here
        m_technique = AllocationTechniqueFactory::createAllocationTechique(EAllocationTechnique::Default);
        m_technique->boot(m_name, m_size, m_rawMemory, m_dynamic);

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
            delete m_technique;
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
        sprintf_s(buffer, "Memory Context %s initialization successful! Allocated %lu bytes from OS", m_name, m_size);
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

    void* MemoryContext::Alloc(size_t allocSize)
    {
        //TODO: Add an assert check for m_technique
        void* mem = m_technique->alloc(allocSize);

        //char buffer[1024];
        //sprintf_s(buffer, "Allocated %lu in MemoryContext %s. Available: %lu", allocSize, m_name, m_technique->getAvailableSize());
        //QLOG_E(MemoryContextLogger, buffer);
        return mem;
    }

    void MemoryContext::Free(void* mem)
    {
        //TODO: Add an assert check for m_technique
        m_technique->free(mem);

        char buffer[1024];
        sprintf_s(buffer, "Freed from MemoryContext %s. Available: %lu", m_name, m_technique->getAvailableSize());
        QLOG_E(MemoryContextLogger, buffer);
    }
}