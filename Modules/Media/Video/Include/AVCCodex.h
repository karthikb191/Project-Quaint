#ifndef _H_AVC_CODEX
#define _H_AVC_CODEX

#include <BMFHelpers.h>
#include <fstream>
#include <Interface/IMemoryContext.h>
#include <BMFStructures_Common.h>
#include <iostream>

namespace Quaint { namespace Media
{
    struct NALUnit
    {
        NALUnit(int8_t numBytes)
        : m_numBytesInNALUnit(numBytes)
        {}

        uint8_t getNALRefIDC() { return m_flags & 0b01100000; }
        uint8_t getNALUnitType() { return m_flags & 0b00011111; }

        virtual void parse(std::fstream& handle, uint64_t& bytesRead)
        {
            char buf[4] = {'\0'};
            BMF_READ_VAR(buf, 1, handle, BMF_CHAR_TO_UINT8, m_flags);

            uint8_t nalID = getNALRefIDC();
            uint8_t type = getNALUnitType();
            type = getNALUnitType();
        }
        void dump(const char* byteBuffer, uint16_t length)
        {
            std::cout << "\n";
            for(int i = 0; i < length; i++)
            {
                uint32_t v = (uint8_t)byteBuffer[i];
                std::cout << std::hex << v << " ";
            }
            std::cout << "\n";
        }

        uint8_t     m_numBytesInNALUnit;
        uint8_t     m_flags;
    };

    struct SequenceParameterSetNALUnit : public NALUnit
    {
        SequenceParameterSetNALUnit()
        : NALUnit(0)
        {}
        SequenceParameterSetNALUnit(int8_t nalUnitBytes)
        : NALUnit(nalUnitBytes)
        {}

        virtual void parse(std::fstream& handle, uint64_t& bytesRead) override
        {
            NALUnit::parse(handle, bytesRead);
            
        }
    };

    struct AVCDecoderConfigurationRecord
    {
        AVCDecoderConfigurationRecord(IMemoryContext* context)
        : m_sequenceParamSets(context)
        {}

        uint8_t                                 m_version;
        uint8_t                                 m_avcProfileIndication;
        uint8_t                                 m_profileCompatibility;
        uint8_t                                 m_avcLevelIndication;
        uint8_t                                 m_nalUnitLength; /*Modifying this to actually hold the correct value of NAL unit when parsing. Actual val is 2 bits*/
        uint8_t                                 m_numSequenceParamSets; /*Modifying this to actually hold the correct value of NAL unit when parsing. Actual val is 5 bits*/
        QArray<SequenceParameterSetNALUnit>     m_sequenceParamSets;
        uint8_t                                 m_numPictureParamSets;

    };

    struct AVCConfigurationBox : public Box
    {
        AVCConfigurationBox(IMemoryContext* context)
        : m_decoderRecord(context)
        {}
        AVCDecoderConfigurationRecord       m_decoderRecord;
    };


}}

#endif //_H_AVC_CODEX