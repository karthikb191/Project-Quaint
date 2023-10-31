#ifndef _H_BMF_STRUCTURES_COMMON
#define _H_BMF_STRUCTURES_COMMON

#include <type_traits>
#include <BMFHelpers.h>

namespace Quaint { namespace Media{
    constexpr uint32_t UUID = 'u' << 24 | 'u' << 16 | 'i' << 8 | 'd';

    
    struct NameCode
    {
        union
        {
            uint32_t    m_val;
            char        m_c[4]; /*This will appear in reverse in little endian machines. Dont panic*/
        };
    };

    struct alignas(8) BoxHeader
    {
        BoxHeader(){}
        BoxHeader(uint32_t pSz, char pTy[4])
        : m_sz(pSz)
        , m_ty(BMF_CHAR_TO_UINT32(pTy))
        , m_cTy{pTy[0],pTy[1],pTy[2],pTy[3]}
        {}

        uint32_t m_sz       = 0;
        uint32_t m_ty       = 0;
        uint64_t m_lSz      = 0;
        uint8_t m_uTy[16]   = {0};

        //For Debug
        char m_cTy[4];
    };
    struct alignas(8) FullBoxHeader
    {
        union
        {
            uint32_t m_dat;
            struct
            {
                uint8_t         m_ver : 8;
                union 
                {
                    uint32_t    uiFlags : 32;
                    char        cFlags[4];
                }m_flags;
            };
        };
    };

    struct alignas(8) Box
    {
        Box() {}
        Box(const uint32_t pSz, char pTyp[4]) : m_hdr(pSz, pTyp){}
        Box(const Box& box) = default;
        Box& operator=(const Box& other)
        {
            m_hdr = other.m_hdr;
            return *this;
        }
        void setBox(const Box& other)
        {
            m_hdr = other.m_hdr;
        }
        BoxHeader       m_hdr;
    };
    struct alignas(8) FullBox : public Box
    {
        FullBox() {}
        FullBox(const Box& box) : Box(box){}
        FullBoxHeader   m_fHdr;

        void setFullBox(const FullBox& other)
        {
            m_hdr = other.m_hdr;
            m_fHdr = other.m_fHdr;
        }
    };


}}

#endif //_H_BMF_STRUCTURES_COMMON