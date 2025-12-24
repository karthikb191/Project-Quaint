#ifndef _H_BOLT_MEMORY_PROVIDER
#define _H_BOLT_MEMORY_PROVIDER

#include <MemoryModule.h>
#include <MemoryManager.h>
#include <MemoryConstants.h>

namespace Bolt
{
    Quaint::IMemoryContext* G_BOLT_DEFAULT_MEMORY = Quaint::MemoryModule::get().getMemoryManager().getMemoryContextByName(BOLT_MEMORY_PARTITION);
}

#endif //_H_BOLT_MEMORY_PROVIDER