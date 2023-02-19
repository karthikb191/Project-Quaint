#include "MemCore/Techniques/DefaultAllocTechnique.h"
#include "MemCore/GlobalMemoryOverrides.h"
#include <QuaintLogger.h>

namespace Quaint
{
    DECLARE_LOG_CATEGORY(DefaultAllocLogger);
    DEFINE_LOG_CATEGORY(DefaultAllocLogger);
    
    void DefaultAllocTechnique::shutdown()
    {
        MemoryChunk* current = m_rootChunk;
        while(current != nullptr)
        {
            current->m_isUsed = false;
            m_availableSize += sizeof(MemoryChunk);
            current = current->m_next;
        }
        char buffer[1024];
        sprintf_s(buffer, "Memory chunk overhead cleared from DefaultAllocTechnique. Total available: %zu", m_availableSize);
        QLOG_E(DefaultAllocLogger, buffer);
        m_isRunning = false;
    }
    
    //TODO: Pass in a memory context here
    void DefaultAllocTechnique::boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic)
    {
        m_availableSize = size;
        //TODO: Assert available size is greater than a certain threshold and power of 2
        if(m_availableSize < 1024)
        {
            //TODO: Throw an assert here. Dont log
            //QLOG_E(DefaultAllocLogger, "Boot Failed! partition size provided is not sufficient");
        }

        //Creates initial Memory chunk
        m_rootChunk = (MemoryChunk*)rawMemory;
        *m_rootChunk = MemoryChunk();
        m_rootChunk->m_size = m_availableSize - sizeof(MemoryChunk);
        m_rootChunk->m_rawData = (char*)rawMemory + sizeof(MemoryChunk);
        m_rootChunk->m_next = nullptr;
        m_currentFree = m_rootChunk;
        m_availableSize -= sizeof(MemoryChunk);
        m_isRunning = true;
    }
    
    void DefaultAllocTechnique::reboot(size_t size, void* rawMemory)
    {
        //TODO: Fill this up
    }

    void DefaultAllocTechnique::mergeUnusedChunks()
    {
        MemoryChunk* current = m_rootChunk;
        MemoryChunk* prevUnused = nullptr;
        size_t maxContiguosBlockSize = 0;
        while (current != nullptr)
        {
            if(!current->m_isUsed)
            {
                if(prevUnused == nullptr)
                {
                    prevUnused = current;
                }
                else
                {
                    // Loop can continue after this cuz we are not modified the data that already exists in current.
                    // current->m_next should still be valid to go into the next iteration
                    prevUnused->m_next = current->m_next;
                    prevUnused->m_size += current->m_size + sizeof(MemoryChunk);    
                }
                if(prevUnused->m_size > maxContiguosBlockSize)
                {
                    maxContiguosBlockSize = prevUnused->m_size;
                    m_currentFree = prevUnused;
                }
            }
            else
            {
                prevUnused = nullptr;
            }

            current = current->m_next;
        }
        
    }

    DefaultAllocTechnique::MemoryChunk* DefaultAllocTechnique::getFirstFitChunk(size_t allocSize)
    {
        if(!m_currentFree->m_isUsed && allocSize <= m_currentFree->m_size)
        {
            return m_currentFree;
        }
        mergeUnusedChunks();
        return m_currentFree;
    }
    
    /*This can return nullptr if freechunk cant be created*/
    DefaultAllocTechnique::MemoryChunk* DefaultAllocTechnique::createFreeChunk(void* memLocation, size_t availableSize)
    {
        if(availableSize < sizeof(MemoryChunk) + 1024)
        {
            QLOG_E(DefaultAllocLogger, "Could not create a free chunk");
            return nullptr;
        }

        MemoryChunk* chunk = new (memLocation) MemoryChunk();
        chunk->m_size = availableSize - sizeof(MemoryChunk);
        chunk->m_rawData = (char*)memLocation + sizeof(MemoryChunk);
        m_availableSize -= sizeof(MemoryChunk);
        return chunk;
    }

    void* DefaultAllocTechnique::alloc(size_t allocSize)
    {
        //If we still can't find current free Chunk, return
        MemoryChunk* chunk = getFirstFitChunk(allocSize);
        //TODO: Assert check free chunk
        if(chunk->m_isUsed)
        {
            QLOG_E(DefaultAllocLogger, "No suitable free chunk could be found");
            return nullptr;
        }

        //Update current free chunk
        size_t remainingSize = chunk->m_size - allocSize;
        m_currentFree = createFreeChunk((char*)chunk->m_rawData + allocSize, remainingSize);

        if(m_currentFree != nullptr)
        {
            //If we are successfully able to create a new chunk, modify allocated chunk's params appropriately
            m_currentFree->m_next = chunk->m_next;
            chunk->m_next = m_currentFree;
            chunk->m_size = allocSize;
        }
        else
        {
            // If we couldn't create a new chunk for some reason, just reset m_currentFree chunk to m_rootChunk.
            // This will be resolved in call to getFirstFreeChunk
            m_currentFree = m_rootChunk;
        }
        chunk->m_isUsed = true;
        m_availableSize -= allocSize;

        return chunk->m_rawData;
    }

    void DefaultAllocTechnique::free(void* mem)
    {
        if(!m_isRunning)
            return;
        MemoryChunk* current = m_rootChunk;
        while(current != nullptr)
        {
            // If memory is encountered, just set the flag to false.
            // Will be reused when mergeUnusedChunks() is called
            if(current->m_rawData == mem)
            {
                current->m_isUsed = false;
                m_availableSize += current->m_size;
                break;
            }
            current = current->m_next;
        }
    }

    size_t DefaultAllocTechnique::getTrackerBlocks(std::vector<TrackerBlock>& trackerBlocks)
    {
        MemoryChunk* current = m_rootChunk;
        size_t numBlocks = 0;

        while(current != nullptr)
        {
            if(current->m_isUsed)
            {
                ++numBlocks;
            }
            current = current->m_next;
        }

        trackerBlocks.reserve(numBlocks);
        
        current = m_rootChunk;
        numBlocks = 0;

        while(current != nullptr)
        {
            if(current->m_isUsed)
            {
                TrackerBlock block;
                sprintf_s(block.m_name, "Temp");
                block.m_size = sizeof(MemoryChunk) + current->m_size;
                block.m_startAddress = *(int*)(current);
                trackerBlocks.emplace_back(block);
                ++numBlocks;
            }
            current = current->m_next;
        }

        return numBlocks;
    }
}