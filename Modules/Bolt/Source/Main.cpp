#include <iostream>
#include "../Include/Bolt.h"
#include <vulkan/vulkan.h>
int main()
{
    std::cout << "Hello Renderer!" << std::endl;
    Bolt::BoltRenderer::get()->startEngine();
    
    uint32_t instanceCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceCount, nullptr);
    std::cout << "VK Instances : " << instanceCount << std::endl;
    
    Bolt::BoltRenderer::get()->shutdown();
    
    return 0;
}