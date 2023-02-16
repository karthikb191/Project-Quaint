#ifndef _H_MEMORY_MODULE
#define _H_MEMORY_MODULE
#include <MemoryManager.h>
#include <MemoryDefinitions.h>
#include <Module.h>
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
            m_memoryManager.initialize();
        }

        void shutdown_impl() override
        {
            m_memoryManager.shutdown();
        }

    private:
        MemoryModule() = default;
        virtual ~MemoryModule() = default;
        MemoryModule(const MemoryModule&) = delete;
        MemoryModule(const MemoryModule&&) = delete;
        MemoryModule& operator= (const MemoryModule&) = delete;
        MemoryModule& operator= (const MemoryModule&&) = delete;

        MemoryManager       m_memoryManager;
    };
    
}

#endif //_H_MEMORY_MODULE