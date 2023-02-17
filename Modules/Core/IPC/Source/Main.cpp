#include<Module.h>

#include <QuaintLogger.h>
#include <MemoryModule.h>

namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}


#include <iostream>

int main()
{
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    std::cout << "Hello IPC" << std::endl;
    return 0;
}