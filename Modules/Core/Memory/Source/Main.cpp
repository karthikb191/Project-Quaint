//#include "MemCore/GlobalMemoryOverrides.cpp"
#include <iostream>
#include <vector>
#include <QuaintLogger.h>
#include <MemoryDefinitions.h>
#include <chrono>

int main()
{
    std::cout << "Hello Memory Manager\n";
    Quaint::RegisterMemoryPartitions();
    
    Quaint::MemoryManager::initialize();
    
    int validContexts = Quaint::MemoryManager::getValidContexts();
    std::cout << "Valid Contexts: " << validContexts << std::endl;
    //for(int i = 0; i < validContexts; i++)
    //{
    //    Quaint::MemoryContext* Context = (Quaint::MemoryManager::getMemoryContexts() + i);
    //    std::cout << "Context: " <<  Context->m_name << " " << Context->m_size << " " << Context->m_valid << std::endl;
    //}

    auto before = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000; i++)
    {
        int* temp = new int(10);
    }
    auto after = std::chrono::high_resolution_clock::now();

    std::cout << "Time: " << (after - before).count() << std::endl;
    Quaint::MemoryManager::shutdown();
}