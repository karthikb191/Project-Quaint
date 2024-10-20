#ifndef _H_Q_STATIC_STRING
#define _H_Q_STATIC_STRING
#include <ostream>
#include <assert.h>

namespace Quaint
{

template<int N>
class QStaticString
{
public:
    constexpr QStaticString()
    {
        static_assert(N > 0, "Expected string of non-zero size");
        memset(m_rawData, '\0', N);
    }

    constexpr QStaticString(const QStaticString<N>& other)
    {
        strcpy_s(m_rawData, N, other.getBuffer());
    }
    
    template<size_t SZ>
    constexpr QStaticString(const char (&str)[SZ])
    : QStaticString()
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");
        strcpy_s(m_rawData, SZ, str);
    }

    const char* getBuffer() const { return m_rawData; }
    char* getBuffer_NonConst() { return m_rawData; }
    size_t length() const { return strlen(m_rawData); }
    size_t availableSize() const { return N - length(); }

    template<size_t SZ>
    bool append(const QStaticString<SZ>& other)
    {
        size_t otherLen = other.length() + 1;
        assert(availableSize() >= otherLen && "Non fatal error! Append will fail. Total characters exceeded size");
        if(availableSize() < otherLen) 
        {
            return false;
        }

        strcat_s(m_rawData, length() + otherLen, other.getRawData());
        return true;
    }
    bool append(const char* str)
    {
        size_t otherLen = strlen(str) + 1;
        assert(availableSize() >= otherLen && "Non fatal error! Append will fail. Total characters exceeded allocated size");
        if(availableSize() < otherLen) return false;

        strcat_s(m_rawData, length() + otherLen, str);
        return true;
    }

    QStaticString& operator=(const char* other)
    {
        uint32_t size = strlen(other) + 1;
        assert(size < N && "character sequence doesn't fit in this object");
        memset(m_rawData, '\0', N);
        strcpy_s(m_rawData, size, other);
        return *this;
    }
    template<size_t SZ>
    QStaticString& operator=(const char (&str)[SZ])
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");
        memset(m_rawData, '\0', N);
        strcpy_s(m_rawData, SZ, str);
        return *this;
    }
    template<size_t SZ>
    QStaticString& operator=(const QStaticString<SZ>& other)
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");

        memset(m_rawData, '\0', N);
        memcpy(m_rawData, other.getBuffer(), SZ);
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const QStaticString<N>& str)
    {
        os << str.m_rawData;
        return os;
    }
private:
    char        m_rawData[N];
};

typedef QStaticString<64> QName;
typedef QStaticString<1024> QPath;
typedef QStaticString<256> QString256;
}
#endif