#ifndef _H_MEMORY_CONTEXT
#define _H_MEMORY_CONTEXT

#include <stdint.h>
#include <Memory>
#include <MemCore/MemoryData.h>
#include <MemCore/Techniques/IAllocationTechnique.h>

namespace Quaint
{
    class MemoryContext
    {
    public:
        constexpr MemoryContext() 
        : m_name(""), m_size(0), m_valid(false), m_rawMemory(nullptr), m_dynamic(false)
        , m_techniqueType(EAllocationTechnique::Default), m_technique(nullptr)
        {}
        constexpr MemoryContext(const char* name, size_t size, bool dynamic = false, 
        EAllocationTechnique technique = EAllocationTechnique::Default)
            : m_name(name), m_size(size), m_valid(size > 1024), m_dynamic(dynamic), m_rawMemory(nullptr)
            , m_techniqueType(EAllocationTechnique::Default), m_technique(nullptr)
        {}

        const char* getContextName() const { return m_name; }
        const size_t getTotalContextSize() const { return m_size; }
        const bool getIsContextValid() const { return m_valid; }
        const size_t getAvailableSize() const { return 0; } //TODO: Fill this up 

        bool Initialize();
        void Invalidate();
        bool Shutdown();
        bool InitializeContextAndTechnique(IAllocationTechnique* technique);
        inline void* Alloc(size_t allocSize)
        {
            //TODO: Add an assert check for m_technique
            void* mem = m_technique->alloc(allocSize);

            //char buffer[1024];
            //sprintf_s(buffer, "Allocated %lu in MemoryContext %s. Available: %lu", allocSize, m_name, m_technique->getAvailableSize());
            //QLOG_E(MemoryContextLogger, buffer);
            return mem;
        }
        inline void Free(void* mem)
        {
            //TODO: Add an assert check for m_technique
            if(!m_valid)
                return;
            m_technique->free(mem);

            //char buffer[1024];
            //sprintf_s(buffer, "Freed from MemoryContext %s. Available: %lu", m_name, m_technique->getAvailableSize());
            //QLOG_E(MemoryContextLogger, buffer);
        }
    private:
        bool RequestMemoryFromOS();
    
        const char*             m_name;
        size_t                  m_size;     /*In Bytes*/
        void*                   m_rawMemory;
        bool                    m_valid;
        bool                    m_dynamic;  /*Whether Memory of this context can be dynamically adjusted*/
        EAllocationTechnique    m_techniqueType;
        IAllocationTechnique*   m_technique;
    };
}

#endif //_H_MEMORY_CONTEXT