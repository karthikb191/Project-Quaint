#include<Module.h>
namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);
    
    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}

#include<LoggerModule.h>
#include<MemoryModule.h>

#include <iostream>
#include <BoltRenderer.h>
int main()
{
    std::cout << "Hello Engine!" << std::endl;
    Bolt::BoltRenderer::get()->startEngine();
    Bolt::BoltRenderer::get()->shutdown();
    return 0;
}