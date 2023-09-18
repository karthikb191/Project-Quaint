#ifndef _H_BMF_STRUCTURES
#define _H_BMF_STRUCTURES

#include <BMFHelpers.h>

namespace Quaint { namespace Media
{
    constexpr uint32_t UUID = 'u' << 24 | 'u' << 16 | 'i' << 8 | 'd';
    
    struct alignas(8) BoxHeader
    {
        BoxHeader(){}
        BoxHeader(uint32_t pSz, char pTy[4])
        {
            m_sz = pSz;
            m_ty = BMF_CHAR_TO_UINT32(pTy);   
        }

        uint32_t m_sz       = 0;
        uint32_t m_ty       = 0;
        uint64_t m_lSz      = 0;
        uint8_t m_uTy[16]   = {0};
    };
    struct alignas(8) FullBoxHeader
    {
        union
        {
            uint32_t m_dat;
            struct
            {
                uint8_t         m_hdr : 8;
                uint32_t        m_flgs : 24;
            };
        };
    };

    struct alignas(8) Box
    {
        Box() {}
        Box(const uint32_t pSz, char pTyp[4]) : m_hdr(pSz, pTyp){}

        BoxHeader       m_hdr;
    };
    struct alignas(8) FullBox : public Box
    {
        FullBoxHeader   m_fHdr;
    };


    /*Specific Box Structures*/

    struct FileTypeBox : public Box
    {
        FileTypeBox(){}
        FileTypeBox(const uint32_t pSz) : Box(pSz, "ftyp"){}
        FileTypeBox(const Box& pBox) : Box(pBox){}
        uint32_t    m_majorBrand;
        uint32_t    m_minor_version;
        uint32_t*   m_compatibleBrands = nullptr; //Extends to the end of Box
    };

}}

#endif //_H_BMF_STRUCTURES