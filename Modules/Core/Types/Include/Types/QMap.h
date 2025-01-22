#ifndef _H_Q_MAP
#define _H_Q_MAP
#include <ostream>
#include <Types/QRBTree.h>
#include <Interface/IMemoryContext.h>

namespace Quaint
{
    template<typename First, typename Second>
    struct QPair
    {
        First first;
        Second second;

        bool operator ==(const First& key)
        {
            return first == key;
        }
        bool operator ==(const QPair<First,Second>& other)
        {
            return this->operator == (other.first);
        }
        bool operator!=(const First& key)
        {
            return first != key;
        }
        bool operator !=(const QPair<First,Second>& other)
        {
            return this->operator != (other.first);
        }
        bool operator <(const First& key)
        {
            return first < key;
        }
        bool operator <(const QPair<First,Second>& other)
        {
            return this->operator < (other.first);
        }
        bool operator >(const First& key)
        {
            return first > key;
        }
        bool operator >(const QPair<First,Second>& other)
        {
            return this->operator > (other.first);
        }

        friend std::ostream& operator<<(std::ostream& os, const QPair& pair)
        {
            os << pair.first << " : "<< pair.second;
            return os; 
        }
    };

    template<typename Key, typename Data>
    class QMap
    {
    public:
        typedef Key                                                             key_type;
        typedef Data                                                            value_type;
        typedef size_t                                                          size_type;
        typedef Data&                                                           reference;
        typedef const Data&                                                     const_reference;
        typedef typename QRBTree<QPair<Key, Data>>::Iterator                    Iterator;
        typedef typename const QRBTree<QPair<Key, Data>>::Iterator              Const_Iterator;
        typedef typename QRBTree<QPair<Key, Data>>::Iterator                    Reverse_Iterator;
        typedef typename const QRBTree<QPair<Key, Data>>::Iterator              Const_Reverse_Iterator;

        QMap(IMemoryContext* context)
        : m_context(context)
        , m_tree(context)
        {}

        QMap(const std::initializer_list<QPair<Key, Data>>& list)
        {
            for(size_t i = 0; i < list.size(); i++)
            {
                insert(*(list.begin() + i));
            }
            return *this;
        }

        template<typename TKey, typenamt TData>
        QMap(const QMap<TKey, TData>& other)
        {
            clear();
            for(auto& entry : other)
            {
                insert(entry);
            }
        }

        template<typename TKey, typenamt TData>
        QMap(QMap<TKey, TData>&& other)
        {
            clear();
            m_context = other.m_context;
            m_tree = other.m_tree;
        }

        ~QMap()
        {
            clear();
        }

        template<typename TKey, typenamt TData>
        QMap& operator=(const QMap<TKey, TData>& other)
        {
            clear();
            for(auto& entry : other)
            {
                insert(entry);
            }
        }

        template<typename TKey, typenamt TData>
        QMap& operator=(QMap<TKey, TData>&& other)
        {
            clear();
            m_context = other.m_context;
            m_tree = other.m_tree;
        }

        QMap& operator=(const std::initializer_list<QPair<Key, Data>>& list)
        {
            for(size_t i = 0; i < list.size(); i++)
            {
                insert(*(list.begin() + i));
            }
            return *this;
        }

        void insert(const QPair<Key, Data>& pair)
        {
            m_tree.insert(std::move(pair));
        }

        //TODO: Add an emplace function
        //template<typename ...Args>
        //void emplace(Args... args)
        //{
        //    m_tree.insert(std::move(Key(args...)));
        //}

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

        Data& at(Key key)
        {
            QPair<Key, Data>& data = m_tree.getData(key);
            return data.second;
        }

        Data& operator[](Key key)
        {
            return at(key);
        }

        //Iterator Functions---------------------------------------
        Iterator begin()
        {
            Iterator itr(&m_tree, true);
            ++itr;
            return itr;
        }
        Const_Iterator cbegin() const
        {
            Iterator itr(&m_tree, true);
            ++itr;
            return itr;
        }
        Reverse_Iterator rbegin()
        {
            Iterator itr(&m_tree, false);
            ++itr;
            return itr;
        }
        Const_Reverse_Iterator crbegin() const
        {
            Iterator itr(&m_tree, false);
            ++itr;
            return itr;
        }
        Iterator end()
        {
            return Iterator(&m_tree);
        }
        Const_Iterator cend() const
        {
            return Iterator(&m_tree);
        }
        Reverse_Iterator rend()
        {
            return Iterator(&m_tree, false);
        }
        Const_Reverse_Iterator crend() const
        {
            return Iterator(&m_tree, false);
        }
    //-------------------------------------------------------

        void print()
        {
            m_tree.print();
        }

    private:
        IMemoryContext*                         m_context = nullptr;      
        QRBTree<QPair<Key, Data>>               m_tree;
    };
}

#endif //_H_Q_MAP