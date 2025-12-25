#ifndef _Q_ALLOCATOR
#define _Q_ALLOCATOR

#include <limits.h>
#include <assert.h>
#include <MemoryConstants.h>
#include <Types/QStaticString.h>
#include <Interface/IMemoryContext.h>

namespace Quaint
{
    class QAllocatorBase
    {
    public:
        QAllocatorBase(IMemoryContext* context, const Quaint::QName& name);

        void* allocate(size_t size);
        void* allocate(size_t size, uint16_t alignment);
        void deallocate(void* p);
        void deallocate(void* p, size_t n);
    
    private:
        QName m_name;
        IMemoryContext* m_context;
    };

    template<typename T>
    class QAllocator : public QAllocatorBase
    {
    public:
        typedef T*                  pointer;
        typedef const T*            const_pointer;
        typedef void*               void_pointer;
        typedef const void*         const_void_pointer;
        typedef T                   value_type;

        QAllocator(IMemoryContext* context, const Quaint::QName& name);

        pointer allocate(size_t size);
        pointer allocate(size_t size, uint16_t alignment);
        void deallocate(pointer p);
        void deallocate(pointer p, size_t n);

        void setName(const QName& name) { m_name = name; }
        const QName& getName() { return m_name; }

    protected:
    };

    inline QAllocatorBase::QAllocatorBase(IMemoryContext* context, const Quaint::QName& name)
    : m_context(context)
    , m_name(name)
    {
        assert(context != nullptr && "Invalid memory context passed");
    }

    inline void* QAllocatorBase::allocate(size_t size)
    {
        return allocate(size, DEFAULT_ALIGNMENT);
    }
    inline void* QAllocatorBase::allocate(size_t size, uint16_t alignment)
    {
        return m_context->AllocAligned(size, alignment);
    }
    inline void QAllocatorBase::deallocate(void* p)
    {
        deallocate(p, 0);
    }
    inline void QAllocatorBase::deallocate(void* p, size_t n)
    {
        m_context->Free(p);
    }

    template<typename T>
    inline QAllocator<T>::QAllocator(IMemoryContext* context, const Quaint::QName& name)
    : QAllocatorBase(context, name)
    {
    }

    template<typename T>
    inline typename QAllocator<T>::pointer QAllocator<T>::allocate(size_t size)
    {
        return static_cast<pointer>(QAllocatorBase::allocate(size, DEFAULT_ALIGNMENT));
    }
    template<typename T>
    inline typename QAllocator<T>::pointer QAllocator<T>::allocate(size_t size, uint16_t alignment)
    {
        return static_cast<pointer>(QAllocatorBase::allocate(size, alignment));
    }

    template<typename T>
    inline void QAllocator<T>::deallocate(pointer p)
    {
        QAllocatorBase::deallocate(static_cast<void*>(p), 0);
    }
    template<typename T>
    inline void QAllocator<T>::deallocate(pointer p, size_t n)
    {
        QAllocatorBase::deallocate(static_cast<void*>(p));
    }

    //Helper Functions
    
}

#endif //_Q_ALLOCATOR