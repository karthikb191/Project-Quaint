#include <MemoryContext.h>
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

        //Requests Memory from OS
        m_rawMemory = malloc(m_size);
        if(m_rawMemory == nullptr)
        {
            char buffer[1024];
            sprintf_s(buffer, "Memory Contest %s initialzation failed!", m_name);
            QLOG_E(MemoryContextLogger, buffer);
            return false;
        }
        
        char buffer[1024];
        sprintf_s(buffer, "Memory Contest %s initialization successful! Allocated %lu bytes from OS", m_name, m_size);
        QLOG_E(MemoryContextLogger, buffer);
        
        return true;
    }
}