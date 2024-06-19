#ifndef _H_Q_CT_STRING
#define _H_Q_CT_STRING

namespace Quaint
{
    /* A light-weight class that carries indices 0 - (Size - 1) individually*/
    template<int ... ARGS>
    struct ArgsPack
    {};
    
    template<int N, int ITR, int ... ARGS>
    struct IndexExtractor
    {
        typedef typename IndexExtractor<N - 1, ITR + 1, ARGS..., ITR>::res res;
    };

    template<int ITR, int ... ARGS>
    struct IndexExtractor<0, ITR, ARGS...>
    {
        typedef typename ArgsPack<ARGS...> res;
    };

    template<int N>
    struct GetIndices
    {
        typedef typename IndexExtractor<N, 0>::res res;
    };

    //TODO: Improve upon this
    /* Compile-Time string */
    template<int N>
    class QCTString
    {
    public:
        template<int ... ARGS>
        constexpr QCTString(const char* c, const ArgsPack<ARGS...>)
        : buffer { c[ARGS]... }
        {
        }

        const char* getBuffer() const { return buffer; }
    private:
        const char buffer[N] = { '\0' };
    };
    
    /* Helper to create a compile-time object with given sequence of characters from input const array */
    /* Should ideally only use helpers to create QCTString Objects */
    template<int SZ>
    constexpr const QCTString<SZ> createCTString(const char(&str)[SZ])
    {
        return QCTString<SZ>(str, GetIndices<SZ>::res());
    }
}

#endif //_H_Q_CT_STRING