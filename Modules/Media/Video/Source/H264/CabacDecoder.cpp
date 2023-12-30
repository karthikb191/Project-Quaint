#include <H264/CabacDecoder.h>
#include <assert.h>

namespace Quaint { namespace Media{

void CabacDecoderEngine::initContextVariables(uint8_t eSliceType, uint8_t uiCabac_init_idc)
{
    assert(uiCabac_init_idc < 4 && "Invalid cabac_init_idc");
    //For P, SP, B slices, initialization depends on cabac_init_idc SE

    //Engine has the latest valid ctx initiazation values for the respective slice and cabac_init_idc
    if(eSliceType == ESliceType::E_I && m_eCtxInitState == EContextInitState::E_CABAC_INIT_I_SLICE) return;
    if(eSliceType != ESliceType::E_I && m_eCtxInitState == (uiCabac_init_idc + 1)) return;
    
    const int8_t (*cabacContextTable)[2] = nullptr;
    if(eSliceType == ESliceType::E_I)
    {
        cabacContextTable = C_cabac_context_init_I;
        m_eCtxInitState = E_CABAC_INIT_I_SLICE;
    }
    else
    {
        cabacContextTable = C_cabac_context_init_PB[uiCabac_init_idc];
        switch (uiCabac_init_idc)
        {
        case 0: m_eCtxInitState = E_CABAC_INIT_IDC_0;
            break;
        case 1: m_eCtxInitState = E_CABAC_INIT_IDC_1;
            break;
        case 2: m_eCtxInitState = E_CABAC_INIT_IDC_2;
            break;
        default: m_eCtxInitState = E_CABAC_INIT_INVALID;
            break;
        }
    }

    assert(m_eCtxInitState != E_CABAC_INIT_INVALID && "Invalid cabac_init_idc passed");

    //Get m and n
    for(int sliceQp = 0; sliceQp < 52; ++sliceQp)
    {
        for(int ctxIdx = 0; ctxIdx < 1024; ctxIdx++)
        {
            int8_t m = cabacContextTable[ctxIdx][0];
            int8_t n = cabacContextTable[ctxIdx][1];

            uint32_t preCtxState = CLIP3(1, 126, ((m * sliceQp) >> 4) + n );
            m_ctx[sliceQp][ctxIdx].stateIdx = 0;
            m_ctx[sliceQp][ctxIdx].valMPS = 0;
        }
    }
}
void CabacDecoderEngine::initDecodeEngine()
{

}

}}