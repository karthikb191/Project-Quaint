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
#include <Types/Concurrency/QCondition.h>
#include <Types/QFastDelegates.h>
#include <Types/QFastArray.h>
#include <Types/QStaticString.h>
#include <Types/QArray.h>

class A
{
public:
    virtual void TestFunc(int var) const
    {
        std::cout << "Test Func in class A" << var << "\n";
    }
    static void StaticMemberFunc(int n)
    {
        std::cout << "Called static member Fun" << n << "\n";
    }

    virtual void TwoParams(int a, int b)
    {
        std::cout << "Two Params Test: " << a << " " << b << "\n";
    }
    
    virtual void ThreeParams(int a, int b, int c)
    {
        std::cout << "Three Params Test: " << a << " " << b << " " << c << "\n";
    }
    
    virtual void FourParams(int a, int b, int c, int d)
    {
        std::cout << "Four Params Test: " << a << " " << b << " " << c << " " << d << "\n";
    }
    
    virtual void FiveParams(int a, int b, int c, int d, int e)
    {
        std::cout << "Five Params Test: " << a << " " << b << " " << c << " " << d << " " << e << "\n";
    }
    
    virtual void SixParams(int a, int b, int c, int d, int e, int f)
    {
        std::cout << "Six Params Test: " << a << " " << b << " " << c << " " << d << " " << e << " " << f << "\n";
    }
    
    virtual void SevenParams(int a, int b, int c, int d, int e, int f, int g) const
    {
        std::cout << "Seven Params Test: " << a << " " << b << " " << c << " " << d << " " << e << " " << f  << " " << g << "\n";
    }
};

class B : public A
{
public:
    virtual void TestFunc(int var) const
    {
        std::cout << "Test Func in class B" << var << "\n";
    }
};

class Test
{
public:
    int i = 100;
    char J = 'c';
    long k = 1994724892;
    std::string test = "sdufhskfhskdfhsd3jfs";
    char* ptr = nullptr;
};

Quaint::QCondition condition;

void TestJob(void* param)
{
    for(int i = 0; i < 1000; i++)
    {
        std::cout << i << "  ";
    }
    std::cout << "\nFinished Processing in one thread\n\n\n";
    condition.signal();
}


namespace Quaint
{
    void printArray(const QArray<int>& arr)
    {
        for(auto itr = arr.begin_c(); itr != arr.end_c(); ++itr)
        {
            std::cout << *itr << " ";
        }
        std::cout << "\n";
    }
    void QArrayTests()
    {
        MemoryManager* memoryManager = &MemoryModule::get().getMemoryManager();
        QArray<int> arr1(memoryManager->getDefaultMemoryContext());
        arr1.pushBack(0);
        arr1.pushBack(1);
        arr1.pushBack(2);
        arr1.pushBack(3);
        arr1.pushBack(4);
        arr1.pushBack(5);
        arr1.pushBack(6);
        arr1.pushBack(7);
        arr1.pushBack(8);
        arr1.pushBack(9);

        printArray(arr1);

        arr1.removeAt(9);
        printArray(arr1);

        arr1.removeRange(arr1.begin(), arr1.begin() + 3);
        printArray(arr1);

        arr1.insertRangeAt(0, {90, 80, 70, 60, 50});
        printArray(arr1);

        arr1.insertAt(arr1.getSize() - 1, 787);

        QArray<int> arr2(arr1);
        printArray(arr2);

        arr2.insertAt(0, 101);
        arr2.insertAt(0, 102);
        arr2.insertAt(0, 103);
        arr2.pushBack(104);
        arr2.pushBack(105);
        arr2.pushBack(106);
        arr2.insertAt(arr2.getSize()/2, 107);
        printArray(arr2);

        arr1.insertRangeAt(5, arr2, arr2.begin_c(), arr2.end_c() - 1);

        QArray<int> arr3(std::move(arr1));
        printArray(arr3);

        std::cout << "AT: " << arr3.at(3) << "\n";

        printArray(arr1);
    }
}

int main()
{
    
    std::cout << "Hello Memory Manager\n";

    Quaint::QStaticString<64> str("Test Test");
    
    Quaint::QArrayTests();

    Quaint::QStaticString<64> str1 = str;
    str1.append("lloo");
    str1 = "Test Again";

    Quaint::QStaticString<4> str2("One");
    str1 = str2;
    
    int validContexts = Quaint::MemoryModule::get().getMemoryManager().getValidContexts();
    Quaint::MemoryManager* memoryManager = &Quaint::MemoryModule::get().getMemoryManager();
    Quaint::IMemoryContext* context = memoryManager->getDefaultMemoryContext();
    using namespace Quaint;
    A a;
    B b;
    auto del = CREATE_AND_BIND_DELEAGATE_1(&a, &A::TestFunc, context);
    del(100);
    A* bder = &b;
    auto del1 = CREATE_AND_BIND_DELEAGATE_1(bder, &A::TestFunc, context);
    del1(200);

    auto del2 = CREATE_AND_BIND_DELEAGATE_1(bder, &B::TestFunc, context);
    del2(250);

    auto del4 = CREATE_AND_BIND_DELEAGATE_1(&A::StaticMemberFunc, context);
    del4(400);

    
    //lambda.operator()(1000);
    auto del5 = QFastDelegate1<float, int>(context);
    float outerValue = 101.1f;
    del5.BindLambda(
        [outerValue](int param) -> float
        {    
            std::cout << outerValue << " Called Delegate " << param << "\n";
            return 101.1f;
        }
    );
    float res = del5(2000);
    std::cout << "res: " << res << "\n";

    auto del6 = del5;
    del6(4000);
    
    auto twop = CREATE_AND_BIND_DELEAGATE_2(&a, &A::TwoParams, context);
    twop(11, 12);

    auto threep = CREATE_AND_BIND_DELEAGATE_3(&a, &A::ThreeParams, context);
    threep(11, 12, 13);

    auto fourp = CREATE_AND_BIND_DELEAGATE_4(&a, &A::FourParams, context);
    fourp(11, 12, 13, 14);

    auto fivep = CREATE_AND_BIND_DELEAGATE_5(&a, &A::FiveParams, context);
    fivep(11, 12, 13, 14, 15);

    auto sixp = CREATE_AND_BIND_DELEAGATE_6(&a, &A::SixParams, context);
    sixp(11, 12, 13, 14, 15, 16);

    auto sevenp = CREATE_AND_BIND_DELEAGATE_7(&a, &A::SevenParams, context);
    sevenp(11, 12, 13, 14, 15, 16, 17);

    DECLARE_DELEGATE_Seven_Params(MyOwnDel, void, int, int, int, int, int, int, int);
    MyOwnDel dd(context);
    dd.Bind(&a, &A::SevenParams);
    dd(101, 102, 103, 104, 105, 106, 107);



    
    Quaint::ThreadParams params;
    params.m_job = Quaint::ThreadParams::JobType(TestJob);
    params.m_threadInitState = Quaint::EThreadInitState::Started;

    Quaint::QThread Thread1;
    Thread1.initializeThread(params);
    Thread1.run();
    condition.wait();

    Quaint::QThread Thread2;
    Thread2.initializeThread(params);
    Thread2.run();
    condition.wait();

    Quaint::QThread Thread3;
    Thread3.initializeThread(params);
    Thread3.run();
    condition.wait();

    Quaint::QThread Thread4;
    Thread4.initializeThread(params);
    Thread4.run();
    condition.wait();

    Thread1.join();
    Thread2.join();
    Thread3.join();
    Thread4.join();

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

    int* temp = new int(100);
    delete temp;

    auto before = std::chrono::high_resolution_clock::now();
    int* testInt[10000];
    int* testInt2[10000];

    Test* testStruct = nullptr;
    for(int i = 0; i < 10000; i++)
    {
        if( i == 300)
        {
            testStruct = new Test();
        }
        testInt[i] = new int(i);
    }

    for(int i = 0; i < 10000; i++)
    {
        if( i%50 == 0 )
        {
            delete testInt[i];
        }
    }

    Quaint::RBTree::print();

    std::cout <<"\n\n\n\n";

    //for(int i = 0; i < 10000; i++)
    //{
    //    if(i%2 != 0)
    //    {
    //        std::cout << *testInt[i] << "\n";
    //    }
    //}

    for(int i = 0; i < 100; i++)
    {
        testInt2[i] = new int(i);
    }

    Quaint::RBTree::print();

    auto after = std::chrono::high_resolution_clock::now();
    
    std::cout << "Time: " << (after - before).count() << std::endl;
    

#pragma region MemoryTracker
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
#pragma endregion

using namespace Quaint;
    SHUTDOWN_MODULE(LoggerModule);

    //TODO: Kill all running threads
    return 0;
}

