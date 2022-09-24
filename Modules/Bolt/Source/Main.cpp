#include <iostream>
#include "../Include/Bolt.h"

int main()
{
    std::cout << "Hello Renderer!" << std::endl;
    Bolt::BoltRenderer::get()->startEngine();
    Bolt::BoltRenderer::get()->shutdown();
    return 0;
}