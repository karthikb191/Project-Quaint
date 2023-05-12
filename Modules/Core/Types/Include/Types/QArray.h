#ifndef _Q_ARRAY
#define _Q_ARRAY

#include <cstring>
#include <limits.h>
#include <assert.h>
#include <Interface/IMemoryContext.h>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Quaint
{
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

        QArray(IMemoryContext* context, size_t rs = 4)
        {
            m_context = context;
            reserve(rs);
        }
        /*Copy constructing. Therefore uses other's memory context and deep copies data from other*/
        QArray(const QArray<T>& other)
        {
            m_context = other.getMemoryContext();

            reserve(other.getReservedSize());
            memcpy(m_rawData, other.getBuffer(), other.getSize() * TYPE_SIZE);
            m_size = other.getSize();
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
            reserve(other.getReservedSize());
            memcpy(m_rawData, other.getBuffer(), other.getSize() * TYPE_SIZE);
            m_size = other.getSize();
        }
        /*Reclaims storage in current object and simply points to data in the other structure*/
        QArray& operator=(QArray<T>&& other)
        {
            //Our object is already constructed. Reclaim the allocated storage and then just reassign from other
            if(m_rawData != nullptr)
            {
                if(m_context != nullptr)
                {
                    QUAINT_DELETE_ARRAY(m_context, m_rawData);
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
        }
        ~QArray()
        {
            if(m_rawData == nullptr) return;
            
            if(m_context != nullptr)
            {
                QUAINT_DELETE_ARRAY(m_context, m_rawData);
                //m_context->Free(m_rawData);
            }
            else
            {
                delete[] m_rawData;
            }
        }

        Iterator begin(){ return m_rawData; }
        Const_Iterator begin_c() const { return m_rawData; }
        Iterator end() { return m_rawData + m_size; }
        Const_Iterator end_c() const { return m_rawData + m_size; }

        void reserve(size_t size)
        {
            m_reservedSize = size;
            resize();
        }
        void pushBack(const T& t)
        {
            if(m_size >= m_reservedSize)
            {
                reserve(INC_RESERVE_FROM(m_size));
            }

            *(m_rawData + m_size) = t;
            ++m_size;
        }
        //TODO: insert range; insert other array
        void insertAt(size_t index, const T& t)
        {
            insertRangeAt(index, {t});
        }
        void insertRangeAt(size_t index, const QArray& other, Const_Iterator start, Const_Iterator last)
        {
            assert((index >= 0 && index < m_size) && "Trying to insert at invalid index");
            assert((start >= other.begin_c() && last < other.end_c() && last >= start) && "Trying to insert invalid range");
            
            //TODO
            size_t requiredSize = m_size + last - start + 1;
            if(requiredSize > m_reservedSize)
            {
                reserve((requiredSize + 8 / 4) * 4);
            }

            size_t numElemsToInsert = (last - start) + 1;

            memmove(m_rawData + index + numElemsToInsert, m_rawData + index, (m_size - index) * TYPE_SIZE);
            memcpy(m_rawData + index, start, numElemsToInsert * TYPE_SIZE);

            m_size += numElemsToInsert;
        }
        template<size_t SZ>
        void insertRangeAt(size_t index, const T(&list)[SZ])
        {
            static_assert(SZ > 0, "Passed Invalid array");
            assert((index >= 0 && index < m_size) && "Trying to insert at invalid index");
            if(m_size + SZ > m_reservedSize)
            {
                reserve(((m_size + SZ + 8) / 4) * 4);
            }

            memmove(m_rawData + index + SZ, m_rawData + index, (m_size - index) * TYPE_SIZE);
            memcpy(m_rawData + index, list, SZ * TYPE_SIZE);

            m_size += SZ;
        }

        /*Remove an element from back*/
        void popBack()
        {
            assert(m_size > 0 && "Trying to remove element from an empty array");
            --m_size;
        }
        /*Remove an element at index*/
        void removeAt(size_t index)
        {
            assert((index >= 0 && index < m_size) && "Trying to remove element with invalid index");
            
            removeRange(begin() + index, begin() + index);
        }
        void removeAt(Const_Iterator itr)
        {
            removeRange(itr, itr);
        }
        /*Removes [start, last] from array*/
        void removeRange(Iterator start, Iterator last)
        {
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

        T& at(size_t index)
        {
            return operator[](index);
        }
        T& operator[](size_t index)
        {
            assert((index >= 0 && index < m_size) && "Trying to access invalid element");
            return *(m_rawData + index);
        }

        size_t getReservedSize() const { return m_reservedSize; }
        size_t getSize() const { return m_size; }
        const T* getBuffer() const { return m_rawData; }
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
            T* oldData = m_rawData;
            
            if(m_context != nullptr)
            {
                m_rawData = QUAINT_NEW_ARRAY(m_context, T, m_reservedSize); 
                //(T*)m_context->Alloc(m_reservedSize * TYPE_SIZE);
                if(oldData != nullptr)
                {
                    memcpy(m_rawData, oldData, m_size * TYPE_SIZE);
                    QUAINT_DELETE_ARRAY(m_context, oldData);
                    //m_context->Free(oldData);
                }
            }
            else
            {
                m_rawData = new T[m_reservedSize];
                if(oldData != nullptr)
                {
                    memcpy(m_rawData, oldData, m_size * TYPE_SIZE);
                    delete[] oldData;
                }
            }
        }

        IMemoryContext*     m_context       = nullptr;
        size_t              m_reservedSize  = 4;
        size_t              m_size          = 0;
        T*                  m_rawData       = nullptr;
    };
}

#endif //_Q_ARRAY