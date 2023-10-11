#ifndef _H_BMF_STRUCTURES
#define _H_BMF_STRUCTURES

#include <BMFHelpers.h>
#include <Math/QMat.h>
#include <Interface/IMemoryContext.h> 
#include <Types/QArray.h>
#include <Types/QFastArray.h>

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
                union 
                {
                    uint32_t    uiFlags : 24;
                    char        cFlags[4];
                }m_flags;
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
        QFastArray<uint32_t, 10> m_compatibleBrands; //Extends to the end of Box 
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


//------------- Track Related Boxes -------------------------------
    
    struct alignas(16) TrackBox : public Box
    {
        struct TrackProfile : public Box
        {
            //TODO
        };
        struct TrackHeaderBox : public FullBox
        {
            uint32_t            m_creationTime;
            uint32_t            m_modificationTime;
            uint32_t            m_trackId;
            char                m_reserved1[4];
            uint32_t            m_duration; /*Indicates duration of this track, timescale indicated in Movie Header Box*/
            char                m_reserved2[8];
            int16_t             m_layer;
            int16_t             m_alternateGroup;
            float               m_volume;   /*Should be parsed a 8.8 Fixed point*/
            char                m_reserved3[2];
            QMat3x3             m_matrix;
            float               m_width;    /*Should be parsed as 16.16 Fixed point*/
            float               m_height;   /*Should be parsed as 16.16 Fixed point*/

            bool isTrackEnabled()
            {
                return m_fHdr.m_flags.uiFlags & 0x000001;
            }
            /*The value 1 indicates that the track, or one of its alternatives (if any) forms a direct part of the presentation. 
            The value 0 indicates that the track does not represent a direct part of the presentation*/
            bool canTrackOrAnySubTrackPresentDirectly()
            {
                return m_fHdr.m_flags.uiFlags & 0x000002;
            }
            /*1 indicates width and height are not expressed in pixel units. 
            The values are only an indication of the desired aspect ratio*/
            bool trackSizeIsAspectRatio()
            {
                return m_fHdr.m_flags.uiFlags & 0x000008;
            }
        };
        struct TrackApertureModeDimensionsBox : public Box
        {
            //TODO
        };
        struct ClippingBox : public Box
        {

        };
        struct TrackMatteBox : public Box
        {

        };
        struct EditBox : public Box
        {
            EditBox(IMemoryContext* context)
            : m_editLists(context, context)
            {}
            struct EditListBox : public FullBox
            {
                EditListBox(IMemoryContext* context)
                : m_entries(context)
                {}
                EditListBox(const EditListBox& other) = default;
                EditListBox& operator=(const EditListBox& other) = default;

                struct Entry
                {
                    uint32_t    m_duration;
                    uint32_t    m_mediaTime;
                    float       m_mediaRate;
                };

                uint32_t        m_numEntries;
                QArray<Entry>   m_entries;
            };

            Quaint::QArray<EditListBox>     m_editLists;
        };
        struct TrackReferenceBox : public Box
        {

        };
        struct TrackExcludeFromAutoSelectionBox : public Box
        {

        };
        struct TrackLoadSettingsBox : public Box
        {

        };
        struct TrackInputMapBox : public Box
        {

        };
        struct MediaBox : public Box
        {

        };
        struct UserDefinedDataBox : public Box
        {

        };

        TrackBox(IMemoryContext* context)
        : m_edit(context)
        {}
        
        TrackProfile                        m_trackProfile;
        TrackHeaderBox                      m_trackHeader;
        TrackApertureModeDimensionsBox      m_apertureModeDims;
        ClippingBox                         m_clipping;
        TrackMatteBox                       m_trackMatte;
        EditBox                             m_edit;
        TrackReferenceBox                   m_trackRef;
        TrackExcludeFromAutoSelectionBox    m_trackExcludeFromAutoSelection;
        TrackLoadSettingsBox                m_trackLoadSettings;
        TrackInputMapBox                    m_trackInput;
        MediaBox                            m_media;
        UserDefinedDataBox                  m_usrData;
    };
//-----------------------------------------

    struct MovieBox : public Box
    {
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

        MovieBox(IMemoryContext* context)
        : m_context(context)
        , m_tracks(context, context)
        {}
        MovieHeader                                                 m_movieHeader;
        IMemoryContext*                                             m_context;
        Quaint::QArray<TrackBox>                   m_tracks;
    };
}}

#endif //_H_BMF_STRUCTURES