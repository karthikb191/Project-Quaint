#ifndef _H_CODEC_GOLOMB
#define _H_CODEC_GOLOMB
#include <stdint.h>
#include <math.h>

namespace qcodec
{
#define PROCESS_BIT_GOLADAP(data, pos, mask)\
    if(((uint8_t*)data)[pos] & mask == 0){ \
        if(k > 0) { ++r; }\
        if(r % (2 * k) == 0)\
        {   /*Add To Output*/ \
            r = 0; ++k; /*Exact multiple of k*/ \
        }\
        else {\
        /*Encode 0 marking the end of the current run and followed by K bits remainder. Clear r and decrement k by one*/ \
        }\
    }\

inline void encode_goladap(void* data, uint32_t length)
{
    uint32_t pos = 0;
    
    int r = 0;  // run
    int k = 0;  // num mult of m
    while(pos != length)
    {
        PROCESS_BIT_GOLADAP(data, pos, 0);
        PROCESS_BIT_GOLADAP(data, pos, 1);
        PROCESS_BIT_GOLADAP(data, pos, 2);
        PROCESS_BIT_GOLADAP(data, pos, 3);
        PROCESS_BIT_GOLADAP(data, pos, 4);
        PROCESS_BIT_GOLADAP(data, pos, 5);
        PROCESS_BIT_GOLADAP(data, pos, 6);
        PROCESS_BIT_GOLADAP(data, pos, 7);
        ++pos;
    }
}

inline void* decode_goladap()
{

}
}

#endif //_H_CODEC_GOLOMB