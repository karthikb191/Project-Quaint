#ifndef _H_MEMORY_CONTEXT
#define _H_MEMORY_CONTEXT

#include <stdint.h>

namespace Quaint
{
    class MemoryContext
    {
        public:
            constexpr MemoryContext() : m_name(""), m_size(0), m_valid(false){}
            constexpr MemoryContext(const char* name, uint32_t size) : m_name(name), m_size(size), m_valid(size > 1024){}
        
            const char*           m_name;
            uint32_t        m_size;
            bool            m_valid;
    };
}

#endif //_H_MEMORY_CONTEXT