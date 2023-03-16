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
#include <type_traits>
#include <MemCore/Techniques/BestFitPoolAllocTechnique.h>
#include <Types/Concurrency/QThread.h>

class Test
{
public:
    int i = 100;
    char J = 'c';
    long k = 1994724892;
    std::string test = "sdufhskfhskdfhsd3jfs";
    char* ptr = nullptr;
};

void TestJob(void* param)
{
    std::cout << "Test job Running\n";
}

int main()
{
    
    std::cout << "Hello Memory Manager\n";
    
    int validContexts = Quaint::MemoryModule::get().getMemoryManager().getValidContexts(); 
    
    Quaint::ThreadParams params;
    params.m_job = Quaint::ThreadParams::JobType(TestJob);
    params.m_threadInitState = Quaint::EThreadInitState::Started;

    Quaint::QThread Thread;
    Thread.initializeThread(params);
    Thread.run();

#pragma region Test
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(10));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(20));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(5));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(1));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(78));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(190));
//
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(7));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(34));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(65));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(94));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(11));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(5));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(21));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(24));
    //Quaint::RBTree::insert(new Quaint::RBTree::RBNode(67));
//
    //Quaint::RBTree::print();
//
    //std::cout << "\n\n\n";
//
    //Quaint::RBTree::RBNode* node = Quaint::RBTree::find(7);
//
    //node = Quaint::RBTree::find(10);
    //if(node != nullptr)
    //{
    //    Quaint::RBTree::remove(node);
    //}
//
    //std::cout << "\n\n\n";
    //Quaint::RBTree::print();
//
    //node = Quaint::RBTree::find(7);
    //if(node != nullptr)
    //{
    //    Quaint::RBTree::remove(node);
    //}
    //node = Quaint::RBTree::find(20);
    //if(node != nullptr)
    //{
    //    Quaint::RBTree::remove(node);
    //}
    //node = Quaint::RBTree::find(34);
    //if(node != nullptr)
    //{
    //    Quaint::RBTree::remove(node);
    //}
    //node = Quaint::RBTree::find(1);
    //if(node != nullptr)
    //{
    //    Quaint::RBTree::remove(node);
    //}
    //std::cout << "\n\n\n";
    //Quaint::RBTree::print();
#pragma endregion

    auto before = std::chrono::high_resolution_clock::now();
    int* testInt[10000];
    Test* testStruct = nullptr;
    for(int i = 0; i < 10000; i++)
    {
        if( i == 300)
        {
            testStruct = new Test();
        }
        testInt[i] = new int(10);
    }
    auto after = std::chrono::high_resolution_clock::now();
    
    std::cout << "Time: " << (after - before).count() << std::endl;
    
    //for(int i = 0; i < 10000; i++)
    //{
    //    std::cout << *testInt[i] << "\n";
    //}

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

    //TODO: Kill all running threads
    return 0;
}

