#ifndef _H_BEST_FIT_POOL_ALLOC_TECHNIQUE
#define _H_BEST_FIT_POOL_ALLOC_TECHNIQUE
#include "IAllocationTechnique.h"
#include <iostream>
namespace Quaint
{
    
    struct RBTree;
    struct RBNode;
    extern RBNode m_sentinel;
    struct RBNode
    {
        RBNode(int data) : m_data(data) {}
        RBNode*     m_parent = &m_sentinel;
        RBNode*     m_left = &m_sentinel;
        RBNode*     m_right = &m_sentinel;
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
            if(y->m_parent == &m_sentinel)
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
            if(x->m_right != &m_sentinel)
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
            if(y->m_parent == &m_sentinel)
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
            if(x->m_left != &m_sentinel)
            {
                x->m_left->m_parent = x;
            }
            y->m_right = x;
        }

        void RBInsertFixup(RBNode* node)
        {
            while(node->m_parent != &m_sentinel && node->m_parent->m_parent != &m_sentinel && node->m_parent->m_isRed)
            {
                //If Node's parent is red, we have a violation in RB-Tree Property
                if(node->m_parent->m_parent->m_right == node->m_parent)
                {
                    //If the current node's parent is right child
                    //Case - 1: We know that node->parent->parent(NPP) is black. So, if NPP's left child is red, 
                    //we can simply switch colors
                    RBNode* uncle = node->m_parent->m_parent->m_left;
                    if(uncle != &m_sentinel && uncle->m_isRed)
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
                    if(uncle != &m_sentinel && uncle->m_isRed)
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
            m_root->m_isRed = false;
        }

        //TODO: Don't use new here. Use the memory pool that's available
        void insert(int data)
        {
            RBNode* newNode = new RBNode(data);
            if(m_root == &m_sentinel)
            {
                m_root = newNode;
                return;
            }

            RBNode* current = m_root;
            RBNode* target = m_root;
            while(current != &m_sentinel)
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
            RBInsertFixup(newNode);
        }

        void transplant(RBNode* u, RBNode* v)
        {
            if(u->m_parent == &m_sentinel)
            {
                m_root = v;
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
            while(current->m_left != &m_sentinel)
            {
                current = current->m_left;
            }
            return current;
        }

        void RBDeleteFixup(RBNode* node)
        {
            while(node != m_root && !node->m_isRed)
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
                        node = m_root;
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
                        node = m_root;
                    }
                }
            }
            node->m_isRed = false;
        }
        
        //TODO: Convert this to delete based on Node address
        void remove(RBNode* node)
        {
            static RBNode sentinel(-1);
            RBNode* y = node;
            bool yOriginalColorIsRed = y->m_isRed;

            RBNode* x = &m_sentinel;
            
            if(node->m_left == &m_sentinel)
            {
                x = node->m_right;
                transplant(node, node->m_right);
            }
            else if(node->m_right == &m_sentinel)
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
                //Move Y To Z'a Place and take it's color
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

        RBNode* find(int n)
        {
            RBNode* current = m_root;
            while(current != &m_sentinel)
            {
                if(current->m_data == n)
                {
                    return current;
                }
                if(n > current->m_data)
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


        RBNode* m_root = &m_sentinel;
#ifdef _DEBUG
        void print()
        {
            printTree(m_root, 0);
        }
private:
        void printTree(RBNode* node, int tabs)
        {
            if(node->m_right != &m_sentinel)
                printTree(node->m_right, tabs + 1);

            for(int i = 0; i < tabs; i++)
                std::cout << "\t";
            std::cout << node->m_data << (node->m_isRed ? "r" : "b") << std::endl;

            if(node->m_left != &m_sentinel)
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