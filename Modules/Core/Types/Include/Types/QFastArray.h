#ifndef _H_Q_FAST_ARRAY
#define _H_Q_FAST_ARRAY

#include <assert.h>

namespace Quaint
{
/*
* Array of static size with minimal operations
*/
template<typename T, size_t N>
class QFastArray
{
public:
    using value_type        = T;
    using reference         = const T&;
    using const_reference   = const T&;
    using size_type         = size_t;
    using Iterator          = T*;
    using Const_Iterator    = const T*;

    explicit constexpr QFastArray()
    : m_data()
    {
        static_assert(N > 0, "Expected a positive non empty array size");
    }
    
    template<size_t SZ>
    explicit constexpr QFastArray(const T(&list)[SZ])
    : QFastArray()
    {
        static_assert(SZ == N, "Value initalizer list did not match with declared array size");
        for(size_t i = 0; i < N; i++)
        {
            m_data[i] = list[i];
        }
    }
    template<typename ...ARGS>
    constexpr QFastArray(ARGS... args)
    : m_data{args...}
    {
        static_assert(N > 0, "Expected a positive non empty array size");
        static_assert(sizeof...(ARGS) == N, "Value initalizer list did not match with declared array size");
    }
    
    ~QFastArray() = default;

    Iterator begin() { return (T*)m_data; }
    Iterator end() { return (T*)m_data + N; }
    Const_Iterator begin() const { return (T*)m_data; }
    Const_Iterator end() const { return (T*)m_data + N; }

    constexpr void operator=(const T(&list)[N])
    {
        for(size_t i = 0; i < N; i++)
        {
            m_data[i] = list[i];
        }
    }

    constexpr T& operator[](size_t index)
    {
        return m_data[index];
    }
    
    constexpr size_t getSize()
    {
        return N;
    }
    bool isEmpty() { return m_size == 0; }

    const T* getBuffer() const { return m_data; }
    T* getBuffer_NonConst() { return m_data; }
    
    //Explicit copy
    template<size_t SZ>
    void copyFrom(const QFastArray<T, SZ>& src)
    {
        static_assert(N >= SZ, "Copy to destination array of smaller size not supported");
        memcpy((void*)m_data, (void*)src.getRawData(), SZ * sizeof(T));
    }

    //No default copy and move
    QFastArray(const QFastArray&) = default;
    QFastArray(QFastArray&&) = default;
    QFastArray& operator=(const QFastArray&) = default;
    QFastArray& operator=(QFastArray&&) = default;

private:
    T                   m_data[N];
};

//This throws an internal compiler error when initializing global const char* array
template<typename T, size_t SZ>
constexpr QFastArray<T,SZ> createFastArray(const T(&list)[SZ])
{
    return QFastArray<T, SZ>(list);
}

// Work-around for internal compiler error when creating a global const char* array. 
// Usage: auto arr = createFastArray<const char*> ("one", "two", "three");
template<typename T, typename ...ARGS>
QFastArray<T, sizeof...(ARGS)> createFastArray(ARGS... args)
{
     return QFastArray<T, sizeof...(ARGS)>(args...);
}

}
#endif