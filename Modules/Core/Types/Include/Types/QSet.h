#ifndef _H_Q_SET
#define _H_Q_SET
#include <Types/QRBTree.h>
#include <Interface/IMemoryContext.h>

namespace Quaint
{
    /*Contains unique elements*/
    template<typename Key>
    class QSet
    {
    public:
        typedef Key                                             key_type;
        typedef Key                                             value_type;
        typedef size_t                                          size_type;
        typedef Key&                                            reference;
        typedef const Key&                                      const_reference;
        typedef typename QRBTree<Key>::Iterator                 Iterator;
        typedef typename const QRBTree<Key>::Iterator           Const_Iterator;
        typedef typename QRBTree<Key>::Iterator                 Reverse_Iterator;
        typedef typename const QRBTree<Key>::Iterator           Const_Reverse_Iterator;

        QSet(IMemoryContext* context)
        : m_context(context)
        , m_tree(context)
        {}
        QSet& operator=(const std::initializer_list<Key>& list)
        {
            for(size_t i = 0; i < list.size(); i++)
            {
                insert(*(list.begin() + i));
            }
            return *this;
        }

        void insert(const Key& key)
        {
            m_tree.insert(std::move(key));
        }

        template<typename ...Args>
        void emplace(Args... args)
        {
            m_tree.insert(std::move(Key(args...)));
        }

        bool contains(const Key& key)
        {
            return m_tree.find(key) != nullptr;
        }

        void clear()
        {
            erase(begin(), end());
        }

        void erase(const Key& key)
        {
            m_tree.remove(m_tree.find(key));
        }

        void erase(Iterator& first, Iterator& last)
        {
            for(auto itr = first; itr != last; )
            {
                itr.erase();
            }
        }

        size_t getSize()
        {
            return m_tree.getSize();
        }
        bool empty()
        {
            return m_tree.isEmpty();
        }

//#ifdef DEBUG_BUILD
        void print()
        {
            m_tree.print();
        }
//#endif

    //Iterator Functions---------------------------------------
        Iterator begin()
        {
            QRBTree<Key>::Iterator itr(&m_tree, true);
            ++itr;
            return itr;
        }
        Const_Iterator cbegin() const
        {
            QRBTree<Key>::Iterator itr(&m_tree, true);
            ++itr;
            return itr;
        }
        Reverse_Iterator rbegin()
        {
            QRBTree<Key>::Iterator itr(&m_tree, false);
            ++itr;
            return itr;
        }
        Const_Reverse_Iterator crbegin() const
        {
            QRBTree<Key>::Iterator itr(&m_tree, false);
            ++itr;
            return itr;
        }
        Iterator end()
        {
            return QRBTree<Key>::Iterator(&m_tree);
        }
        Const_Iterator cend() const
        {
            return QRBTree<Key>::Iterator(&m_tree);
        }
        Reverse_Iterator rend()
        {
            return QRBTree<Key>::Iterator(&m_tree, false);
        }
        Const_Reverse_Iterator crend() const
        {
            return QRBTree<Key>::Iterator(&m_tree, false);
        }
    //-------------------------------------------------------

    private:
        IMemoryContext*         m_context = nullptr;      
        QRBTree<Key>            m_tree;
    };
}

#endif