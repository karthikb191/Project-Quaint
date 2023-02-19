#ifndef _H_MEMORY_TRACKER
#define _H_MEMORY_TRACKER

namespace Quaint
{
    /*
    Memory Tracker data layout is of form
    ---------------------------------------------------------------
    //TODO: Num Contexts
    ------------------------------------- Context0
    //TODO: Alloc Technique size
    Alloc Technique Header Size
    Num Tracker Blocks
    ------------------------- Memory blocks in context
    --------------- Block0
    Name [64 bits]
    Start Address [From Header]
    Size [Actual memory size, excluding header]
    --------------- Block1
    Name [64 bits]
    Start Address [From Header]
    Size [Actual memory size, excluding header]
    ---------------...and so on
    ------------------------------------- Context1
    ...
    ------------------------- Memory blocks in context
    --------------- Block0
    ...
    --------------- Block1
    ...
    ---------------...and so on
    ------------------------------------- ...
    ---------------------------------------------------------------
    */

    //TODO: CROSS-PLATFORM
    #pragma pack(push, 1)
    struct ContextHeader
    {
        //TODO: Add a named mutex for synchronization
        //TODO: Size of these can probably be 2 bytes
        size_t m_headerSize = {0};
        size_t m_numTrackerBlocks = {0};
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct TrackerBlock
    {
        char        m_name[64] = {0};
        size_t      m_startAddress = {0}; //Address of the header
        size_t      m_size = {0};
    };
    #pragma pack(pop)
}

#endif //_H_MEMORY_TRACKER