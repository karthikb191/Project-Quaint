#include <Module.h>
//Initialize all modules doing anything.
namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}

#include <MemoryModule.h>
#include <QuaintLogger.h>

#include <iostream>
#include <vector>
#include <chrono>

int main()
{
    std::cout << "Hello Memory Manager\n";

    int validContexts = Quaint::MemoryModule::get().getMemoryManager().getValidContexts(); 
    std::cout << "Valid Contexts: " << validContexts << std::endl;
    //for(int i = 0; i < validContexts; i++)
    //{
    //    Quaint::MemoryContext* Context = (Quaint::MemoryManager::getMemoryContexts() + i);
    //    std::cout << "Context: " <<  Context->m_name << " " << Context->m_size << " " << Context->m_valid << std::endl;
    //}

    auto before = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; i++)
    {
        int* temp = new int(10);
    }
    auto after = std::chrono::high_resolution_clock::now();
    
    std::cout << "Time: " << (after - before).count() << std::endl;

using namespace Quaint;
    SHUTDOWN_MODULE(LoggerModule);
    SHUTDOWN_MODULE(MemoryModule);

    return 0;
}

