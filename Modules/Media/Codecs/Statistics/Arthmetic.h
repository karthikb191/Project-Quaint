#include "../ParsingHelpers.h"
#include <Types/QMap.h>


namespace qcodec
{

    inline void encode_Arithmetic(const void* inBuf, const uint32_t length, void* outBuf)
    {
        uint32_t curInIdx = 0;
        uint32_t curOutIdx = 0;

        uint32_t frequencies[257] = {0};
        
        /*Cumulative Frequencies. 0the symbol is EOF for now*/
        double cp[257] = {0};
        buildFrequencyTableFromData(inBuf, length, frequencies);
        double entropy = buildCumulativeProbabilitesFromFrequencies(frequencies, length, cp);
        
        double left = 0.; /*Left Range*/
        double range = 1.; /*Right Range*/

        while(curInIdx < length)
        {
            
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