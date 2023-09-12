#ifndef _H_BMF_STRUCTURES
#define _H_BMF_STRUCTURES

namespace Quaint { namespace Media
{
    constexpr uint32_t UUID = 'u' << 24 | 'u' << 16 | 'i' << 8 | 'd';
    
    struct alignas(8) BoxHeader
    {
        uint32_t m_sz       = 0;
        uint32_t m_ty       = 0;
        uint64_t m_lSz      = 0;
        uint8_t m_uTy[16]   = {0};
    };

}}

#endif //_H_BMF_STRUCTURES