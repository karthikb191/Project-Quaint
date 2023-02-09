#ifndef _H_GLOBAL_MEMORY_OVERRIDES
#define _H_GLOBAL_MEMORY_OVERRIDES

//TODO: Later move this to a precompiled header

void* operator new(size_t size, const char* contextName);
void operator delete(void* mem, const char* contextName);

void* operator new(size_t size, int contextId);
void operator delete(void* mem, int contextId);

#endif //_H_GLOBAL_MEMORY_OVERRIDES