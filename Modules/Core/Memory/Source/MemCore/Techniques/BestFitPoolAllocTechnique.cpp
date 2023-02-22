#include <MemCore/Techniques/BestFitPoolAllocTechnique.h>
namespace Quaint
{
    RBNode m_sentinel = RBNode(-1);
    void BestFitPoolAllocTechnique::boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic)
    {
        m_availableSize = size;
        //TODO: Assert available size is greater than a certain threshold and power of 2
        if(m_availableSize < 1024)
        {
            return;
            //TODO: Throw an assert here. Dont log
            //QLOG_E(DefaultAllocLogger, "Boot Failed! partition size provided is not sufficient");
        }

        //TODO: initialize RB tree with a free node and push it into doubly linked list

        //Creates initial Memory chunk
        m_isRunning = true;
    }
    void BestFitPoolAllocTechnique::reboot(size_t size, void* rawMemory)
    {

    }
    void* BestFitPoolAllocTechnique::alloc(size_t allocSize)
    {
        return nullptr;
    }
    void BestFitPoolAllocTechnique::free(void* mem)
    {

    }
    void BestFitPoolAllocTechnique::shutdown()
    {

    }
}