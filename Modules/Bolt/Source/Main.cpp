#include <iostream>
#include "../Include/Bolt.h"
#include <vulkan/vulkan.h>

int main()
{
    std::cout << "Hello Renderer!" << std::endl;
    Bolt::BoltRenderer::get()->startEngine();
    
    uint32_t instanceCount = 0;
    
    Bolt::BoltRenderer::get()->shutdown();
    
    return 0;
}