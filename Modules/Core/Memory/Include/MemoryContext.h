#ifndef _H_MEMORY_CONTEXT
#define _H_MEMORY_CONTEXT

#include <QuaintLogger.h>
#include <stdint.h>
#include "MemCore/Techniques/IAllocationTechnique.h"

namespace Quaint
{
    DECLARE_LOG_CATEGORY(MemoryContextLogger);

    class MemoryContext
    {
        public:
            constexpr MemoryContext() 
            : m_name(""), m_size(0), m_valid(false), m_rawMemory(nullptr), m_dynamic(false)
            , m_technique(nullptr)
            {}
            constexpr MemoryContext(const char* name, size_t size, bool dynamic = false)
             : m_name(name), m_size(size), m_valid(size > 1024), m_dynamic(dynamic), m_rawMemory(nullptr)
             , m_technique(nullptr)
            {}
            bool Initialize();
            void* Alloc(size_t allocSize);
            void Free();

            const char*             m_name;
            void*                   m_rawMemory;
            size_t                  m_size;     /*In Bytes*/
            bool                    m_valid;
            bool                    m_dynamic;  /*Whether Memory of this context can be dynamically adjusted*/
            IAllocationTechnique*   m_technique;
    };
}

#endif //_H_MEMORY_CONTEXT