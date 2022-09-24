#include <iostream>
#include <Bolt.h>
int main()
{
    std::cout << "Hello Engine!" << std::endl;
    Bolt::BoltRenderer::get()->startEngine();
    Bolt::BoltRenderer::get()->shutdown();
    return 0;
    
}