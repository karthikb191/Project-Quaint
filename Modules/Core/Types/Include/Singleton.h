#ifndef _H_SINGLETON
#define _H_SINGLETON
#include <assert.h>
#include <Interface/IMemoryContext.h>

template <class T>
class Singleton
{
public:
    virtual ~Singleton() = default;

    template<typename ...ARGS>
    static void Create(Quaint::IMemoryContext* context, ARGS... constructionArgs)
    {
        if(m_dataPtr != nullptr) return;

        m_context = context;
        m_dataPtr = (T*)QUAINT_ALLOC_MEMORY_ALIGNED(m_context, sizeof(T), alignof(T));
        assert(m_dataPtr != nullptr);
        new(m_dataPtr)T(constructionArgs...);
    }
    static void Shutdown()
    {
        assert(m_dataPtr != nullptr);
        m_dataPtr->~T();
        QUAINT_DEALLOC_MEMORY(m_context, m_dataPtr);
        m_dataPtr = nullptr;
        m_context = nullptr;
    }
    static T* Get()
    {
        assert(m_dataPtr != nullptr);
        return m_dataPtr;
    }
    static Quaint::IMemoryContext* GetContext() { return m_context; }

private:
    Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton(const Singleton &&) = delete;
    Singleton &operator=(const Singleton &) = delete;
    Singleton &operator=(const Singleton &&) = delete;

    friend typename T;

protected:
    static Quaint::IMemoryContext*      m_context;
    static T*                           m_dataPtr;
};

#define DECLARE_SINGLETON(TYPE) friend typename Singleton<TYPE>;


#define DEFINE_SINGLETON(TYPE) \
    TYPE* Singleton<TYPE>::m_dataPtr = nullptr;\
    Quaint::IMemoryContext* Singleton<TYPE>::m_context = nullptr;

#endif //_H_SINGLETON