#include <H264/CabacDecoder.h>
#include <assert.h>

namespace Quaint { namespace Media{


/*  Context variable initialization process:
    Variables m, n are used in initialization of context variables. They are provided by tables 9-12 to 9-33
    These initialized context variables are assigned to all syntax elements in clauses 7.3.4 and 7.3.5. except end-of-slice flag

    For each context variable, pStateIdx and valMPS are initialized.
    pStateIdx corresponds to probability state index and valMPS corresponds to value of most probable symbol (the actual result we want)
*/
void CabacDecoderEngine::initContextVariables(uint8_t eSliceType, uint8_t uiCabac_init_idc)
{
    assert(uiCabac_init_idc < 4 && "Invalid cabac_init_idc");
    //For P, SP, B slices, initialization depends on cabac_init_idc SE

    //Engine has the latest valid ctx initiazation values for the respective slice and cabac_init_idc
    //TODO: Add logs
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
    // According to specification: preCtxState = Clip3( 1, 126, ( ( m * Clip3( 0, 51, SliceQPY ) ) >> 4 ) + n )
    // Negative values of SliceQPY are clipped to 0.
    // Therefore, we cache all possible values 0 - 51.
    // For each context variable, pStateIdx and valMPS are initialized.
    // The elements in m_ctx corresponds to values of ctxIdx 
    for(int sliceQp = 0; sliceQp < 52; ++sliceQp)
    {
        for(int ctxIdx = 0; ctxIdx < 1024; ctxIdx++)
        {
            int8_t m = cabacContextTable[ctxIdx][0];
            int8_t n = cabacContextTable[ctxIdx][1];

            uint32_t preCtxState = CLIP3(1, 126, ((m * sliceQp) >> 4) + n );
            if(preCtxState <= 63)
            {
                m_ctx[sliceQp][ctxIdx].stateIdx = 63 - preCtxState;
                m_ctx[sliceQp][ctxIdx].valMPS = 0;
            }
            else
            {
                m_ctx[sliceQp][ctxIdx].stateIdx = preCtxState - 64;
                m_ctx[sliceQp][ctxIdx].valMPS = 1;
            }
        }
    }
}

/*  This process is invoked before decoding the first macroblock of a slice 
    or after the decoding of any pcm_alignment_zero_bit and all pcm_sample_luma and pcm_sample_chroma data for a macroblock of type I_PCM
    Outputs: Initialized CodIRange nad CodIOffset variables. Status of arithmetic decoding engine is represented by these two variables
*/
void CabacDecoderEngine::initDecodeEngine(BitParser& parser)
{
    m_uiCodIRange = 510; /* Entire arithmetic range for decoding process */
    m_uiCodIOffset = parser.readBits(9); /* Offset into the arithmetic range */

    assert((m_uiCodIOffset != 510 && m_uiCodIOffset != 511) && "Invalid Offset retrieved");
    

}

uint8_t CabacDecoderEngine::decodeBin(BitParser& parser, uint16_t ctxIdx, bool bypass)
{
    assert(ctxIdx < 1024 && "invalid ctxIdx passed");

    if(bypass)
    {
        return decodeBypass(parser, ctxIdx);
    }

    if(ctxIdx == 276)
    {
        return decodeTerminate(parser, ctxIdx);
    }

    //Decode Decision
    uint8_t binVal = 0;
    uint16_t stateIdx = m_ctx[ctxIdx]->stateIdx;
    uint8_t qCodIRangeIdx = (m_uiCodIRange >> 6) & 3;
    uint16_t codIRangeLPS = C_range_table_lps[stateIdx][qCodIRangeIdx];

    m_uiCodIRange -= codIRangeLPS;

    //If Offset is more than Range(which now represents range of MPS), it must mean the symbol LPS.
    // Since we only have 2 values, the possible value is ~valMPS 
    if(m_uiCodIOffset >= m_uiCodIRange)
    {
        binVal = 1 - m_ctx[ctxIdx]->valMPS;
        m_uiCodIOffset -= m_uiCodIRange;
        m_uiCodIRange = codIRangeLPS;
    }
    else
    {
        binVal = m_ctx[ctxIdx]->valMPS;
    }

    //State Transition
    if(binVal == m_ctx[ctxIdx]->valMPS)
    {
        m_ctx[ctxIdx]->stateIdx = C_state_transition_table[m_ctx[ctxIdx]->stateIdx][1];
    }
    else
    {
        if(m_ctx[ctxIdx]->stateIdx == 0) m_ctx[ctxIdx]->valMPS = 1 - m_ctx[ctxIdx]->valMPS;
        m_ctx[ctxIdx]->stateIdx = C_state_transition_table[m_ctx[ctxIdx]->stateIdx][0];
    }

    //Renormalization
    renormalize(parser);

    return binVal;
}

uint8_t CabacDecoderEngine::decodeBypass(BitParser& parser, uint16_t ctxIdx)
{
    uint8_t binVal = 0;
    m_uiCodIOffset <<= 1;
    m_uiCodIOffset |= parser.readBits(1);
    if(m_uiCodIOffset >= m_uiCodIRange)
    {
        binVal = 1;
        m_uiCodIOffset -= m_uiCodIRange;
    }
    return binVal;
}
uint8_t CabacDecoderEngine::decodeTerminate(BitParser& parser, uint16_t ctxIdx)
{
    uint8_t binVal = 1;

    m_uiCodIOffset -= 2;
    if(m_uiCodIOffset < m_uiCodIRange)
    {
        binVal = 0;
        renormalize(parser);
    }
    return binVal;
}

void CabacDecoderEngine::renormalize(BitParser& parser)
{
    uint8_t failCounter = 0;
    while(m_uiCodIRange < 256)
    {
        assert(failCounter < 250 && "Unnatural Ranges encountered while trying to normalize");
        m_uiCodIRange <<= 1;
        m_uiCodIOffset <<= 1;
        m_uiCodIOffset |= parser.readBits(1);

        ++failCounter;
    }
}

/*Types of Binarization Supported:
1. Unary Binarization
2. Truncated Unary Binarization
3. Concatenated Unary / Kth Order EXP-Golomb
4. Fixed Length (FL) Binarization

*/

//Notes:
/*  When starting parsing of slice data of slice in clause 7.3.4, the initialization of CABAC process is invoked
    For each requested value of syntax element, a binarization scheme is derived as described in 9.3.2
    Binarization for the syntax element and the sequence of parsed bins determines the decoding process flow as described in clause 9.3.3

    For EACH bin of binarization of syntac element(binIdx), a context Index (ctxIdx) is derived as per 9.3.3.2 *****
    For each ctxIdx, the arithmetic decoding process is invoked as per 9.3.3.2
    The resulting sequence of bin string is compared to any pre-defined strings given by documentation.


    If decode request is processed for syntax element "mb_type" and decoded value of mb_type is I_PCM, the decoding engine is re-initialized
    after decoding any pcm_alignment_zero_bit and all pcm_sample_luma and pcm_sample_chroma data sets.
*/



}}