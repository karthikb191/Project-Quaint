#ifndef _H_I_ALLOCATION_Technique
#define _H_I_ALLOCATION_Technique

class IAllocationTechnique
{
public:
    virtual void boot(const char* ContextName, size_t size, void* rawMemory, bool dynamic = false) = 0;
    virtual void reboot(size_t size, void* rawMemory) = 0;
    virtual void* alloc(size_t allocSize) = 0;
    virtual void free(void* mem) = 0;
    virtual void shutdown() = 0;
    size_t getAvailableSize() { return m_availableSize; }
    virtual ~IAllocationTechnique() {};
protected:
    size_t          m_availableSize = 0;
};

#endif //_H_I_ALLOCATION_Technique