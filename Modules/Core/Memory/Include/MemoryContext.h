#ifndef _H_MEMORY_CONTEXT
#define _H_MEMORY_CONTEXT

#include <stdint.h>
#include <Memory>
#include <MemCore/MemoryData.h>
#include <MemCore/Techniques/IAllocationTechnique.h>
#include <Interface/IMemoryContext.h>
#include <stdlib.h>

//#define Q_DISABLE_CUSTOM_MEMORY_ALLOCATION

namespace Quaint
{
    class MemoryContext : public IMemoryContext
    {
    public:
        constexpr MemoryContext() 
        : m_name(""), m_size(0), m_valid(false), m_rawMemory(nullptr), m_dynamic(false)
        , m_techniqueType(EAllocationTechnique::Default), m_technique(nullptr)
        {}
        constexpr MemoryContext(const char* name, size_t size, bool dynamic = false, 
        EAllocationTechnique technique = EAllocationTechnique::Default)
            : m_name(name), m_size(size), m_valid(size > 1024), m_dynamic(dynamic), m_rawMemory(nullptr)
            , m_techniqueType(technique), m_technique(nullptr)
        {}

        const char* getContextName() const { return m_name; }
        const size_t getTotalContextSize() const { return m_size; }
        const bool getIsContextValid() const { return m_valid; }
        const size_t getAvailableSize() const { return 0; } //TODO: Fill this up 

        bool Initialize();
        void Invalidate();
        bool Shutdown();
        bool InitializeContextAndTechnique(IAllocationTechnique* technique);
        inline virtual void* Alloc(size_t allocSize) override
        {
#ifndef Q_DISABLE_CUSTOM_MEMORY_ALLOCATION
            //TODO: Add an assert check for m_technique
            void* mem = m_technique->alloc(allocSize);

            //char buffer[1024];
            //sprintf_s(buffer, "Allocated %lu in MemoryContext %s. Available: %lu", allocSize, m_name, m_technique->getAvailableSize());
            //QLOG_E(MemoryContextLogger, buffer);
            return mem;
#else
            return malloc(allocSize);
#endif
        }
        inline virtual void* AllocAligned(size_t allocSize, size_t alignment) override
        {
#ifndef Q_DISABLE_CUSTOM_MEMORY_ALLOCATION
            return m_technique->allocAligned(allocSize, alignment);
#else
            //TODO: _aligned_malloc is MSVC specific. Add wordarounds for other platforms 
            return _aligned_malloc(allocSize, alignment);
#endif
        }

        inline virtual void Free(void* mem) override
        {
#ifndef Q_DISABLE_CUSTOM_MEMORY_ALLOCATION
            //TODO: Add an assert check for m_technique
            if(!m_valid || mem == nullptr)
                return;
            m_technique->free(mem);
#else       
            free(mem);
#endif
            //char buffer[1024];
            //sprintf_s(buffer, "Freed from MemoryContext %s. Available: %lu", m_name, m_technique->getAvailableSize());
            //QLOG_E(MemoryContextLogger, buffer);
        }

        inline virtual size_t GetBlockSize(void* mem) override
        {
            return m_technique->getBlockSize(mem);
        }

        #ifdef _DEBUG
        /* After writing, should return address to the immediate address */
        void* writeMemoryTrackInfo(void* memoryPoiter, size_t contextHeaderoffset, size_t trackerBlocksOffset);
        #endif

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