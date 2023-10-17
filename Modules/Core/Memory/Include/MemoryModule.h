#ifndef _H_MEMORY_MODULE
#define _H_MEMORY_MODULE
#include <MemoryManager.h>
#include <MemoryDefinitions.h>
#include <Module.h>
#include <string>
#include <MemCore/GlobalMemoryOverrides.h>

namespace Quaint
{
    //TODO: Extend from module class
    class MemoryModule : public Module<MemoryModule>
    {
        BEFRIEND_MODULE(MemoryModule);
    
    public:
        MemoryManager& getMemoryManager()
        {
            return m_memoryManager;
        }

protected:
        void initModule_impl() override
        {
            Module<MemoryModule>::initModule_impl();
        }

        void shutdown_impl() override
        {
            Module<MemoryModule>::shutdown_impl();
            m_memoryManager.shutdown();
        }

    private:
        MemoryModule()
        {
            m_memoryManager.initialize();
        }
        virtual ~MemoryModule() = default;
        MemoryModule(const MemoryModule&) = delete;
        MemoryModule(const MemoryModule&&) = delete;
        MemoryModule& operator= (const MemoryModule&) = delete;
        MemoryModule& operator= (const MemoryModule&&) = delete;

        MemoryManager       m_memoryManager;
        
    };
}

#endif //_H_MEMORY_MODULE