#ifndef _H_H264_CABAC_DECODER
#define _H_H264_CABAC_DECODER
#include <cstdint>
#include <H264/CabacCommon.h>

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

    struct SyntaxElement
    {
        /*Input is the SE to be parsed and previously parsed SEs*/
        void parseSE(const SyntaxElement& se);

    };


    class CabacContext
    {
        //======== ???Parsing of elements are based on their contexts??? ============
    public:

        /*Self Contained functions*/
        void decodeBin();
        void decodeBypass();
        void decodeTerminate();
    };
    
    class CabacDecoderEngine
    {

    public:
        /*Should be called for first SE in slice*/
        void initContextVariables(uint8_t eSliceType, uint8_t uiCabac_init_idc);
        void initDecodeEngine();
        
        /*This is populated during initialization of context variables*/
        EContextInitState       m_eCtxInitState = EContextInitState::E_CABAC_INIT_INVALID;
        ContextVar              m_ctx[52][1024];

    };

}}

#endif //_H_H264_CABAC_DECODER