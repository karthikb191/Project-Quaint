#include <Module.h>
#include <MemoryModule.h>
#include <QuaintLogger.h>
#include <windows.h>
//Initialize all modules before doing anything.
namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);

    CREATE_MODULE(IPCModule);
    INIT_MODULE(IPCModule);
}

#include <IPCModule.h>

#include <iostream>
#include <vector>
#include <chrono>

#include <MemCore/Techniques/BestFitPoolAllocTechnique.h>

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

    auto before = std::chrono::high_resolution_clock::now();
    //int* testInt[10000];

    Quaint::RBTree tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    tree.insert(1);
    tree.insert(78);
    tree.insert(190);

    tree.insert(7);
    tree.insert(7);
    tree.insert(7);

    tree.print();

    //Test* testStruct = nullptr;
    //for(int i = 0; i < 10000; i++)
    //{
    //    if( i == 300)
    //    {
    //        testStruct = new Test();
    //    }
    //    testInt[i] = new int(10);
    //}
    //auto after = std::chrono::high_resolution_clock::now();
    //
    //std::cout << "Time: " << (after - before).count() << std::endl;
    
    //const Quaint::SharedMemoryHandle* handle = 
    //Quaint::IPCModule::get()
    //.getIPCManager()->requestSharedMemory("TestMemoryMap", Quaint::ESharedMemoryType::SharedOSMemory, 10 * 1024 * 1024);

    //if(handle == nullptr)
    //{
    //    std::cout << "Could not allocate shared memory!" << std::endl;
    //}
    //else
    //{
    //    Quaint::MemoryModule::get().getMemoryManager().populateTrackerInformation(handle->m_dataBuffer);
    //}

    //int i = 0;
    //while(true)
    //{
    //    Sleep(10);
    //    ++i;
    //    std::cout << "Woke up!!" << std::endl;
    //    //if(i == 10)
    //    //    break;
    //}

    //if(handle != nullptr)
    //{
    //    Quaint::IPCModule::get().getIPCManager()->releaseSharedMemory(handle);
    //}

using namespace Quaint;
    SHUTDOWN_MODULE(LoggerModule);

    //Calling this here is crashing the application. Probably cuz destructors of global statics now point to invalid memory location
    //SHUTDOWN_MODULE(MemoryModule);    

    return 0;
}

