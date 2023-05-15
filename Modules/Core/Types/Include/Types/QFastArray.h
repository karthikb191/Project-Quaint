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
    //TODO: Add Iterator support

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

    const T* getRawData() const { return m_data; }
    
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

template<typename T, size_t SZ>
static QFastArray<T,SZ> createFastArray(const T(&list)[SZ])
{
    return QFastArray<T, SZ>(list);
}

}
#endif