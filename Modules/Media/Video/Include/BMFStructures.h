#ifndef _H_BMF_STRUCTURES
#define _H_BMF_STRUCTURES

#include <BMFHelpers.h>
#include <Math/QMat.h>

namespace Quaint { namespace Media
{
    constexpr uint32_t UUID = 'u' << 24 | 'u' << 16 | 'i' << 8 | 'd';
    
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
        FullBox() {}
        FullBox(const Box& box) : Box(box){}
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

    struct MediaDataBox : public Box
    {
        MediaDataBox(){};
        MediaDataBox(const uint32_t pSz) : Box(pSz, "mdat"){};
        MediaDataBox(const Box& box) : Box(box){}        
    };

    //TODO
    struct ProfileBox : public Box
    {

    };

    /*Movie header is a leaf atom*/
    struct MovieHeader : public FullBox
    {
        uint32_t            m_creationTime;
        uint32_t            m_modificationTime;
        uint32_t            m_timeScale;
        uint32_t            m_duration;
        float               m_preferredRate;
        float               m_preferredVolume;
        char                m_reserved[10]; //10 Bytes reserved for Apple. This might change across file formats 
        QMat3x3             m_matStructure;
        uint32_t            m_previewTime;
        uint32_t            m_previewDuration;
        uint32_t            m_posterTime;
        uint32_t            m_selectionTime;
        uint32_t            m_selectionDuration;
        uint32_t            m_currentTime;
        uint32_t            m_nextTrackID;
    };

    struct TrackBox : public Box
    {

    };

    struct MovieBox : public Box
    {
        MovieBox(IMemoryContext* context)
        : m_context(context)
        , m_tracks(context)
        {}
        MovieHeader                     m_movieHeader;
        IMemoryContext*                 m_context;
        Quaint::QArray<TrackBox>        m_tracks;
    };
}}

#endif //_H_BMF_STRUCTURES