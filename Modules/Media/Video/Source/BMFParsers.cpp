
#include <assert.h>
#include <MemoryModule.h>
#include <Interface/IMemoryContext.h>
#include <BMFParsers.h>
#include <VideoModule.h>
#include <QuaintLogger.h>


namespace Quaint{ namespace Media {
    
    DECLARE_LOG_CATEGORY(BMFPARSER);
    DEFINE_LOG_CATEGORY(BMFPARSER);

    inline uint8_t getMaskForBit(uint8_t bit)
    {
        return 1 << (7 - bit); 
    }

    BitParser::BitParser(IMemoryContext* context, uint32_t length)
    : m_context(context)
    , m_length(length)
    , m_numBits(length * 8)
    {
        assert(m_context != nullptr && "Invalid Memory Context passe");
            
        m_buffer = (uint8_t*)QUAINT_ALLOC_MEMORY(m_context, m_length);
    }
    BitParser::BitParser(IMemoryContext* context, uint8_t* bitBuffer, uint32_t length)
    : m_context(context)
    , m_length(length)
    , m_numBits(length * 8)
    {
        assert(m_context != nullptr && "Invalid Memory Context passe");
            
        m_buffer = (uint8_t*)QUAINT_ALLOC_MEMORY(m_context, m_length);
        memcpy(m_buffer, bitBuffer, length);
    }
    BitParser::~BitParser()
    {
        if(m_buffer != nullptr)
        {
            QUAINT_DEALLOC_MEMORY(m_context, m_buffer);
        }
    }

    void BitParser::populateBufferFromHandle(std::fstream& handle)
    {
        assert(handle.is_open() && !handle.bad() && "Handled passed to bit parser is in invalid state");
        handle.read((char*)m_buffer, m_length);
    }

    uint32_t BitParser::nextBits(uint8_t n)
    {
        if(n == 0) return 0;
        assert(m_bitPos + n <= m_length * 8 && "Trying to read beyond valid bit buffer");
        
        uint32_t res = 0;
        uint64_t currentPos = m_bitPos;
        
        while(n != 0)
        {
            uint16_t currentByte = (uint32_t)(currentPos >> 3);
            res <<= 1;
            res += (getMaskForBit(currentPos % 8) & m_buffer[currentByte]) == 0 ? 0 : 1;
            --n; ++currentPos;
        }
        return res;
    }

    uint32_t BitParser::readBits(uint8_t n)
    {        
        if(n == 0) return 0;
        if(m_bitPos + n > m_numBits)
        {
            QLOG_W(BMFPARSER, "Bit Buffer Overflow!!!");
            m_overflow = true;
            return 0;
        }
        
        if(m_complete)
        {
            QLOG_W(BMFPARSER, "Bit parser reached the end of stream. Cannot continue further");
            return 0;
        }

        uint32_t res = 0;

        while(n != 0)
        {
            uint32_t currentByte = (uint32_t)(m_bitPos >> 3);
            uint8_t mask = getMaskForBit(m_bitPos % 8);
            res <<= 1;
            res += (m_buffer[currentByte] & mask) ? 1 : 0;
            --n; ++m_bitPos;
        }

        if(m_bitPos == m_numBits)
        {
            m_complete = true;
        }

        return res;
    }
    
    void BitParser::alignToByte()
    {
        if(m_bitPos % 8 == 0) return;
        
        m_bitPos += 8 - (m_bitPos % 8);
        
        if(m_bitPos == m_length * 8)
        {
            m_complete = true;
        }
    }
    
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
    
    uint32_t BitParser::ue()
    {
        int leadingZeroBits = -1;
        for(uint32_t b = 0; b == 0 && !m_overflow; ++leadingZeroBits)
        {
            b = readBits(1);
        }
        if(m_overflow)
        {
            return 0;
        }
        uint32_t suffixRes = readBits(leadingZeroBits);
        return uint32_t(pow(2, leadingZeroBits)) - 1 + suffixRes;
    }
	int32_t BitParser::se()
    {
        float ueInput = (float)ue();
        return (int32_t)(pow(-1, ueInput + 1)) * (int32_t)(ceil(ueInput/2.0f));
    }


    uint32_t BitParser::readBits_exp(uint8_t n)
    {
        if(n == 0) return 0;
        assert(m_bitPos + n <= m_length * 8 && "Trying to parse beyond valid bit buffer");

        uint32_t res = 0;

        uint8_t numRead = 0;
        uint16_t endByte = (uint16_t)((m_bitPos + n) >> 3);

        while(numRead < n)
        {
            uint16_t currentByte = (uint16_t)(m_bitPos >> 3);
            if(currentByte != endByte)
            {
                uint8_t numBitsLeft = 8 - (m_bitPos % 8);
                res <<= numBitsLeft; //Make room for bits coming in
                uint8_t mask = 0;
                while(numBitsLeft != 0)
                {
                    mask |= getMaskForBit(m_bitPos % 8);
                    ++m_bitPos;
                    ++numRead;
                    --numBitsLeft;
                }
                res |= m_buffer[currentByte] & mask;
            }
            else
            {
                uint8_t numBitsLeft = n - numRead;
                uint8_t mask = 0;
                res <<= numBitsLeft;
                while(numBitsLeft != 0)
                {
                    mask |= getMaskForBit(m_bitPos % 8);
                    ++m_bitPos;
                    ++numRead;
                    --numBitsLeft;
                }
                res |= (m_buffer[currentByte] & mask) >> (7 - ((m_bitPos - 1) % 8));
            }
        }

        return res;

    }

}}