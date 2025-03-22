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

    template<int SZ>
    class QCTString;

    template<int SZ>
    constexpr const QCTString<SZ> createCTString(const char(&str)[SZ]);
    template<int SZ>
    constexpr const QCTString<SZ> createEmptyCTString(const char(&str)[SZ]);

    //TODO: Improve upon this
    /* Compile-Time string */
    template<int N>
    class QCTString
    {
    public:
        constexpr QCTString()
        : buffer{'\0'}
        {}

        template<int ... ARGS>
        constexpr QCTString(const ArgsPack<ARGS...>)
        {
        }
        
        template<size_t SZ>
        constexpr QCTString(const char (&str)[SZ])
        : buffer { }
        {
            for(int i = 0; i < SZ; ++i)
            {
                buffer[i] = str[i];
            }
        }

        //template<int ... ARGS>
        //constexpr QCTString(const char* c)
        ////: buffer { c[ARGS]... }
        //{
        //    for(int i = 0; i < N; ++i)
        //    {
        //        buffer[i] = c[i];
        //    }
        //}
        template<int ... ARGS>
        constexpr QCTString(const int P, const int Q, const char* first, const char* second, const ArgsPack<ARGS...>)
        //: buffer { '\0' }
        {
            for(int i = 0; i < P - 1; ++i)
            {
                buffer[i] = first[i];
            }
            for(int i = 0; i < Q; ++i)
            {
                buffer[P+i-1] = second[i];
            }
        }

        const int getSize() const { return N; }
        const char* getBuffer() const { return buffer; }

        template<int M>
        constexpr bool compare(const QCTString<M>& other) const
        {
            if(N != M) return false;
            for(int i = 0; i < N; ++i)
            {
                if(buffer[i] != other.buffer[i])
                {
                    return false;
                }
            }
            return true;
        }
        template<int M>
        constexpr bool compare(const char(&other)[M]) const
        {
            if(N != M) return false;
            for(int i = 0; i < N; ++i)
            {
                if(buffer[i] != other[i])
                {
                    return false;
                }
            }
            return true;
        }

        template<const int M>
        constexpr QCTString<N+M-1> concat(const QCTString<M>& other) const
        {
            assert((M > 0 && N > 0) && "invalid strings given");
            constexpr int SZ = N + M - 1;
            return QCTString<SZ>(N, M, buffer, other.buffer, GetIndices<SZ>::res());
        }
    private:
        char buffer[N] = { '\0' };
    };
    
    /* Helper to create a compile-time object with given sequence of characters from input const array */
    /* Should ideally only use helpers to create QCTString Objects */
    template<int SZ>
    constexpr const QCTString<SZ> createCTString(const char(&str)[SZ])
    {
        return QCTString<SZ>(str);
    }

    template<int SZ>
    constexpr const QCTString<SZ> createEmptyCTString()
    {
        return QCTString<SZ>(GetIndices<SZ>::res());
    }
}

#endif //_H_Q_CT_STRING