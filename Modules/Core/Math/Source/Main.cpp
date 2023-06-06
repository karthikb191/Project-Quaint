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
#include <Types/QRBTree.h>
#include <Types/QMap.h>
#include <Types/QSet.h>

int main()
{
    std::cout << "Hello Math module\n";
    return 0;
}