#ifndef _H_MEMORY_HELPERS
#define _H_MEMORY_HELPERS

#include <MemoryConstants.h>
#include <Types/QAllocator.h>

namespace Quaint
{
    class IMemoryContext;
    extern Quaint::IMemoryContext* G_DEFAULT_MEMORY;

    namespace internal
    {
        template<typename Iterator>
        struct DefaultIteratorTraits
        {
            typedef typename Iterator::iterator_category iterator_category;
			typedef typename Iterator::value_type        value_type;
			typedef typename Iterator::difference_type   difference_type;
			typedef typename Iterator::pointer           pointer;
			typedef typename Iterator::reference         reference;
        };
    }

    template<typename Iterator>
    struct IteratorTraits : internal::DefaultIteratorTraits<Iterator> {};

    //Override for pointer iterator types
    template<typename T>
    struct IteratorTraits<T*> 
    {
        typedef T               value_type;
        typedef ptrdiff_t       difference_type;
        typedef T*              pointer;
        typedef T&              reference;
    };


    inline void* AllocateMemory(QAllocatorBase& allocator, size_t size)
    {
        return allocator.allocate(size);
    }
    inline void* AllocateMemory(QAllocatorBase& allocator, size_t size, uint16_t alignment)
    {
        return allocator.allocate(size, alignment);
    }
    inline void DeAllocateMemory(QAllocatorBase& allocator, void* memory)
    {
        allocator.deallocate(memory);
    }

    template<typename T>
    inline T* getAddress(const T& val)
    {
        const char& cc = reinterpret_cast<const char&>(val);
        char& c = const_cast<char&>(c);
        return reinterpret_cast<T*>(&c);
    }

    template<typename Iterator>
    inline Iterator Construct_In_Uninitialized_Storage(Iterator first, size_t count)
    {
        //We first need the value type of the iterator
        typedef typename IteratorTraits<Iterator>::value_type value_type;

        Iterator memLoc = first;
        //Default construct count elements in the uninitialized storage
        for(; count > 0; --count, ++memLoc)
        {
            ::new(addressof(*memLoc)) value_type();
        }
        return memLoc;
    }
    
}

#endif //_H_MEMORY_HELPERS