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

class Test
{
    int i = 100;
    char J = 'c';
    long k = 1994724892;
    std::string test = "sdufhskfhskdfhsd3jfs";
    char* ptr = nullptr;
};

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
    int* testInt[10000];

    Test* testStruct = nullptr;
    for(int i = 0; i < 10000; i++)
    {
        if( i == 100)
        {
            testStruct = new Test();
        }
        testInt[i] = new int(10);
    }
    auto after = std::chrono::high_resolution_clock::now();
    
    std::cout << "Time: " << (after - before).count() << std::endl;
    
    for(int i = 0; i < 10000; i++)
    {
        std::cout << *testInt[i] << "\n";   
    }

using namespace Quaint;
    SHUTDOWN_MODULE(LoggerModule);

    //Calling this here is crashing the application. Probably cuz destructors of global statics now point to invalid memory location
    //SHUTDOWN_MODULE(MemoryModule);    

    return 0;
}

