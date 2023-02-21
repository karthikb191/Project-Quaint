#ifndef _H_BEST_FIT_POOL_ALLOC_TECHNIQUE
#define _H_BEST_FIT_POOL_ALLOC_TECHNIQUE
#include "IAllocationTechnique.h"
#include <iostream>
namespace Quaint
{
    struct RBNode
    {
        RBNode(int data) : m_data(data) {}
        RBNode*     m_parent = { nullptr };
        RBNode*     m_left = { nullptr };
        RBNode*     m_right = { nullptr };
        int         m_data = { 0 };
        bool        m_isRed = false;
    };

    struct RBTree
    {
        void LeftRotate(RBNode* x)
        {
            // Pre-condition that must be satisfied here is that
            // X already has a valid right child(Y).
            // Y takes X's place
            //Move parents
            RBNode* y = x->m_right;
            y->m_parent = x->m_parent;
            x->m_parent = y;
            if(y->m_parent == nullptr)
            {
                m_root = y;
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
            if(x->m_right != nullptr)
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
            if(y->m_parent == nullptr)
            {
                m_root = y;
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
            if(x->m_left != nullptr)
            {
                x->m_left->m_parent = x;
            }
            y->m_right = x;
        }

        void RBFixup(RBNode* node)
        {
            while(node->m_parent != nullptr && node->m_parent->m_parent != nullptr && node->m_parent->m_isRed)
            {
                //If Node's parent is red, we have a violation in RB-Tree Property
                if(node->m_parent->m_parent->m_right == node->m_parent)
                {
                    //If the current node's parent is right child
                    //Case - 1: We know that node->parent->parent(NPP) is black. So, if NPP's left child is red, 
                    //we can simply switch colors
                    RBNode* uncle = node->m_parent->m_parent->m_left;
                    if(uncle != nullptr && uncle->m_isRed)
                    {
                        node->m_parent->m_parent->m_isRed = true;
                        node->m_parent->m_parent->m_left->m_isRed = false;
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
                    if(uncle != nullptr && uncle->m_isRed)
                    {
                        node->m_parent->m_parent->m_isRed = true;
                        node->m_parent->m_parent->m_right->m_isRed = false;
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
            m_root->m_isRed = false;
        }

        //TODO: Don't use new here. Use the memory pool that's available
        void insert(int data)
        {
            RBNode* newNode = new RBNode(data);
            if(m_root == nullptr)
            {
                m_root = newNode;
                return;
            }

            RBNode* current = m_root;
            RBNode* target = m_root;
            while(current != nullptr)
            {
                target = current;
                if(newNode->m_data > current->m_data)
                {
                    current = current->m_right;
                }
                else
                {
                    current = current->m_left;
                }
            }

            if(newNode->m_data > target->m_data)
            {
                target->m_right = newNode;
            }
            else
            {
                target->m_left = newNode;
            }
            newNode->m_parent = target;
            newNode->m_isRed = true;
            RBFixup(newNode);
        }


        RBNode* m_root = nullptr;
#ifdef _DEBUG
        void print()
        {
            printTree(m_root, 0);
        }
private:
        void printTree(RBNode* node, int tabs)
        {
            if(node->m_right != nullptr)
                printTree(node->m_right, tabs + 1);

            for(int i = 0; i < tabs; i++)
                std::cout << "\t";
            std::cout << node->m_data << (node->m_isRed ? "r" : "b") << std::endl;

            if(node->m_left != nullptr)
                printTree(node->m_left, tabs + 1);

        }
#endif
    };
    
    class BestFitPoolAllocTechnique : public IAllocationTechnique
    {
    public:
        void boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic = false) override;
        void reboot(size_t size, void* rawMemory) override;
        void* alloc(size_t allocSize) override;
        void free(void* mem) override;
        void shutdown() override;
        size_t getHeaderSize() override;
    #ifdef _DEBUG
        size_t getTrackerBlocks(std::vector<TrackerBlock>& trackerBlocks) { return 0; }
    #endif
        virtual ~BestFitPoolAllocTechnique() {};
    private:

    };
}

#endif //_H_BEST_FIT_POOL_ALLOC_TECHNIQUE