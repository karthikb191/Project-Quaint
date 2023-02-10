#ifndef _H_MEMORY_MODULE
#define _H_MEMORY_MODULE
#include "MemoryManager.h"

namespace Quaint
{
    //TODO: Extend from module class
    class MemoryModule
    {
    public:
        static void initialize()
        {
            MemoryManager::initialize();
        }
    private:
        MemoryModule();
        ~MemoryModule();
        MemoryModule(const MemoryModule&) = delete;
        MemoryModule(const MemoryModule&&) = delete;
        MemoryModule& operator= (const MemoryModule&) = delete;
        MemoryModule& operator= (const MemoryModule&&) = delete;
    };
}

#endif //_H_MEMORY_MODULE