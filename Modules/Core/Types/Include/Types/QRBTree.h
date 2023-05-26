#ifndef _H_Q_RB_TREE
#define _H_Q_RB_TREE
#include <Interface/IMemoryContext.h>

/* Properties:
* 1. Every node is either red or black
* 2. Root is black.
* 3. There are no subsequent red nodes. A Red parent has black childred
* 4. No. of black nodes from root to any leaf is equal on all paths
* 5. Every leaf should be black (leaves are sentinels & this rule is easiest to uphold)
*/

// RB-Tree is represented as an array.
// Sentinel occupies 0th index
namespace Quaint
{
template<typename T, typename V = int>
class QRBTree
{
    struct QRBNode
    {
        T         key;
        V         data;
        QRBNode*  parent = nullptr;
        QRBNode*  right = nullptr;
        QRBNode*  left = nullptr;
        bool      isRed = false;
    };

public:
    QRBTree(IMemoryContext* context)
    : m_context(context)
    {
        m_sentinel.left = &m_sentinel;
        m_sentinel.right = &m_sentinel;
        m_sentinel.parent = &m_sentinel;

        m_root = &m_sentinel;
    }
    ~QRBTree()
    {
        deleteSubTree(m_root);
    }

    void deleteSubTree(QRBNode* node)
    {
        if(node == & m_sentinel) return;

        deleteSubTree(node->left);
        deleteSubTree(node->right);

        deleteNode(node);
    }

    /*Creates a new node on heap and inserts into tree*/
    void insert(T key)
    {
        QRBNode* node = createNewNode();
        node->key = key;
        node->isRed = true;

        if(m_root == &m_sentinel)
        {
            m_root = node;
            m_root->isRed = false;
            return;
        }

        QRBNode* current = m_root;
        QRBNode* target = current;
        while(current != &m_sentinel)
        {
            target = current;
            if(key < current->key)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }

        if(key < target->key)
        {
            target->left = node;
        }
        else
        {
            target->right = node;
        }
        node->parent = target;

        insertFixup(node);

    }
    bool remove(T key)
    {
        QRBNode* node = find(key);
        if(node == nullptr)
        {
            return false;
        }
        QRBNode* fixupNode = &m_sentinel;

        bool originallyBlack = !(node->isRed);
        if(node->left == &m_sentinel)
        {
            fixupNode = node->right;
            transplant(node, node->right);
        }
        else if(node->right == &m_sentinel)
        {
            fixupNode = node->left;
            transplant(node, node->left);
        }
        else
        {
            //Node has 2 valid children
            QRBNode* minNode = getMinInSubTree(node->right);
            fixupNode = minNode->right;
            originallyBlack = !(minNode->isRed);

            //This condition is for case when fixupNode is sentinel
            if(minNode->parent == node)
            {
                fixupNode->parent = minNode;
            }
            else
            {
                //minNode's right takes minNode's place
                transplant(minNode, minNode->right);
                
                fixupNode = minNode->right;

                minNode->right = node->right;
                node->right->parent = minNode;
            }
            transplant(node, minNode);
            minNode->left = node->left;
            node->left->parent = minNode;
            minNode->isRed = node->isRed;
        }

        //If the original node is black, we would have introduced multiple violations
        if(originallyBlack)
        {
            removeFixup(fixupNode);
        }

        deleteNode(node);
        return true;
    }

    QRBNode* find(const T& key)
    {
        QRBNode* current = m_root;
        while(current != &m_sentinel)
        {
            if(current->key == key)
            {
                return current;
            }
            else if(key < current->key)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }
        return nullptr;
    }

    void print()
    {
        if(m_root == &m_sentinel)
        {
            std::cout <<"empty tree\n";
        }
        printImpl(m_root, 0);
        std::cout <<"\n";
    }
private:
    /*Transplant links node's parent to other. If node has children, that should be explicitly handled*/
    void transplant(QRBNode* node, QRBNode* other)
    {
        if(node->parent == &m_sentinel)
        {
            m_root = other;
        }
        else if(node->parent->left == node)
        {
            node->parent->left = other;
        }
        else
        {
            node->parent->right = other;
        }

        //It's possible for "other" to be sentinel
        other->parent = node->parent;
        
    }
    void leftRotate(QRBNode* node)
    {
        //Should have a valid right child
        assert(node->right != &m_sentinel);

        QRBNode* r = node->right;

        //node's left remains same. R's right remains same
        r->parent = node->parent;
        node->parent = r;
        if(r->parent == &m_sentinel)
        {
            m_root = r;
        }
        else if(r->parent->left == node)
        {
            r->parent->left = r;
        }
        else
        {
            r->parent->right = r;
        }

        node->right = r->left;
        if(node->right != &m_sentinel)
        {
            node->right->parent = node;
        }
        r->left = node;
    }
    void rightRotate(QRBNode* node)
    {
        assert(node->left != &m_sentinel);

        QRBNode* l = node->left;

        l->parent = node->parent;
        node->parent = l;
        if(l->parent == &m_sentinel)
        {
            m_root = l;
        }
        else if(l->parent->left == node)
        {
            l->parent->left = l;
        }
        else
        {
            l->parent->right = l;
        }

        //node's right and l's left remains same
        node->left = l->right;
        if(node->left != &m_sentinel)
        {
            node->left->parent = node;
        }  
        l->right = node;
    }
    QRBNode* getMinInSubTree(QRBNode* start)
    {
        QRBNode* current = start;
        QRBNode* target = start;
        while(current != &m_sentinel)
        {
            target = current;
            current = current->left;
        }
        return target;
    }
    
    void insertFixup(QRBNode* node)
    {
        while(node->parent->isRed)
        {
            if(node->parent == node->parent->parent->left)
            {
                QRBNode* u = node->parent->parent->right;
                if(u->isRed)
                {
                    u->isRed = false;
                    node->parent->isRed = false;
                    node->parent->parent->isRed = true;

                    node = node->parent->parent;
                }
                else
                {
                    if(node->parent->right == node)
                    {
                        node = node->parent;
                        leftRotate(node);
                    }
                    node->parent->isRed = false;
                    node->parent->parent->isRed = true; 
                    rightRotate(node->parent->parent);
                }
            }
            else
            {
                QRBNode* u = node->parent->parent->left;
                if(u->isRed)
                {
                    u->isRed = false;
                    node->parent->isRed = false;
                    u->parent->isRed = true;

                    node = node->parent->parent;
                }
                else
                {
                    if(node->parent->left == node)
                    {
                        node = node->parent;
                        rightRotate(node);
                    }
                    node->parent->isRed = false;
                    node->parent->parent->isRed = true; 
                    leftRotate(node->parent->parent);
                }
            }
        }

        m_root->isRed = false;
    }

    //node entering this function will be doubly black
    void removeFixup(QRBNode* node)
    {
        while(node != m_root && !node->isRed)
        {
            if(node == node->parent->left)
            {
                QRBNode* sibling = node->parent->right;
                if(node->parent->right->isRed)
                {
                    // If parent's right child is red, parent is black.
                    // Transfer blackness to parent's right child
                    // node will still be doubly black, but this op converts into case - 2
                    sibling->isRed = false;
                    node->parent->isRed = true;
                    leftRotate(node->parent);
                    sibling = node->parent->right;
                }
                //Sibling is black. Parent could be red
                if(!sibling->left->isRed && !sibling->right->isRed)
                {
                    sibling->isRed = true;
                    node = node->parent;
                }
                else
                {
                    //sibling is black(cuz, one of left or right is red)
                    if(sibling->left->isRed)
                    {
                        //This will turn new sibling's left child black and right child red
                        sibling->left->isRed = false;
                        sibling->isRed = true;
                        rightRotate(sibling);
                        sibling = node->parent->right;
                    }

                    sibling->isRed = node->parent->isRed;
                    sibling->right->isRed = false;
                    node->parent->isRed = false;
                    leftRotate(node->parent);
                    node = m_root;
                }
            }
            else
            {
                QRBNode* sibling = node->parent->left;
                if(node->parent->left->isRed)
                {
                    sibling->isRed = false;
                    node->parent->isRed = true;
                    rightRotate(node->parent);
                    sibling = node->parent->left;
                }
                //Sibling is black. Parent could be red
                if(!sibling->left->isRed && !sibling->right->isRed)
                {
                    sibling->isRed = true;
                    node = node->parent;
                }
                else
                {
                    if(sibling->right->isRed)
                    {
                        sibling->right->isRed = false;
                        sibling->isRed = true;
                        leftRotate(sibling);
                        sibling = node->parent->left;
                    }

                    sibling->isRed = node->parent->isRed;
                    sibling->left->isRed = false;
                    node->parent->isRed = false;
                    rightRotate(node->parent);
                    node = m_root;
                }
            }
        }

        node->isRed = false;
    }

    QRBNode* createNewNode()
    {
        if(m_context == nullptr)
        {
            return new QRBNode();
        }

        QRBNode* node = (QRBNode*)m_context->Alloc(sizeof(QRBNode));
        ::new((void*)node) QRBNode();
        node->parent = &m_sentinel;
        node->left = &m_sentinel;
        node->right = &m_sentinel;
        return node;
    }
    void deleteNode(QRBNode* node)
    {
        if(m_context == nullptr)
        {
            delete node;
            return;
        }

        node->~QRBNode();
        m_context->Free(node);
    }

    void printImpl(QRBNode* node, int tabCount)
    {
        if(node == &m_sentinel) return;

        printImpl(node->right, tabCount + 1);
        for(int i = 0; i < tabCount; i++)
        {
            std::cout << "\t";
        }
        std::cout << node->key << (node->isRed ? "r":"b") << "\n";
        printImpl(node->left, tabCount + 1);
    }


    IMemoryContext*     m_context = nullptr;

    QRBNode*            m_root = nullptr;
    QRBNode             m_sentinel;
};
}
#endif