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

	char* getBuffer_NonConst() { return (char*)m_buffer; }
	bool isComplete() { return m_complete; }
	bool isOverflown() { return m_overflow; }

	void populateBufferFromHandle(std::fstream& handle);
	/*Reads next bits without incrementing pointer*/
	uint32_t nextBits(uint8_t n);
	uint8_t getBitVal(uint32_t n);
	uint32_t readBits(uint8_t n);
	uint32_t readBits_exp(uint8_t n);

	void alignToByte();
	bool moreRBSPDataExists();

	/* Exp-Golomb-code parse*/
	uint32_t ue();
	/* Kth Order Exp_Golomb Code(Unsigned)*/
	uint32_t ue_k(uint8_t k);
	int32_t se();

private:
	uint32_t                m_length = 0;
	IMemoryContext*         m_context = nullptr;
	uint8_t*                m_buffer = nullptr;
	uint64_t                m_bitPos = 0;
	const uint64_t			m_numBits = 0;
	bool					m_complete = false;
	bool					m_overflow = false;
};
    
}}

#endif //_H_BMF_PARSER