
#include <assert.h>
#include <MemoryModule.h>
#include <Interface/IMemoryContext.h>
#include <BMFParsers.h>
#include <VideoModule.h>

namespace Quaint{ namespace Media {

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

    void BitParser::populareBufferFromHandle(fstream& handle)
    {
        assert(handle.is_open() && !handle.bad() && "Handled passed to bit parser is in invalid state");
        handle.read((char*)m_buffer, m_length);
    }

    //TODO: Add unit tests for this
    uint32_t BitParser::readBits(uint8_t n)
    {        
        if(n == 0) return 0;
        assert(m_bitPos + n <= m_length * 8 && "Trying to parse beyond valid bit buffer");

        uint32_t res = 0;

        while(n != 0)
        {
            uint16_t currentByte = (uint16_t)(m_bitPos >> 3);
            uint8_t mask = getMaskForBit(m_bitPos % 8);
            res <<= 1;
            res += (m_buffer[currentByte] & mask) ? 1 : 0;
            --n; ++m_bitPos;
        }
        return res;
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