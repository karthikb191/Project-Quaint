#include "../ParsingHelpers.h"
#include <Types/QMap.h>

namespace qcodec
{
    inline void encode_Arithmetic(const void* inBuf, const uint32_t inBufLength, void* outBuf, const uint32_t outBufLength)
    {
        assert((outBuf != nullptr && inBufLength != 0) && "Invalid out buffer provided");
        uint32_t curInIdx = 0;
        uint32_t curOutByteIdx = 0;
        uint8_t curOutBitIdx = 0;

        uint32_t frequencies[257] = {0};
        frequencies[0] = 1; //EOF
        buildFrequencyTableFromData(inBuf, inBufLength, frequencies);
        buildCumulativeFrequencies(frequencies);
        
        uint64_t range = ARITH_TOP;
        uint64_t low = 0;
        uint64_t high = low + range - 1;
        uint32_t bitCounter = 0;
        
        while(curInIdx < inBufLength)
        {
            uint8_t val = (inBuf, inBufLength, curInIdx);
            if(val == '\0')
            {
                std::cout << "Encounted EOF\n";
            }

            //Scaled low and high
            range = high - low + 1;
            low = low + (range * CUMULATIVE_FREQ[val]) / MAX_CUMULATIVE_FREQ;
            high = low + ((range * CUMULATIVE_FREQ[val + 1]) / MAX_CUMULATIVE_FREQ) - 1;

            uint8_t bitVal = 0;
            bool writeBit = false;
            if(high < ARITH_HALF)
            {
                // Low and high are to the left of half
                // MSB of low and high will be 0 and not change anymore
                writeBit = true; bitVal = 0;
            }
            else if(low >= ARITH_HALF)
            {
                // Low and high are to the right of half
                // MSB of low and high will be 1 and never change
                writeBit = true; bitVal = 1;
                low -= ARITH_HALF;
                high -= ARITH_HALF;
            }
            else if((high - low) <= ARITH_QUARTER)
            {
                /* Might lead to an underflow. Shift and increase range
                // In this case, low and high are on opp sides of HALF and we will have a bit sequence something similar to this. 
                //  |     |        ||        |     |
                //  |---0.01------0.1------0.11----|
                //  |     |        ||        |     |
                //          <------here---->
                // Out bit sequence will be like 0.01xxxxxx and 0.10yyyyyy
                // Cuz (high-low) is less that quarter
                // So, our goal is to shift the values to 0.0xxxxxx and 0.1yyyyyy (Eliminating 2nd MSB)
                // 1. Sub quarter (0.01) will get us: 0.00xxxxxx and 0.01yyyyy
                // 2. left shift will get us: 0.0xxxxxx and 0.1yyyyyy
                // As we are basically eliminating one bit, we need to keep track of it
                // If we can finally add a bit, we also follow it up with bits we are keeping track in counter. It can mean one of the following:
                // If we decide to add 1, it means we are above half and whatever bits we've removed so far are 0.
                // If we decide to remove 1, it means we are above half and whatever bits we've removed so far are 1.
                // This is because, before shifting out, the value can fall to the right or left of 0.1.
                // If it falls to the right, we will have 2nd MSB as 0 and if it's on left, 2nd MSB is 1.
                // These are the bits we are keeping track of.
                // The actual value(before any shifts) can only fall on either side of 0.1. 
                 Shifting range is just for our convenience and we still need to account for the "removed" bits.
                 The same logic transfers perfectly to decimal values too 
                */

                writeBit = false;
                low -= ARITH_HALF;
                high -= ARITH_HALF;
                ++bitCounter;
            }
            else
            {
                continue;
            }

            if(writeBit)
            {
                writeBitAndAdvance(outBuf, outBufLength, bitVal, curOutByteIdx, curOutBitIdx);
                //Write any bytes that we removed
                for(uint64_t i = 0; i < bitCounter; ++i)
                {
                    writeBitAndAdvance(outBuf, outBufLength, !bitVal, curOutByteIdx, curOutBitIdx);
                    --bitCounter;
                }
            }
            low <<= 1;
            high <<= 1;
        }

        /*Any leftover conflicted bits*/
        if(bitCounter != 0)
        {
            writeBitAndAdvance(outBuf, outBufLength, 0, curOutByteIdx, curOutBitIdx);
                //Write any bytes that we removed
                for(uint64_t i = 0; i < bitCounter; ++i)
                {
                    writeBitAndAdvance(outBuf, outBufLength, 1, curOutByteIdx, curOutBitIdx);
                    --bitCounter;
                }
        }
    }

    /*Do NOT call directly*/
    inline void _initArithmeticEncoder()
    {

    }
    
    inline void decode_Arithmetic(const void* inBuf, void* outBuf)
    {

    }

    inline void encode_ArithmeticBinary(const void* inBuf, const uint32_t length, void* outBuf)
    {

    }

    void decode_ArithmeticBinary(const void* inBuf, void* outBuf)
    {

    }

}