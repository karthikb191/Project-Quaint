#include <iostream>
#include <vulkan/vulkan.h>
#include <BoltRenderer.h>
#include <Types/QFastArray.h>
#include <Types/QStaticString.h>
#include <Types/QStaticString_W.h>

#include <MemoryModule.h>
#include <LoggerModule.h>
#include <RenderModule.h>
#include <Core/Camera.h>

namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);
    
    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}

namespace Bolt
{
    CREATE_MODULE(RenderModule);
    INIT_MODULE(RenderModule);
}

int main()
{
    std::cout << "Hello Renderer!" << std::endl;
    
    Quaint::QStaticString_W<64> testStr(L"Test Test");
    testStr.length();

    Bolt::RenderModule::get().start(Quaint::MemoryModule::get().getMemoryManager().getDefaultMemoryContext());
    
    //TODO: Loop through application module 
    Bolt::RenderModule::get().getBoltRenderer()->update();

    Bolt::RenderModule::get().stop();
    Bolt::RenderModule::shutdown();
    
    //SHUTDOWN_MODULE(RenderModule);
    //Bolt::BoltRenderer::get()->shutdown();
    
    return 0;
}