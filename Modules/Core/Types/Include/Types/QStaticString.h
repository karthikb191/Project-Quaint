#ifndef _H_Q_STATIC_STRING
#define _H_Q_STATIC_STRING

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
    
    template<size_t SZ>
    constexpr QStaticString(const char (&str)[SZ])
    : QStaticString()
    {
        static_assert(SZ > 0, "Assigning an empty string is invalid");
        static_assert(N >= SZ, "Passed string is larger than the type can hold");
        strcpy_s(m_rawData, SZ, str);
    }

    const char* getRawData() const { return m_rawData; }
    size_t length() const { return strlen(m_rawData); }
    size_t availableSize() const { return N - length(); }

    template<size_t SZ>
    bool append(const QStaticString<SZ>& other)
    {
        size_t otherLen = other.length() + 1;
        if(availableSize() < otherLen)  return false;

        strcat_s(m_rawData, length() + otherLen, other.getRawData());
        return true;
    }
    bool append(const char* str)
    {
        size_t otherLen = strlen(str) + 1;
        if(availableSize() < otherLen) return false;

        strcat_s(m_rawData, length() + otherLen, str);
        return true;
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
        memcpy(m_rawData, other.getRawData(), SZ);
        return *this;
    }
    

private:
    char        m_rawData[N];
};

}
#endif