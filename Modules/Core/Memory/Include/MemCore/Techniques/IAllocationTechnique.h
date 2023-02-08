#ifndef _H_I_ALLOCATION_Technique
#define _H_I_ALLOCATION_Technique

class IAllocationTechnique
{
public:
    virtual void boot(size_t size, void* rawMemory, bool dynamic = false) = 0;
    virtual void reboot(size_t size, void* rawMemory) = 0;
    virtual void* alloc(size_t allocSize) = 0;
    virtual void free() = 0;
};

#endif //_H_I_ALLOCATION_Technique