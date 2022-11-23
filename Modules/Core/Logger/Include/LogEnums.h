#ifndef _H_LOG_ENUMS
#define _H_LOG_ENUMS
#include <cstdint>
namespace Quaint
{
    enum class Category : uint8_t
    {
        VeryVerbose = 0,
        Verbose = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
        Default = 6
    };
}

#endif //_H_LOG_ENUMS