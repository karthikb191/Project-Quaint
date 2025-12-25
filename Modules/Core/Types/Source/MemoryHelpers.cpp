#include <Types/MemoryHelpers.h>
#include <MemoryModule.h>
#include <MemoryManager.h>
#include <MemoryConstants.h>

namespace Quaint
{
    Quaint::IMemoryContext* G_DEFAULT_MEMORY = Quaint::MemoryModule::get().getMemoryManager().getMemoryContextByName(DEFAULT_MEMORY_NAME);
}