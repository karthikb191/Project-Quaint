#ifndef _H_BOLT_MEMORY_PROVIDER
#define _H_BOLT_MEMORY_PROVIDER

namespace Quaint
{
    class IMemoryContext;
}

#define BOLT_ALLOCATOR Quaint::QAllocatorBase(Bolt::G_BOLT_DEFAULT_MEMORY, "BoltAllocator")

namespace Bolt
{
    extern Quaint::IMemoryContext* G_BOLT_DEFAULT_MEMORY;
}

#endif //_H_BOLT_MEMORY_PROVIDER