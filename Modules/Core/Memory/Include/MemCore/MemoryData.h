#ifndef _H_MEMORY_DATA
#define _H_MEMORY_DATA
namespace Quaint
{
    enum class EAllocationTechnique
    {
        Default,
        Invalid
        /*Each allocation technique should be accompanied by a class*/
    };

    struct MemoryChunk
    {
        bool            m_isUsed = false;
        size_t          m_size = 0;
        void*           m_rawData = nullptr;
        MemoryChunk*    m_next = nullptr;
        //TODO: Disable new and delete operations on this struct
    };
}

#endif //_H_MEMORY_DATA