#include <iostream>
#include <vector>
#include <QuaintLogger.h>
#include <MemoryDefinitions.h>

int main()
{
    std::cout << "Hello Memory Manager\n";
    Quaint::RegisterMemoryPartitions();
    
    Quaint::MemoryManager::get()->initialize();
    int validContexts = Quaint::MemoryManager::getValidContexts();
    std::cout << "Valid Contexts: " << validContexts << std::endl;
    for(int i = 0; i < validContexts; i++)
    {
        Quaint::MemoryContext* Context = (Quaint::MemoryManager::getMemoryContexts() + i);
        std::cout << "Context: " <<  Context->m_name << " " << Context->m_size << " " << Context->m_valid << std::endl;
    }
}