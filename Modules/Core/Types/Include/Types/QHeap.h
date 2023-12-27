#ifndef _H_Q_HEAP
#define _H_Q_HEAP

#include <Interface/IMemoryContext.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <math.h>

namespace Quaint
{
    enum _HeapType : int
    {
        EMIN = 1,
        EMAX = 2
    };
    
    /* Properties:
    *   1. Fast Access   2. Heap Allocated   3. Random accessible
    *   4. Copies data when swap occurs. This can be improved with an aux array
    */
    template<typename T, int TYPE = _HeapType::EMIN>
    class QHeap
    {
        #define TYPE_SIZE       sizeof(T)
        #define DATA            ((T*)m_rawData)
        #define CONST_DATA      ((const T*)m_rawData)

    public:
        typedef T                       value_type;
        typedef const T&                reference_type;
        typedef const T&                const_reference;
        typedef size_t                  size_type;
        
        //TODO: Cosmetic. Write an new iterator later
        //typedef typename QPriorityQueue<T>::Iterator                = Iterator;
        //typedef typename const QPriorityQueue<T>::Iterator          = Const_Iterator;

        /*Size in Priority Queue is reserved to the powers of 2*/
        QHeap(IMemoryContext* context)
        {
            m_context = context;
            // Starting with 16 elements
            reserve(1 << 4);
            m_size = 0;
        }
        QHeap(const QHeap& other)
        {

        }
        QHeap(QHeap&& other);
        QHeap& operator=(const QHeap& other);
        QHeap& operator=(QHeap&& other);

        ~QHeap()
        {
            if(m_rawData)
            {
                QUAINT_DEALLOC_MEMORY(m_context, m_rawData);
                m_rawData = nullptr;
            }
            m_context = nullptr;
        }

        size_t getSize() { return m_size; }

        //Iterator& begin();
        //Iterator& end();
        //Const_Iterator& begin();
        //Const_Iterator& end();

        void insert(const T& data)
        {
            if(m_size == 0)
            {
                m_rawData[0] = data;
                ++m_size;
                return;   
            }

            size_t current = m_size;
            ++m_size;
            DATA[current] = data;
            while(current != 0)
            {
                size_t p = parent(current);

                if(compare(DATA[current], DATA[p]))
                {
                    T temp = DATA[current];
                    DATA[current] = DATA[p];
                    DATA[p] = temp;
                    current = p;
                    continue;
                }
                break;
            }
            
            if(m_size == m_reservedSize)
            {
                reserve(m_reservedSize >> 1);
            }
        }

        /*Removes and returns the top of heap*/
        void top(T* top = nullptr)
        {
            if(top != nullptr) *top = *(peek());
            remove(0);
        }

        T* peek()
        {
            if(m_size == 0) return nullptr;
            
            return &(m_rawData[0]);
        }

        /*Removes Key at index*/
        void remove(size_t index)
        {
            assert(index < m_size && "Trying to remove element with invalid index");
            size_t lastIdx = m_size - 1;
            
            if(lastIdx == index)
            {
                --m_size;
                return;
            }
            
            //Replace with the node being removed
            DATA[index] = DATA[lastIdx];
            --m_size;

            // Bubble down the newly moved element
            // Loop so long as node has a child
            size_t current = index;
            size_t leftIdx = left(current);
            size_t rightIdx = right(current);
            while(leftIdx < m_size)
            {
                size_t idxToReplace = leftIdx;
            
                if(compare(DATA[rightIdx], DATA[leftIdx]))
                {
                    idxToReplace = rightIdx;
                }
                
                if(compare(DATA[current], DATA[idxToReplace]))
                {
                    break;
                }
                //Swap
                T temp = DATA[current];
                DATA[current] = DATA[idxToReplace];
                DATA[idxToReplace] = temp;
                current = idxToReplace;
                leftIdx = left(current);
                rightIdx = right(current);
            }
        }

        void print()
        {
            if(m_size == 0) { std:: cout << "Empty!\n"; return; }
            printSubTree(0);
        }

    private:
        size_t left(size_t nodeIdx)
        {
            return (nodeIdx << 1) + 1;
        }
        size_t right(size_t nodeIdx)
        {
            return (nodeIdx << 1) + 2;
        }
        size_t parent(size_t nodeIdx)
        {
            assert(nodeIdx != 0 && "Root cannot have a parent.");
            return (nodeIdx - 1) >> 1;
        }
        bool isRoot(size_t nodeIdx)
        {
            return (nodeIdx == 0);
        }

        inline bool compare(T& a, T& b)
        {
            if(TYPE == EMIN) return a < b;
            return a > b;
        }

        void reserve(size_t size)
        {
            assert((size < (SIZE_MAX / 2 - 1)) && "Over subscription. Could cause overflow");
            T* oldData = m_rawData;
            m_reservedSize = size;
            m_rawData = (T*)QUAINT_ALLOC_MEMORY_ALIGNED(m_context, m_reservedSize * TYPE_SIZE, alignof(T));
            if(oldData)
            {
                memcpy(m_rawData, oldData, m_size);
                QUAINT_DEALLOC_MEMORY(m_context, oldData);
                oldData = nullptr;
            }
        }

        void printSubTree(size_t idx, size_t tabs = 0)
        {
            if(right(idx) < m_size)
            {
                printSubTree(right(idx), tabs + 1);
            }

            for(size_t i = 0; i < tabs; i++)
            {
                std::cout << "\t";
            }
            std::cout << DATA[idx] << "\n";

            if(left(idx) < m_size)
            {
                printSubTree(left(idx), tabs + 1);
            }
        }

        IMemoryContext*     m_context = nullptr;
        T*                  m_rawData = nullptr;
        size_t              m_size = 0;
        size_t              m_reservedSize = 0;
    };
}

#endif //_H_Q_HEAP