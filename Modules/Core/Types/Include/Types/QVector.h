#ifndef _Q_VECTOR
#define _Q_VECTOR

#include "QAllocator.h"
#include "MemoryHelpers.h"

namespace Quaint
{
    #define DEFAULT_ALLOCATOR allocator_type(G_DEFAULT_MEMORY, "DefaultQuaintAllocator")

    template<typename T, typename TAllocator = Quaint::QAllocatorBase>
    class QVector
    {
    public:
        typedef T                           value_type;
        typedef T*                          pointer;
        typedef const T*                    const_pointer;
        typedef T&                          reference;
        typedef const T&                    const_reference;
        typedef T*                          iterator;
        typedef const T*                    const_iterator;
        //TODO: Reverse iterators
        typedef size_t                      size_type;
        typedef ptrdiff_t                   difference_type;
        typedef TAllocator                  allocator_type;
        typedef QVector<T, TAllocator>      this_type;

        //Constructors
        QVector();
        QVector(size_type size);
        explicit QVector(const allocator_type& allocator);
        explicit QVector(const allocator_type& allocator, size_type size);
        explicit QVector(const allocator_type& allocator, size_type size, T& value);
        QVector(std::initializer_list<value_type> initializerList, const allocator_type& allocator = DEFAULT_ALLOCATOR);
        
        //Copy, Move and assignment
        QVector(QVector&);
        QVector(const QVector&);

        //Destructors
        ~QVector();

        void push_back(const_reference t);
        void push_back(value_type&& t);
        reference push_back();
        void pop_back();
        
        template<typename ...Args>
        void emplace(const_iterator position, Args&&... args);
        template<typename ...Args>
        void emplace_back(Args&&... args);

        //TODO: Insert

        void erase(const_iterator position);
        void erase(const_iterator first, const_iterator last);
        void clear();

        pointer       data() noexcept;
		const_pointer data() const noexcept;

		reference       operator[](size_type n);
		const_reference operator[](size_type n) const;

		reference       at(size_type n);
		const_reference at(size_type n) const;

        void swap(const_reference other);

        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;

        //TODO: Reverse iteration

        bool empty();
        size_type size() const noexcept;
        size_type capacity() const noexcept;

        void resize(size_type size);
        void resize(size_type size, const_reference t);
        void reserve(size_type size);

    protected:
        
        size_type increaseCapacity(size_type size);
        pointer doAllocate(size_type size);
        void doGrow(size_type size);

        /*Retrieves more capacity than the provided size*/
        size_type getMoreCapacity(size_type size);
        const_pointer getCapacityEnd() const { return m_capacityEnd; }

        pointer m_begin;
        pointer m_end;
        pointer m_capacityEnd;
        TAllocator m_allocator;
        size_type m_capacity;
    };

    template<typename T, typename TAllocator>
    inline QVector<T, TAllocator>::~QVector()
    {
        if(m_begin != nullptr)
        {
            DeAllocateMemory(m_allocator, m_begin);
        }
    }

    template<typename T, typename TAllocator>
    inline QVector<T, TAllocator>::QVector()
    : m_begin(NULL)
    , m_end(NULL)
    , m_capacityEnd(NULL)
    , m_allocator(DEFAULT_ALLOCATOR)
    {
    }

    template<typename T, typename TAllocator>
    inline QVector<T, TAllocator>::QVector(const allocator_type& allocator)
    : m_begin(NULL)
    , m_end(NULL)
    , m_capacityEnd(NULL)
    , m_allocator(allocator)
    {
    }

    template<typename T, typename TAllocator>
    inline QVector<T, TAllocator>::QVector(size_type size)
    : m_begin(NULL)
    , m_end(NULL)
    , m_capacityEnd(NULL)
    , m_allocator(DEFAULT_ALLOCATOR)
    {
        m_begin = doAllocate(size);
        Construct_In_Uninitialized_Storage(m_begin, size);
        m_end = m_begin + size;
        m_capacityEnd = m_begin + size;
    }

    template<typename T, typename TAllocator>
    inline QVector<T, TAllocator>::QVector(const allocator_type& allocator, size_type size)
    : m_begin(NULL)
    , m_end(NULL)
    , m_capacityEnd(NULL)
    , m_allocator(allocator)
    {
        //We allocate data, but no elements are pushed
        m_begin = doAllocate(size);
        Construct_In_Uninitialized_Storage(m_begin, size);
        m_end = m_begin + size;
        m_capacityEnd = m_begin + size;
    }

    template<typename T, typename TAllocator>
    inline QVector<T, TAllocator>::QVector(const allocator_type& allocator, size_type size, T& value)
    : m_begin(NULL)
    , m_end(NULL)
    , m_capacityEnd(NULL)
    , m_allocator(allocator)
    {
        //TODO: allocate and assign the value
        m_begin = doAllocate(size);
        m_end = m_begin + size;
        m_capacityEnd = m_end;
    }

    template<typename T, typename TAllocator>
    inline typename Quaint::QVector<T, TAllocator>::pointer Quaint::QVector<T, TAllocator>::doAllocate(size_type size)
    {
        //just allocates the memory and returns the pointer
        const size_t largeValue = ~0 >> 1;
        if(size >= largeValue)
        {
            assert(false && "Trying to allocated abnormally large value");
            return nullptr;
        }

        pointer p = (pointer)m_allocator.allocate(size * sizeof(value_type), alignof(T));
        assert(p != nullptr && "Allocation failed!!");
        return p;
    }

    template<typename T, typename TAllocator>
    void Quaint::QVector<T, TAllocator>::doGrow(size_type size)
    {
        // Allocate
        // Move data
        // Destruct old
        // remap pointers
    }

    template<typename T, typename TAllocator>
    typename Quaint::QVector<T, TAllocator>::size_type Quaint::QVector<T, TAllocator>::getMoreCapacity(size_type size)
    {
        //Gets 50% more size than provided input
        return size * 1.5f;
    }

    template<typename T, typename TAllocator>
    typename Quaint::QVector<T, TAllocator>::size_type Quaint::QVector<T, TAllocator>::size() const noexcept
    {
        return (m_end - m_begin);
    }

    template<typename T, typename TAllocator>
    typename Quaint::QVector<T, TAllocator>::reference Quaint::QVector<T, TAllocator>::operator[](size_type n)
    {
        assert(n < size() && "Accessing out of bounds element");
        return *(m_begin + n);
    }

    template<typename T, typename TAllocator>
	typename Quaint::QVector<T, TAllocator>::const_reference Quaint::QVector<T, TAllocator>::operator[](size_type n) const
    {
        assert(n < size() && "Accessing out of bounds element");
        return *(m_begin + n);
    }


}

#endif