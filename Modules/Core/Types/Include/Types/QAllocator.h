#ifndef _Q_ARRAY
#define _Q_ARRAY

#include <limits.h>
#include <assert.h>
#include <MemoryConstants.h>
#include <Types/QStaticString.h>
#include <Interface/IMemoryContext.h>

namespace Quaint
{
    template<typename T>
    class QAllocator
    {
    public:
        typedef T*                  pointer;
        typedef const T*            const_pointer;
        typedef (void*)(T*)         void_pointer;
        typedef (const void*)(T*)   const_void_pointer;
        typedef T                   value_type;

        QAllocator(IMemoryContext* context, const Quaint::QName& name);

        pointer allocate(size_t size);
        pointer allocate(size_t size, uint16_t alignment);
        void deallocate(pointer p);
        void deallocate(pointer p, size_t n);

        void setName(const QName& name) { m_name = name; }
        const QName& getName() { return m_name; }

    protected:
        QName m_name;
        IMemoryContext m_context;
    };

    template<typename T>
    QAllocator<T>::QAllocator(IMemoryContext* context, const Quaint::QName& name)
    : m_context(context)
    , m_name(name)
    {
        assert(context != nullptr && "Invalid memory context passed");
    }

    template<typename T>
    QAllocator<T>::pointer QAllocator<T>::allocate(size_t size)
    {
        return allocate(size, DEFAULT_ALIGNMENT);
    }
    template<typename T>
    QAllocator<T>::pointer QAllocator<T>::allocate(size_t size, uint16_t alignment)
    {
        return m_context.AllocAligned(size, alignment);
    }

    template<typename T>
    void QAllocator<T>::deallocate(pointer p)
    {
        deallocate(p, 0);
    }
    template<typename T>
    void QAllocator<T>::deallocate(pointer p, size_t n)
    {
        m_context.Free(p);
    }

    //Helper Functions
    
}

#endif //_Q_ARRAY