#include <fstream>
#include <assert.h>
#include <math.h>

namespace qcodec
{
    /*This is mainly cuz we don't want to go beyond 5 bits precision*/
    #define MAX_SYMBOLS 257
    static uint32_t CUMULATIVE_FREQ[MAX_SYMBOLS] = {0};
    static uint32_t ARITH_DANGER_ZONE = (0xFFFFFFFF / 4 - 1);
    static uint32_t MAX_CUMULATIVE_FREQ = ARITH_DANGER_ZONE / 2;
    static uint32_t ARITH_TOP = MAX_CUMULATIVE_FREQ * 2;
    static uint32_t ARITH_HALF = ARITH_TOP / 2;
    static uint32_t ARITH_QUARTER = ARITH_HALF / 2;

    void modifyArithmeticLimits(uint32_t cumulativeFreqency)
    {
        assert((cumulativeFreqency < (0xFFFFFFFF / 4) - 1) && "Sorry. Cumulative frequency went out of bounds");
        MAX_CUMULATIVE_FREQ = cumulativeFreqency;
        ARITH_TOP = MAX_CUMULATIVE_FREQ * 4; // Need Some space to expand
        ARITH_HALF = ARITH_TOP / 2;
        ARITH_QUARTER = ARITH_HALF / 2; /*Entire symbol frequency should ideally fit in one quarter*/
    }

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
        return (((char*)inBuf)[byteIdx] & mask) ? 1 : 0;
    }

    void writeBitAndAdvance(void* outBuf, const uint32_t length, const bool val, uint32_t& byteIdx, uint8_t& bitIdx)
    {
        assert(byteIdx < length && "Overflow on buffer bounds");
        uint8_t maskVal = (val == 0) ? 0 : 1 << (bitIdx % 8); 
        ((char*)outBuf)[byteIdx] |= maskVal;
        if(++bitIdx % 8 == 0)
        {
            bitIdx = 0;
            ++byteIdx;
        }
    }

    inline void buildFrequencyTableFromData(const void* inbuf, const uint32_t length, uint32_t* frequencies)
    {
        if(length == 0) return;
        
        uint32_t curPos = 0;
        while (curPos < length)
        {
            uint32_t byte = getByteAndAdvance(inbuf, length, curPos);
            ++frequencies[byte + 1];
            assert(frequencies[byte + 1] < MAX_CUMULATIVE_FREQ && "Symbol frequency went beyond permitted precision. This might cause underflow");
        }
    }

    inline void buildCumulativeFrequencies(const uint32_t frequencies[MAX_SYMBOLS])
    {
        assert(frequencies != nullptr && "Invalid out buffer");
        //double entropy = 0.0f;

        //0th position is reserved for EOF symbol
        //if(cp != nullptr) cp[0] = 1.0 / (double)length;
        CUMULATIVE_FREQ[0] = 0;
        uint32_t maxCF = 0;
        for(int i = 1; i < 257; ++i)
        {
            //TODO: This might not be correct
            // if(cp != nullptr)
            // {
            //     cp[i] = (double)frequencies[i] / (double) length;
            //     entropy += cp[i] * log2l( 1.0 / cp[i]);
            //     cp[i] += cp[i-1];
            // }

            CUMULATIVE_FREQ[i] = CUMULATIVE_FREQ[i-1] + frequencies[i];
            maxCF = CUMULATIVE_FREQ[i];
        }
        modifyArithmeticLimits(maxCF);
    }

}
