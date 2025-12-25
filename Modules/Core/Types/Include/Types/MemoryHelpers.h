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

    //Copies given value to the storage occupied by the iterator range
    template<typename Iterator, typename T>
    inline Iterator Copy_To_Uninitialized_Storage(Iterator first, size_t count, const T& value)
    {
        typedef typename IteratorTraits<Iterator>::value_type value_type;

        Iterator current = first;
        for(; count > 0; --count, ++current)
        {
            ::new(addressof(*current)) value_type(value);
        }
        return current;
    }

    //Moves values to the destination iterator
    template<typename ForwardIterator, typename DestinationIterator>
    inline DestinationIterator Copy_To(ForwardIterator first, ForwardIterator last, DestinationIterator target)
    {
        typedef typename IteratorTraits<ForwardIterator>::value_type ITYP;
        typedef typename IteratorTraits<DestinationIterator>::value_type OTYP;
        
        //If input and output types are same, we can simply use memmove instead of invoking constructors
        bool isTriviallyCopyable = std::is_same(ITYP) && std::is_same(OTYP);

        //Can move the whole block at once if iterators are contiguous 
        //For now, only pointer iterators are marked contiguous
        bool areIteratorsContiguous = std::is_pointer<ForwardIterator>::value && std::is_pointer<DestinationIterator>::value;

        if(isTriviallyCopyable && areIteratorsContiguous)
        {
            //Block mem copy
            auto count = (last - first);
            std::memmove(eastl::addressof(*target), eastl::addressof(*first), sizeof(OTYP) * count);
			return target + count;
        }
        else if(isTriviallyCopyable)
        {
            //Mem copy individual elements
            DestinationIterator destination = target;
            ForwardIterator current = first;
            while(current != last)
            {
                std::memmove(addressof(destination), addressof(current), sizeof(OTYP));
            }
            return destination;
        }
        else
        {
            //Construct elements by invoking copy constructor
            DestinationIterator destination = target;
            ForwardIterator current = first;
            while(current != last)
            {
                ::new(static_cast<void*>(Addressof(*destination))) value_type(*current);
                ++destination;
                ++current;
            }
            return destination;
        }
    }

    template<typename BiDirectionalIterator>
    inline BiDirectionalIterator Move_Backward(BiDirectionalIterator first, BiDirectionalIterator last, BiDirectionalIterator result)
    {
        BiDirectionalIterator current = last;
        while(current != first)
        {
            *--result = *--current;
        }
        return result;

        //TODO: Expand this!!!

    }

    // Quickly moves the storage in the input range to result.
    // Undefined if the ranges overlap.
    // Only works on pointer iterators for now
    template<typename ForwardIterator>
    inline ForwardIterator Move_Storage(ForwardIterator first, ForwardIterator last, ForwardIterator result)
    {
        assert(std::is_pointer<ForwardIterator>::value && std::is_pointer<ForwardIterator>::value && std::is_pointer<ForwardIterator>::value 
            && "Moving storage is currently only valid on iterators of pointer type"
        );
        assert((result < first || result > last) && "result is in overlapping range. Use Copt_To instead");
        
        typedef IteratorTraits<ForwardIterator>::value_type value_type;

        auto count = (last - first);
        std::memmove(eastl::addressof(*result), eastl::addressof(*first), sizeof(value_type) * count);
        return result + count;
    }
    
    template<typename ForwardIterator>
    inline void Destroy_In_Storage_Impl(ForwardIterator first, ForwardIterator last, std::false_type)
    {
        //is a clas type and has a destructor
        typedef typename IteratorTraits<ForwardIterator>::value_type value_type;
        ForwardIterator current = first;
        while(current != last)
        {
            (*current).~value_type();
            ++current;
        }
    }
    
    template<typename ForwardIterator>
    inline void Destroy_In_Storage_Impl(ForwardIterator first, ForwardIterator last, std::true_type)
    {
        //trivial type. no need to handle
    }

    template<typename ForwardIterator>
    inline void Destroy_In_Storage_Impl(ForwardIterator first, std::false_type)
    {
        typedef typename IteratorTraits<ForwardIterator>::value_type value_type;
        (*first).~value_type();
    }
    template<typename ForwardIterator>
    inline void Destroy_In_Storage_Impl(ForwardIterator first, std::true_type)
    {
        //trivial type. no need to handle
    }

    template<typename ForwardIterator>
    inline void Destroy_In_Storage(ForwardIterator first, ForwardIterator last)
    {
        typedef typename IteratorTraits<ForwardIterator>::value_type value_type;
        Destroy_In_Storage_Impl(first, last, std::is_trivial<value_type>::type());
    }

    template<typename ForwardIterator>
    inline void Destroy_In_Storage(ForwardIterator first)
    {
        typedef typename IteratorTraits<ForwardIterator>::value_type value_type;
        Destroy_In_Storage_Impl(first, std::is_trivial<value_type>::type());
    }
}

#endif //_H_MEMORY_HELPERS