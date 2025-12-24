#ifndef _H_BEST_FIT_POOL_ALLOC_TECHNIQUE
#define _H_BEST_FIT_POOL_ALLOC_TECHNIQUE
#include "IAllocationTechnique.h"
#include "MemoryConstants.h"
#include <iostream>

namespace Quaint
{
    namespace RBTree
    {
        struct RBNode;
        //extern RBNode* sentinel;
        //extern RBNode* root;
        
        struct RBNode
        {
            RBNode(size_t size) : m_size(size) {}

            RBNode*         m_parent = nullptr;
            RBNode*         m_left = nullptr;
            RBNode*         m_right = nullptr;
            size_t          m_size = 0;
            bool            m_isRed = false;
        };

        struct RBTree
        {
            RBNode sentinel = RBNode(0);
            RBNode* root = &sentinel;
        };
    }

    class BestFitPoolAllocTechnique : public IAllocationTechnique
    {
        struct MemoryChunk
        {
            MemoryChunk*    m_left = nullptr;
            MemoryChunk*    m_right = nullptr;
            size_t          m_dataSize = 0;
        };

    public:
        void boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic = false) override;
        void reboot(size_t size, void* rawMemory) override;
        void* alloc(size_t allocSize) override;
        void* allocAligned(size_t allocSize, size_t alignment = DEFAULT_ALIGNMENT) override;
        void free(void* mem) override;
        size_t getBlockSize(void* mem) override;
        void shutdown() override;
        size_t getHeaderSize() { return sizeof(MemoryChunk); };
    #ifdef _DEBUG
        size_t getTrackerBlocks(std::vector<TrackerBlock>& trackerBlocks) { return 0; }
    #endif
        virtual ~BestFitPoolAllocTechnique() {};
    
    private:
        void initTree();
        RBTree::RBNode* getBestFit(size_t allocSize);

        void*                   m_endAddress;
        MemoryChunk*            m_root = nullptr;
        RBTree::RBTree          m_freeMemoryTree;
    };
}

#endif //_H_BEST_FIT_POOL_ALLOC_TECHNIQUE