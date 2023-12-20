#include <fstream>
#include <assert.h>
#include <math.h>

namespace qcodec
{
    /*This is mainly cuz we don't want to go beyond 5 bits precision*/
    #define MAX_CUMULATIVE_FREQ (0xFFFF / 2) - 1

    /* All the functions here dont maintain a state */

    /*Advances the given byteIdx and retrieved the value as uint32_t*/
    inline uint8_t getByteAndAdvance(const void* inBuf, const uint32_t length, uint32_t& byteIdx)
    {
        assert(byteIdx < length && "Reached the end of buffer given");
        ++byteIdx;
        return (uint8_t) ((char*)inBuf)[byteIdx];
    }

    /* Advances the given byte and bit position */
    inline uint8_t getBitAndAdvance(const void* inBuf, const uint32_t length, uint32_t byteIdx, uint8_t& bitIdx)
    {
        assert(byteIdx < length && "Reached the end of buffer given");
        uint8_t mask = 1 << (bitIdx % 8);
        ++bitIdx;
        if(bitIdx % 8 == 0)
        {
            bitIdx = 0;
            ++byteIdx;
        }
        return ((char*)inBuf)[byteIdx] & mask;
    }

    inline void buildFrequencyTableFromData(const void* inbuf, const uint32_t length, uint32_t* frequencies)
    {
        if(length == 0) return;

        uint32_t curPos = 0;
        while (curPos < length)
        {
            uint32_t byte = getByteAndAdvance(inbuf, length, curPos);
            ++frequencies[byte];
            assert(frequencies[byte] < MAX_CUMULATIVE_FREQ && "Symbol frequency went beyond permitted precision. This might cause underflow");
        }
    }

    inline double buildCumulativeProbabilitesFromFrequencies(const uint32_t* frequencies, const uint32_t length, double* cp)
    {
        double entropy = 0.0f;

        //0th position is reserved for EOF symbol
        cp[0] = 1.0 / (double)length; 
        for(int i = 1; i < 257; ++i)
        {
            cp[i] = (double)frequencies[i] / (double) length;
            entropy += cp[i] * log2l( 1.0 / cp[i]);
            cp[i] += cp[i-1];
        } 
        return entropy;
    }

}
