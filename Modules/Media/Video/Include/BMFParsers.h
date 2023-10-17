#ifndef _H_BMF_PARSER
#define _H_BMF_PARSER

#include <fstream>

class Quaint::IMemoryContext;

namespace Quaint { namespace Media{

class BitParser
{
public:
	BitParser(IMemoryContext* context, uint32_t length);
	BitParser(IMemoryContext* context, uint8_t* bitBuffer, uint32_t length);
	~BitParser();

	void populareBufferFromHandle(fstream& handle);
	uint32_t readBits(uint8_t n);
	uint32_t readBits_exp(uint8_t n);

private:
	uint32_t                m_length = 0;
	IMemoryContext*         m_context = nullptr;
	uint8_t*                m_buffer = nullptr;
	uint64_t                m_bitPos = 0;
	const uint64_t			m_numBits = 0;
};

inline uint32_t parseExpGolombCode(uint32_t val)
{
	int leadingZeroBits = -1;
	for(uint32_t b = 0; !b; leadingZeroBits++ )
	{       
			b = val & 0x80000000;
			val <<= 1;
	}
	uint32_t suffixRes = val >> (32 - leadingZeroBits);
	return (uint32_t)(pow(2.f, (float)leadingZeroBits)) - 1 + suffixRes;
}
    
}}

#endif //_H_BMF_PARSER