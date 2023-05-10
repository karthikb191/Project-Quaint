#ifndef _H_Q_STATIC_STRING_W
#define _H_Q_STATIC_STRING_W
#include <wchar.h>
#include <Types/QStaticString.h>

namespace Quaint
{

template<int N>
class QStaticString_W
{
    #define WCHAR_SIZE sizeof(wchar_t)
public:
    constexpr QStaticString_W()
    {
        static_assert(N > 0, "Expected string of non-zero size");
        memset(m_rawData, '\0', N * WCHAR_SIZE);
    }

    template<size_t SZ>
    constexpr QStaticString_W(const wchar_t (&str)[SZ])
    : QStaticString_W()
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");
        wcscpy_s((wchar_t*)m_rawData, SZ, str);
    }

    const char* getRawData() const { return m_rawData; }
    size_t length() const { return wcslen(m_rawData); }
    size_t availableSize() const { return N - length(); }
    
    //TODO: Check this
    template<size_t SZ>
    void toStaticString(QStaticString<SZ>& str)
    {
        static_assert(SZ > N * WCHAR_SIZE, "Passed string might not be big enough to hold wchar entirely");
        wcstombs(str.getRawData(), m_rawData, (N * WCHAR_SIZE) - 1);
    }

    template<size_t SZ>
    bool append(const QStaticString_W<SZ>& other)
    {
        size_t otherLen = other.length() + 1;
        if(availableSize() < otherLen)  return false;

        wcscat_s(m_rawData, length() + otherLen, other.getRawData())
        return true;
    }
    bool append(const wchar_t* str)
    {
        size_t otherLen = strlen(str) + 1;
        if(availableSize() < otherLen) return false;

        wcscat_s(m_rawData, length() + otherLen, str);
        return true;
    }

    template<size_t SZ>
    QStaticString_W& operator=(const char (&str)[SZ])
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");
        memset(m_rawData, '\0', N * WCHAR_SIZE);
        wcscpy_s(m_rawData, SZ, str);
        return *this;
    }
    template<size_t SZ>
    QStaticString_W& operator=(const QStaticString_W<SZ>& other)
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");

        memset(m_rawData, '\0', N * WCHAR_SIZE);
        wcscpy_s(m_rawData, other.getRawData(), SZ);
        return *this;
    }

private:
    wchar_t        m_rawData[N];
};


typedef QStaticString_W<64> QName_W;
}
#endif //_H_Q_STATIC_STRING_W