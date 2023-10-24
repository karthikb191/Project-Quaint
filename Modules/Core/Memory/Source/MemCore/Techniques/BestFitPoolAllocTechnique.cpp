#include <MemCore/Techniques/BestFitPoolAllocTechnique.h>
#include <assert.h>

namespace Quaint
{
#define GET_MULTIPLE_OF_ALIGNMENT(ALIGNMENT, DATA) ALIGNMENT * ((DATA + ALIGNMENT - 1) / ALIGNMENT)
#define GET_MULTIPLE_OF_ALIGNMENT_WITH_PADDING(ALIGNMENT, DATA) GET_MULTIPLE_OF_ALIGNMENT(ALIGNMENT, DATA + PADDING_INFO_SIZE)

    namespace RBTree
    {
        //RBTree::RBNode RBNode::sentinel = RBTree::RBNode(0);
        RBTree::RBNode* sentinel = nullptr;
        RBTree::RBNode* root = nullptr;
        void InitTree()
        {
            sentinel = RBNode::GetSentinel();
            sentinel->m_left = sentinel;
            sentinel->m_right = sentinel;
            sentinel->m_parent = sentinel;

            root = sentinel;
            root->m_parent = sentinel;
            root->m_left = sentinel;
            root->m_right = sentinel;
        }

        void LeftRotate(RBNode* x)
        {
            // Pre-condition that must be satisfied here is that
            // X already has a valid right child(Y).
            // Y takes X's place
            //Move parents
            RBNode* y = x->m_right;
            y->m_parent = x->m_parent;
            x->m_parent = y;
            if(y->m_parent == sentinel)
            {
                //std::cout <<"New Root\n";
                root = y;
            }
            else if(y->m_parent->m_right == x)
            {
                y->m_parent->m_right = y;
            }
            else
            {
                y->m_parent->m_left = y;
            }

            //x becomes y's left node. y's left node becomes x's right node
            x->m_right = y->m_left;
            if(x->m_right != sentinel)
            {
                x->m_right->m_parent = x;
            }
            y->m_left = x;
        }
        void RightRotate(RBNode* x)
        {
            // Pre-Condition that must be satisfied here
            // is that X has a vaild left node(Y). 
            // Y takes X's place
            RBNode* y = x->m_left;
            y->m_parent = x->m_parent;
            x->m_parent = y;
            if(y->m_parent == sentinel)
            {
                //std::cout <<"New Root\n";
                root = y;
            }
            else if(y->m_parent->m_right == x)
            {
                y->m_parent->m_right = y;
            }
            else 
            {
                y->m_parent->m_left = y;
            }

            x->m_left = y->m_right;
            if(x->m_left != sentinel)
            {
                x->m_left->m_parent = x;
            }
            y->m_right = x;
        }

        void RBInsertFixup(RBNode* node)
        {
            while(node->m_parent != sentinel && node->m_parent->m_isRed)
            {
                //If Node's parent is red, we have a violation in RB-Tree Property
                if(node->m_parent->m_parent->m_right == node->m_parent)
                {
                    //If the current node's parent is right child
                    //Case - 1: We know that node->parent->parent(NPP) is black. So, if NPP's left child is red, 
                    //we can simply switch colors
                    RBNode* uncle = node->m_parent->m_parent->m_left;
                    if(uncle != sentinel && uncle->m_isRed)
                    {
                        node->m_parent->m_parent->m_isRed = true;
                        uncle->m_isRed = false;
                        node->m_parent->m_isRed = false;
                        node = node->m_parent->m_parent;
                    }
                    // Now we know that NPP's left child is black or nullptr. Now, depending on whether current node is
                    // left or right child, we perform rotations
                    else
                    {
                        //The goal here is to make newly added node as a root to node's parent and parent's parent.
                        if(node->m_parent->m_left == node)
                        {
                            // We right rotate here to transform newly added node as parent.
                            // With this node's parent effectively becomes it's right-child
                            // and Node's parent's parent will become node's parent
                            node = node->m_parent;
                            RightRotate(node);
                        }
                        // Finally Left Rotate on node's parent.
                        // Newly added node will become the new parent.
                        node->m_parent->m_isRed = false;
                        node->m_parent->m_parent->m_isRed = true;
                        LeftRotate(node->m_parent->m_parent);
                    }
                }
                else if(node->m_parent->m_parent->m_left == node->m_parent)
                {
                    //If the current node's parent is left child
                    RBNode* uncle = node->m_parent->m_parent->m_right;
                    if(uncle != sentinel && uncle->m_isRed)
                    {
                        node->m_parent->m_parent->m_isRed = true;
                        uncle->m_isRed = false;
                        node->m_parent->m_isRed = false;
                        node = node->m_parent->m_parent;
                    }
                    else
                    {
                        if(node == node->m_parent->m_right)
                        {
                            node = node->m_parent;
                            LeftRotate(node);
                        }
                        node->m_parent->m_isRed = false;
                        node->m_parent->m_parent->m_isRed = true;
                        RightRotate(node->m_parent->m_parent);
                    }
                }
            }
            root->m_isRed = false;
        }

        //TODO: Don't use new here. Use the memory pool that's available
        void insert(RBNode* node)
        {
            if(root == sentinel)
            {
                //std::cout <<"New Root\n";
                root = node;
                return;
            }

            RBNode* current = root;
            RBNode* target = root;
            while(current != sentinel)
            {
                //assert(current != nullptr);
                target = current;
                if(node->m_size > current->m_size)
                {
                    current = current->m_right;
                }
                else if (node->m_size == current->m_size)
                {
                    if(node > current)
                    {
                        current = current->m_right;
                    }
                    else
                    {
                        current = current->m_left;
                    }
                }
                else
                {
                    current = current->m_left;
                }
            }

            if(node->m_size > target->m_size)
            {
                target->m_right = node;
            }
            else if(node->m_size == target->m_size)
            {
                if(node > target)
                {
                    target->m_right = node;
                }
                else
                {
                    target->m_left = node;
                }
            }
            else
            {
                target->m_left = node;
            }
            node->m_parent = target;
            node->m_isRed = true;
            RBInsertFixup(node);
        }

        void transplant(RBNode* u, RBNode* v)
        {
            if(u->m_parent == sentinel)
            {
                //std::cout <<"New Root\n";
                root = v;
            }
            else if(u->m_parent->m_left == u)
            {
                u->m_parent->m_left = v;
            }
            else
            {
                u->m_parent->m_right = v;
            }

            v->m_parent = u->m_parent;
            
        }

        RBNode* getMinimumInSubTree(RBNode* node)
        {
            RBNode* current = node;
            while(current->m_left != sentinel)
            {
                current = current->m_left;
            }
            return current;
        }

        void RBDeleteFixup(RBNode* node)
        {
            while(node != root && !node->m_isRed)
            {
                if(node->m_parent->m_right == node)
                {
                    //Uncle should always be present
                    RBNode* uncle = node->m_parent->m_left;
                    if(uncle->m_isRed)
                    {
                        uncle->m_isRed = false;
                        uncle->m_parent->m_isRed = true;
                        RightRotate(uncle->m_parent);
                        uncle = node->m_parent->m_left;
                    }
                    //Case - 2: Uncle is black and Left and right children of uncle are null or red
                    if(!uncle->m_left->m_isRed && !uncle->m_right->m_isRed)
                    {
                        uncle->m_isRed = true;
                        // Extra blackness is added to parent, making in RB or BB to compensate for removing 
                        // a black on uncle and node
                        node = node->m_parent; 
                    }
                    else
                    {   
                        //Uncle's right is red, transform to make both left and right as black 
                        if(uncle->m_right->m_isRed)
                        {
                            uncle->m_right->m_isRed = false;
                            uncle->m_isRed = true;
                            LeftRotate(uncle);
                            uncle = node->m_parent->m_left;
                        }
                        uncle->m_isRed = node->m_parent->m_isRed;
                        node->m_parent->m_isRed = false;
                        uncle->m_left->m_isRed = false;

                        RightRotate(node->m_parent);
                        node = root;
                    }
                }
                else
                {
                    //Uncle should always be present
                    RBNode* uncle = node->m_parent->m_right;
                    if(uncle->m_isRed)
                    {
                        uncle->m_isRed = false;
                        uncle->m_parent->m_isRed = true;
                        LeftRotate(uncle->m_parent);
                        uncle = node->m_parent->m_right;
                    }
                    //Case - 2: Uncle is black and Left and right children of uncle are null or red
                    if(!uncle->m_right->m_isRed && !uncle->m_left->m_isRed)
                    {
                        uncle->m_isRed = true;
                        // Extra blackness is added to parent, making in RB or BB to compensate for removing 
                        // a black on uncle and node
                        node = node->m_parent;
                    }
                    else
                    {   
                        //Uncle's left is red, transform to make both left and right as black 
                        if(uncle->m_left->m_isRed)
                        {
                            uncle->m_left->m_isRed = false;
                            uncle->m_isRed = true;
                            RightRotate(uncle);
                            uncle = node->m_parent->m_right;
                        }
                        uncle->m_isRed = node->m_parent->m_isRed;
                        node->m_parent->m_isRed = false;
                        uncle->m_right->m_isRed = false;
                    
                        LeftRotate(node->m_parent);
                        node = root;
                    }
                }
            }
            node->m_isRed = false;
        }
        
        //TODO: Convert this to delete based on Node address
        void remove(RBNode* node)
        {
            RBNode* y = node;
            bool yOriginalColorIsRed = y->m_isRed;

            RBNode* x = sentinel;
            
            if(node->m_left == sentinel)
            {
                x = node->m_right;

                transplant(node, node->m_right);
            }
            else if(node->m_right == sentinel)
            {
                x = node->m_left;
                transplant(node, node->m_left);
            }
            else
            {
                //Node has 2 valid children
                y = getMinimumInSubTree(node->m_right);

                yOriginalColorIsRed = y->m_isRed;
                // we color Y the same color as z.
                // y->right take's Y's place.
                x = y->m_right;
                //Node being deleted is not immediate parent of Y.
                if(y->m_parent == node)
                {
                    //This check is for sentinel. If y has no children, we store the parent info in sentinel
                    x->m_parent = y;
                }
                else
                {
                    //Move X To Y's place
                    transplant(y, x);
                    y->m_right = node->m_right;
                    y->m_right->m_parent = y;
                    
                }
                //Move Y To Z's Place and take it's color
                transplant(node, y);
                y->m_left = node->m_left;
                node->m_left->m_parent = y;
                y->m_isRed = node->m_isRed;
            }

            if(!yOriginalColorIsRed)
            {
                RBDeleteFixup(x);
            }
        }

        RBNode* find(size_t n)
        {
            RBNode* current = root;
            while(current != sentinel)
            {
                if(current->m_size == n)
                {
                    return current;
                }
                if(n > current->m_size)
                {
                    current = current->m_right;
                }
                else
                {
                    current = current->m_left;
                }
            }
            return nullptr;
        }

        bool containsNode(RBNode* node)
        {
            RBNode* current = root;
            while(current != sentinel)
            {
                if(current == node)
                {
                    return true;
                }
                if(node->m_size > current->m_size)
                {
                    current = current->m_right;
                }
                else if(node->m_size == current->m_size)
                {
                    if(node > current)
                    {
                        current = current->m_right;
                    }
                    else
                    {
                        current = current->m_left;
                    }
                }
                else
                {
                    current = current->m_left;
                }
            }
            return false;
        }

#ifdef _DEBUG
        void printTree(RBNode* node, int tabs)
        {
            if(node->m_right != sentinel)
                printTree(node->m_right, tabs + 1);

            for(int i = 0; i < tabs; i++)
                std::cout << "\t";
            std::cout << node->m_size << (node->m_isRed ? "r" : "b") << std::endl;

            if(node->m_left != sentinel)
                printTree(node->m_left, tabs + 1);

        }
        
        void print()
        {
            printTree(root, 0);
        }
#endif
    }

    void BestFitPoolAllocTechnique::boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic)
    {
        RBTree::InitTree();
        m_availableSize = size;
        //TODO: Assert available size is greater than a certain threshold and power of 2
        if(m_availableSize < 1024)
        {
            return;
            //TODO: Throw an assert here. Dont log
            //QLOG_E(DefaultAllocLogger, "Boot Failed! partition size provided is not sufficient");
        }

        //TODO: initialize RB tree with a free node and push it into doubly linked list
        m_endAddress = (char*)rawMemory + size;
        m_root = new (rawMemory) MemoryChunk();
        m_availableSize -= sizeof(MemoryChunk);
        void* treeStartAddress = (char*)rawMemory + sizeof(MemoryChunk);
        
        RBTree::RBNode* node = new (treeStartAddress) RBTree::RBNode(m_availableSize);
        RBTree::insert(node);

        //Creates initial Memory chunk
        m_isRunning = true;
    }

    void BestFitPoolAllocTechnique::reboot(size_t size, void* rawMemory)
    {

    }

    /*Returns ptr to RB-Tree node. Not Memory Chunk*/
    RBTree::RBNode* BestFitPoolAllocTechnique::getBestFit(size_t allocSize)
    {
        RBTree::RBNode* bestFit = RBTree::sentinel;
        RBTree::RBNode* current = RBTree::root;

        while(current != RBTree::sentinel)
        {
            if(current->m_size >= allocSize)
            {
                bestFit = current;
            }

            if(allocSize < current->m_size)
            {
                current = current->m_left;
            }
            else
            {
                current = current->m_right;
            }
        }

        assert (bestFit->m_size >= allocSize && "Invalid best fit node retrieved");

        return (bestFit == RBTree::sentinel) ? nullptr : bestFit;
    }

    void* BestFitPoolAllocTechnique::alloc(size_t allocSize)
    {
        return allocAligned(allocSize);
    }

    void* BestFitPoolAllocTechnique::allocAligned(size_t allocSize, size_t alignment)
    {
        //TODO: Add an assert

        // Retrieved block should fit padding info (4 bytes) and alignment padding 
        // Also, block should have a minimum size of RBTree::RBNode. If this is violated, tree node data might get stomped when merging free chunks
        size_t totalSize = GET_MULTIPLE_OF_ALIGNMENT_WITH_PADDING(alignment, (allocSize + alignment));// alignment * ((allocSize + PADDING_INFO_SIZE + alignment + alignment - 1) / alignment);
        totalSize = totalSize >= sizeof(RBTree::RBNode) ? 
        totalSize : GET_MULTIPLE_OF_ALIGNMENT_WITH_PADDING(alignment, sizeof(RBTree::RBNode) + alignment);

        RBTree::RBNode* bestFit = getBestFit(totalSize);
        if(bestFit == nullptr)
        {
            //TODO: Assert here
            return nullptr;
        }

        assert(bestFit->m_size >= allocSize);
        //We start modifying the space occupied by RB-Tree node. Remove it here before proceeding
        RBTree::remove(bestFit);

        //Get start address as a multiple of alignment
        void* startAddress = (void*)(GET_MULTIPLE_OF_ALIGNMENT(alignment, (size_t)bestFit));// (void*)(alignment * ( (size_t(bestFit) + alignment - 1) / alignment ));
        
        if(((size_t)startAddress - (size_t)bestFit) <= PADDING_INFO_SIZE)
        {
            size_t additional = alignment;
            if(PADDING_INFO_SIZE > alignment)
            {
                additional = GET_MULTIPLE_OF_ALIGNMENT(alignment, PADDING_INFO_SIZE);// alignment * ((PADDING_INFO_SIZE + alignment - 1) / alignment);
            }
            startAddress = (char*)startAddress + additional;
        }
        PADDING_TYPE* padding = (PADDING_TYPE*)startAddress - 1;
        *padding = (size_t)startAddress - (size_t)bestFit;

        assert(*padding < 256 && "Excessive Padding");

        MemoryChunk* chunk = (MemoryChunk*)((char*)bestFit - sizeof(MemoryChunk));
        chunk->m_dataSize = allocSize;
        void* chunkEndAddr = ((char*)chunk + sizeof(MemoryChunk) + totalSize);

        size_t sizeDiff = 0;
        if(chunk->m_right == nullptr)
        {
            sizeDiff = (size_t)m_endAddress - (size_t)chunkEndAddr;
        }
        else
        {
            sizeDiff = (size_t)chunk->m_right - (size_t)chunkEndAddr;
        }

        
        //if(alignment == 1)
        //{
        //    int i = 100;
        //    i -= 10;
        //    std::cout << "!!!!!Padding: " << padding << std::hex << " Best Fit: " << bestFit << " Start: " 
        //    << startAddress << " end: " << chunkEndAddr << "\n";
        //}
        //else
        //{
        //    std::cout << "Padding: " << padding << std::hex << " Best Fit: " << bestFit << " Start: " 
        //    << startAddress << " end: " << chunkEndAddr << "\n";
        //}

        //TODO: Make this a constant
        //If size diff is greater than a certain threshold, split the node 
        //and add a new empty node entry in the tree
        //Split Node
        if(sizeDiff > 1024)
        {
            //std::cout << "\nSize Diff " << sizeDiff << "\n";
            void* targetMemory = (char*)chunk + sizeof(MemoryChunk) + totalSize;
            MemoryChunk* newChunk = new (targetMemory) MemoryChunk();

            newChunk->m_right = chunk->m_right;
            if(newChunk->m_right != nullptr)
            {
                newChunk->m_right->m_left = newChunk;
            }
            chunk->m_right = newChunk;
            newChunk->m_left = chunk;

            void* treeNodeAdd = (char*)(newChunk) + sizeof(MemoryChunk);
            RBTree::RBNode* newNode = new (treeNodeAdd) RBTree::RBNode(sizeDiff);
            RBTree::insert(newNode);

            //if(RBTree::root->m_parent != RBTree::sentinel || sizeDiff > 1024 * 1024 * 1024)
            //{
            //    int iiii = 100;
            //    iiii -= 10;
            //}
        }


        m_availableSize -= totalSize - sizeof(MemoryChunk);

        //assert to check accidental overflow
#ifdef DEBUG_BUILD
        if(chunk->m_right != nullptr)
        {
            assert((size_t)startAddress + allocSize <= (size_t)chunk->m_right && "Data overflow. You might have corrupted data");
        }
        else
        {
            assert((size_t)startAddress + allocSize <= (size_t)m_endAddress && "Data overflow. You might have corrupted data");
        }
#endif

        //std::cout << "\nAlloc " << startAddress << " " << allocSize << " Remaining: " << m_availableSize << "\n";
        return startAddress;
    }

    void BestFitPoolAllocTechnique::free(void* mem)
    {
        size_t freeChunkSize = 0;
        size_t padding = *((PADDING_TYPE*)mem - 1);

        //std::cout << "Padding Free Addr: " << std::hex << (PADDING_TYPE*)mem - 1 << "\n";

        void* freeChunkAddress = (char*)mem - padding;
        MemoryChunk* chunk = (MemoryChunk*)((char*)mem - padding - sizeof(MemoryChunk));

        if(chunk->m_right == nullptr)
        {
            freeChunkSize = (size_t)m_endAddress - (size_t)chunk - sizeof(MemoryChunk);
        }
        else
        {
            freeChunkSize = (size_t)chunk->m_right - (size_t)chunk - sizeof(MemoryChunk);
        }

        if(chunk->m_right != nullptr)
        {
            RBTree::RBNode* node = (RBTree::RBNode*)((char*)chunk->m_right + sizeof(MemoryChunk));
            //chunk->right is free. Merge it
            if(RBTree::containsNode(node))
            {
                chunk->m_right = chunk->m_right->m_right;
                if(chunk->m_right != nullptr)
                {
                    chunk->m_right->m_left = chunk;
                }

                freeChunkSize += node->m_size + sizeof(MemoryChunk);
                m_availableSize += sizeof(MemoryChunk);    

                RBTree::remove(node);
            }
        }
        if(chunk->m_left != nullptr)
        {
            RBTree::RBNode* node = (RBTree::RBNode*)((char*)chunk->m_left + sizeof(MemoryChunk));
            if(RBTree::containsNode(node))
            {
                //Remove current memory
                chunk->m_left->m_right = chunk->m_right;
                if(chunk->m_right != nullptr)
                {
                    chunk->m_right->m_left = chunk->m_left;
                }
                freeChunkAddress = node;

                freeChunkSize += node->m_size + sizeof(MemoryChunk);
                m_availableSize += sizeof(MemoryChunk);

                RBTree::remove(node);
            }
        }

        //TODO: check if this node is already in tree and trigger heap corruption
        RBTree::RBNode* newNode = new (freeChunkAddress) RBTree::RBNode(freeChunkSize);
        RBTree::insert(newNode);

        //std::cout << "\nFree " << mem << " Chunk Size: " << freeChunkSize << " Remaining: " << m_availableSize << "\n";
    }

    size_t BestFitPoolAllocTechnique::getBlockSize(void* mem)
    {
        size_t freeChunkSize = 0;
        size_t padding = *((PADDING_TYPE*)mem - 1);

        void* freeChunkAddress = (char*)mem - padding;
        MemoryChunk* chunk = (MemoryChunk*)((char*)mem - padding - sizeof(MemoryChunk));
        
        return chunk->m_dataSize;
    }

    void BestFitPoolAllocTechnique::shutdown()
    {
        //TODO:
    }
}