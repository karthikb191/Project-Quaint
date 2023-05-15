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
        extern RBNode* sentinel;
        extern RBNode* root;
        
        struct RBNode
        {
            RBNode(size_t size) : m_size(size) {}

            // TODO: Bad!! Should probably move this to a RBTree structure
            // This lefts us bypass global variable initialization across multiple translational units and retrieve sentinel on demand
            static RBNode* GetSentinel()
            {
                static RBNode sentinel = RBNode(0);
                return &sentinel;
            }

            RBNode*         m_parent = sentinel;
            RBNode*         m_left = sentinel;
            RBNode*         m_right = sentinel;
            size_t          m_size = 0;
            bool            m_isRed = false;
        };

        void InitTree();
        void LeftRotate(RBNode* x);
        void RightRotate(RBNode* x);
        void RBInsertFixup(RBNode* node);
        //TODO: Don't use new here. Use the memory pool that's available
        void insert(RBNode* node);
        void transplant(RBNode* u, RBNode* v);
        RBNode* getMinimumInSubTree(RBNode* node);
        void RBDeleteFixup(RBNode* node);
        //TODO: Convert this to delete based on Node address
        void remove(RBNode* node);
        RBNode* find(size_t n);
        bool containsNode(RBNode* node);
#ifdef _DEBUG
        void printTree(RBNode* node, int tabs);
        void print();
#endif
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
        RBTree::RBNode* getBestFit(size_t allocSize);

        void*                   m_endAddress;
        MemoryChunk*            m_root = nullptr;
    };
}

#endif //_H_BEST_FIT_POOL_ALLOC_TECHNIQUE