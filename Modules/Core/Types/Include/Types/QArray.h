#ifndef _Q_ARRAY
#define _Q_ARRAY

#include <cstring>
#include <limits.h>
#include <assert.h>
#include <Interface/IMemoryContext.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Quaint
{
    //TODO: 
    // 1. Need clear command
    // 2. Cleanup code..... It's pretty messy

    template<typename T>
    class QArray
    {
        #define TYPE_SIZE sizeof(T)
        #define TARGET_RESERVE(X) (size_t)((float)X * 1.5f)
        #define INC_RESERVE_FROM(X) (TARGET_RESERVE(X) > UINT_MAX) ? (UINT_MAX - 1) : (TARGET_RESERVE(X))

        friend class QArray<T>;
    public:
        using value_type      = T;
        using reference       = const T&;
        using const_reference = const T&;
        using size_type       = size_t;

        using Iterator = T*;
        using Const_Iterator = const T*;

public:
        QArray() 
        {
            m_context = nullptr;
            m_size = 0;
        };
        static QArray<T> GetInvalidPlaceholder() { return QArray<T>(); }

        QArray(IMemoryContext* context)
        {
            m_context = context;
            m_size = 0;
            reserve(((m_size + 8) / 4) * 4);
        }

        template<typename ...ARGS>
        QArray(IMemoryContext* context, size_t size = 0, ARGS... args)
        {
            m_context = context;
            m_size = size;
            reserve(((size + 8) / 4) * 4);
            construct(args...);
        }
        //template<typename ...ARGS>
        //QArray(IMemoryContext* context, ARGS... args)
        //{
        //    m_context = context;
        //    m_size = 0;
        //    reserve(4);
        //    construct(args...);
        //}
        
        QArray(IMemoryContext* context, size_t size, const T& defVal)
        {
            m_context = context;
            m_size = size;
            reserve(((size + 8) / 4) * 4);
            for(size_t i = 0; i < m_size; i++)
            {
                *(m_rawData + i) = defVal;
            }
        }
        template<size_t SZ>
        QArray(IMemoryContext* context, const T(&list)[SZ])
        {
            m_context = context;
            m_size = SZ;
            reserve(((m_size + 8) / 4) * 4);
            for(size_t i = 0; i < m_size; i++)
            {
                *(m_rawData + i) = list[i];
            }
        }
        /*Copy constructing. Therefore uses other's memory context and deep copies data from other*/
        QArray(const QArray<T>& other)
        {
            m_context = other.getMemoryContext();

            reserve(other.getReservedSize());
            //memcpy(m_rawData, other.getBuffer(), other.getSize() * TYPE_SIZE);
            m_size = other.getSize();
            for(size_t i = 0; i < m_size; ++i)
            {
                new(m_rawData + i)T(other[i]);
            }
        }
        /*Uses memory context of object being copied from and simply points to the internal data of other*/
        QArray(QArray<T>&& other)
        {
            m_context = other.getMemoryContext();
            m_reservedSize = other.getReservedSize();
            m_rawData = const_cast<T*>(other.getBuffer());
            m_size = other.getSize();

            other.invalidate();
        }

        /*Uses allocator defined in this object and deep copies data from other object*/
        QArray& operator=(const QArray<T>& other)
        {
            //deep copy from other
            m_rawData = nullptr;
            m_context = other.getMemoryContext();
            reserve(other.getReservedSize());
            //memcpy(m_rawData, other.getBuffer(), other.getSize() * TYPE_SIZE);
            m_size = other.getSize();
            for(size_t i = 0; i < m_size; ++i)
            {
                new(m_rawData + i)T(other[i]);
            }
            return *this;
        }
        template<int N>
        QArray& operator=(const T(&other)[N])
        {
            assert(m_context && "No context set");
            clear();
            //deep copy from other
            m_rawData = nullptr;
            reserve(N);
            //memcpy(m_rawData, other.getBuffer(), other.getSize() * TYPE_SIZE);
            m_size = N;
            for(size_t i = 0; i < m_size; ++i)
            {
                new(m_rawData + i)T(other[i]);
            }
            return *this;
        }
        /*Reclaims storage in current object and simply points to data in the other structure*/
        QArray& operator=(QArray<T>&& other)
        {
            //Our object is already constructed. Reclaim the allocated storage and then just reassign from other
            if(m_rawData != nullptr)
            {
                if(m_context != nullptr)
                {
                    destroy();
                    //m_context->Free(m_rawData);
                }
                else
                {
                    delete[] m_rawData;
                }
            }

            m_context = other.getMemoryContext();
            m_reservedSize = other.getReservedSize();
            m_rawData = const_cast<T*>(other.getBuffer());
            m_size = other.getSize();

            other.invalidate();
            return *this;
        }
        ~QArray()
        {
            if(m_rawData == nullptr) return;
            
            if(m_context != nullptr)
            {
                destroy();
                //m_context->Free(m_rawData);
            }
            else
            {
                delete[] m_rawData;
            }
        }

        Iterator begin(){ return m_rawData; }
        Const_Iterator begin() const { return m_rawData; }
        Iterator end() { return m_rawData + m_size; }
        Const_Iterator end() const { return m_rawData + m_size; }

        void resize(size_t size)
        {
            m_size = size;
            reserve(((size + 8) / 4) * 4);
        }

        void clear()
        {
            m_size = 0;
            reserve(4);
        }
        
        template<typename ...ARGS>
        void resizeWithArgs(size_t size, ARGS... args)
        {
            resize(size);
            construct(args...);
        }

        void reserve(size_t size)
        {
            m_reservedSize = size;
            resize();
        }
        void pushBack(const T& t)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            if(m_size >= m_reservedSize)
            {
                reserve(INC_RESERVE_FROM(m_size));
            }

            //memcpy((m_rawData + m_size), &t, TYPE_SIZE);
            new(m_rawData + m_size)T(t);
            ++m_size;
        }
        void pushBack(T&& t)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            if(m_size >= m_reservedSize)
            {
                reserve(INC_RESERVE_FROM(m_size));
            }

            //memcpy((m_rawData + m_size), &t, TYPE_SIZE);
            new(m_rawData + m_size)T(std::move(t));
            ++m_size;
        }

        /* Currently only emplaces at the end */
        template<typename ...ARGS>
        void emplace(ARGS... args)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            if(m_size >= m_reservedSize)
            {
                reserve(INC_RESERVE_FROM(m_size));
            }
            new(m_rawData + m_size)T(args...);

            ++m_size;
        }
        //TODO: insert range; insert other array
        void insertAt(size_t index, const T& t)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            insertRangeAt(index, {t});
        }
        void insertRangeAt(size_t index, const QArray& other, Const_Iterator start, Const_Iterator last)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            assert((index >= 0 && index <= m_size) && "Trying to insert at invalid index");
            assert((start >= other.begin() && last < other.end() && last >= start) && "Trying to insert invalid range");
            
            //TODO
            size_t requiredSize = m_size + last - start + 1;
            if(requiredSize > m_reservedSize)
            {
                reserve(((requiredSize + 8) / 4) * 4);
            }

            size_t numElemsToInsert = (last - start) + 1;

            // TODO: This might not work. Elements must be properly constructed to perform right type of copy.
            // If data is copied this way, and object has any arrays within it, they refer to the same raw memory location.
            // If one array in object is destroyed, other would dangle
            memmove(m_rawData + index + numElemsToInsert, m_rawData + index, (m_size - index) * TYPE_SIZE);
            memcpy(m_rawData + index, start, numElemsToInsert * TYPE_SIZE);

            m_size += numElemsToInsert;
        }

        /* Would not work is list contains arrays. Doesn't invoke a copy constructor, but rather copies contents as it is
        TODO: Create a new variant that does copy contruction
        */
        template<size_t SZ>
        void insertRangeAt(size_t index, const T(&list)[SZ])
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            static_assert(SZ > 0, "Passed Invalid array");
            assert((index >= 0 && index <= m_size) && "Trying to insert at invalid index");
            if(m_size + SZ > m_reservedSize)
            {
                reserve(((m_size + SZ + 8) / 4) * 4);
            }
            
            // TODO: This might not work. Elements must be properly constructed to perform right type of copy.
            // If data is copied this way, and object has any arrays within it, they refer to the same raw memory location.
            // If one array in object is destroyed, other would dangle
            if(m_size - index > 0)
            {
                memmove(m_rawData + index + SZ, m_rawData + index, (m_size - index) * TYPE_SIZE);
            }
            memcpy(m_rawData + index, list, SZ * TYPE_SIZE);

            m_size += SZ;
        }

        /*Remove an element from back*/
        void popBack()
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            assert(m_size > 0 && "Trying to remove element from an empty array");
            --m_size;
        }
        /*Remove an element at index*/
        void removeAt(size_t index)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            assert((index >= 0 && index < m_size) && "Trying to remove element with invalid index");
            
            removeRange(begin() + index, begin() + index);
        }
        void removeAt(Const_Iterator itr)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            removeRange(itr, itr);
        }
        /*Removes [start, last] from array*/
        void removeRange(Iterator start, Iterator last)
        {
            assert(m_context != nullptr && "Needs a valid memory context to work with");
            assert(start >= begin() && last < end() && "Trying to remove invalid element range");
            
            size_t n = (last - start) + 1;
            void* movDst = (void*)(start);
            void* movSrc = (void*)(start + n);
            size_t numToMove = (m_rawData + m_size) - (last) - 1;
            if(numToMove > 0)
            {
                memmove(movDst, movSrc, numToMove * TYPE_SIZE);
            }
            m_size -= n;
        }

        const T& get(size_t index) const
        {
            return operator[](index);
        }
        T& at(size_t index)
        {
            return operator[](index);
        }
        T& operator[](size_t index)
        {
            assert((index >= 0 && index < m_size) && "Trying to access invalid element");
            return *(m_rawData + index);
        }
        const T& operator[](size_t index) const
        {
            assert((index >= 0 && index < m_size) && "Trying to access invalid element");
            return *(m_rawData + index);
        }

        size_t getReservedSize() const { return m_reservedSize; }
        size_t getSize() const { return m_size; }
        const T* getBuffer() const { return m_rawData; }
        T* getBuffer_NonConst() { return m_rawData; }
        bool isEmpty() { return m_size == 0; }

        IMemoryContext* getMemoryContext() const { return m_context; }

    private:
    
        /* should be used with utmost caution. This makes destructor not release memory. 
         * Should ideally be used when moving to a different location
         */
        void invalidate()
        {
            m_rawData = nullptr;
            m_size = 0;
            m_reservedSize = 0;
        }
        void resize()
        {
            assert(m_context != nullptr && "Null Context is not currently supported");
            //TODO: Add support for null context
            T* oldData = m_rawData;
            
            m_rawData = (T*)QUAINT_ALLOC_MEMORY_ALIGNED(m_context, m_reservedSize * TYPE_SIZE, alignof(T));
            
            if(oldData != nullptr)
            {
                if(m_size > 0)
                {
                    memcpy(m_rawData, oldData, m_size * TYPE_SIZE);
                }
                QUAINT_DEALLOC_MEMORY(m_context, oldData); //No need to call destructor here
            }
            
            //else
            //{
            //    m_rawData = new T[m_reservedSize];
            //    if(oldData != nullptr)
            //    {
            //        memcpy(m_rawData, oldData, m_size * TYPE_SIZE);
            //        delete[] oldData;
            //    }
            //}
        }
        template<typename ...ARGS>
        void construct(ARGS... args)
        {
            for(size_t i = 0; i < m_size; i++)
            {
                new(m_rawData + i)T(args...);
            }
        }
        void defConstruct()
        {
            for(size_t i = 0; i < m_size; i++)
            {
                new(m_rawData + i)T();
            }
        }
        void destroy()
        {
            assert(m_context != nullptr && "Null Context is not currently supported");
            
            for(size_t i = 0; i < m_size; i++)
            {
                (m_rawData + i)->~T();
            }
            QUAINT_DEALLOC_MEMORY(m_context, m_rawData);
        }

        IMemoryContext*     m_context       = nullptr;
        size_t              m_reservedSize  = 4;
        size_t              m_size          = 0;
        T*                  m_rawData       = nullptr;
    };
}

#endif //_Q_ARRAY