#ifndef _H_H264_CABAC_DECODER
#define _H_H264_CABAC_DECODER
#include <cstdint>
#include <H264/CabacCommon.h>
#include <BMFParsers.h>

namespace Quaint { namespace Media{
    enum ECabacError
    {
        E_Invalid = -1
    };

    enum ESliceType
    {
        E_I = 0,
        E_SI = 1,
        E_P = 2,
        E_SP = 3,
        E_B = 4
    };
    enum EContextInitState
    {
        E_CABAC_INIT_I_SLICE = 0,
        E_CABAC_INIT_IDC_0 = 1,
        E_CABAC_INIT_IDC_1 = 2,
        E_CABAC_INIT_IDC_2 = 3,
        E_CABAC_INIT_INVALID = 4
    };

    struct ContextVar
    {
        uint8_t     stateIdx;
        uint8_t     valMPS;
    }sCtxVar, *pCtxVar;
    
    class CabacDecoderEngine
    {

    public:
        /*Should be called for first SE in slice*/
        void initContextVariables(uint8_t eSliceType, uint8_t uiCabac_init_idc);
        
        /*
            Parser should point to the right location here.
            TODO: Instead of parser, pass in a blob of data that's stored in heap to work with
        */
        void initDecodeEngine(BitParser& parser);
        
        uint8_t decodeBin(uint16_t ctxIdx, bool bypass = false);
        uint8_t decodeBypass(uint16_t ctxIdx);
        uint8_t decodeTerminate(uint16_t ctxIdx);
        
        /*This is populated during initialization of context variables*/
        EContextInitState       m_eCtxInitState = EContextInitState::E_CABAC_INIT_INVALID;
        ContextVar              m_ctx[52][1024];

        uint16_t                m_uiCodIRange;
        uint16_t                m_uiCodIOffset;
    };

}}

#endif //_H_H264_CABAC_DECODER