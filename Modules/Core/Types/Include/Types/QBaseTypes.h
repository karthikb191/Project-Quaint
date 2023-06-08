#ifndef _H_Q_BASE_TYPES
#define _H_Q_BASE_TYPES

#if defined (QUAINT_PLATFORM_WIN32) || defined (QUAINT_PLATFORM_WIN64)
#include <intrin.h>
#endif


#ifdef INTRINSICS_SUPPORTED

#if defined (QUAINT_PLATFORM_WIN32) || defined (QUAINT_PLATFORM_WIN64)
    #define _i2x32      __m64
    
    #define _f4x32      __m128
    
    #define _i4x32      __m128i
    #define _i8x16      __m128i
    #define _i16x8      __m128i

    #define _d2x64      __m128d
#ifdef __AVX2__
    #define _f
#else
    #define _f8x32      __m128[2]
    #define _i8x32      __m128i[2]

#endif

#endif

#else


#endif //INTRINSICS_SUPPORTED

#endif //_H_Q_BASE_TYPES