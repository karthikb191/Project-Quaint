#ifndef _H_SINGLETON
#define _H_SINGLETON

namespace Quaint
{
    template <class T>
    class Singleton
    {
    public:
        virtual ~Singleton() = default;
        static T *Get()
        {
            if (m_dataPtr == nullptr)
            {
                m_dataPtr = new T();
            }
            return m_dataPtr;
        }

    private:
        Singleton() = default;
        Singleton(const Singleton &) = delete;
        Singleton(Singleton &&) = delete;
        Singleton &operator=(const Singleton &) = delete;
        Singleton &operator=(Singleton &&) = delete;

        friend typename T;
    protected:
        static T *m_dataPtr;
    };
}

#define DECLARE_SINGLETON(TYPE) friend typename Singleton<TYPE>;
#define DEFINE_SINGLETON(TYPE) TYPE *Singleton<TYPE>::m_dataPtr = nullptr;

#endif //_H_SINGLETON