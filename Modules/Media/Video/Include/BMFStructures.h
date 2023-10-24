#ifndef _H_BMF_STRUCTURES
#define _H_BMF_STRUCTURES

#include <BMFHelpers.h>
#include <Math/QMat.h>
#include <Interface/IMemoryContext.h> 
#include <Types/QArray.h>
#include <Types/QFastArray.h>
#include <Types/QStaticString.h>
#include <fstream>
#include <BMFStructures_Common.h>

//TODO: Make this independent of codec.
#include <AVCCodex.h>

namespace Quaint { namespace Media
{
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

    /*Declares the process by which media data in the stream may be presented and thus, nature of media in the stream*/
    struct HandlerReferenceBox : public FullBox
    {
        //WARN! Some of these params may be specific to MOV format.
        NameCode            m_componentType;
        NameCode            m_componentSubType;
        uint32_t            m_manufacturer; /*Reserved. Set to 0*/
        uint32_t            m_flags; /*Reserved. Set to 0*/
        uint32_t            m_flagsMask; /*Reserved. Set to 0*/
        Quaint::QName       m_name;
    };

    struct DataReferenceEntry : public FullBox
    {
        void*   m_data;
    };
    
    struct DataReferenceBox : public FullBox
    {
        DataReferenceBox(IMemoryContext* context)
        : m_entries(context)
        {}
        uint32_t                            m_numEntries;
        Quaint::QArray<DataReferenceEntry>  m_entries;
    };

    struct DataInformationBox : public Box
    {
        DataInformationBox(IMemoryContext* context)
        : m_dataRef(context)
        {}
        DataReferenceBox    m_dataRef;
    };

    struct VideoSampleDescription
    {
        uint32_t            m_sampleDescriptionSize;
        NameCode            m_dataFormat;
        char                m_reserved1[6];
        uint16_t            m_dataRefIndex;
        uint16_t            m_version;
        uint16_t            m_revisionLevel;
        NameCode            m_vendor;
        uint32_t            m_temporalQuality;
        uint32_t            m_spatialQuality;
        uint16_t            m_width;
        uint16_t            m_height;
        float               m_horRes;
        float               m_vertRes;
        uint32_t            m_dataSize;
        uint16_t            m_frameCount;
        QName               m_compressorName;
        uint16_t            m_pixelDepth;
        uint16_t            m_colorTableId;
    };

    struct SampleTableBox : public Box
    {
        struct SampleDescriptionBox : public FullBox
        {
            SampleDescriptionBox(IMemoryContext* context)
            : m_samples(context)
            {}
            uint32_t                        m_numEntries;
            QArray<VideoSampleDescription>  m_samples;
        };
        
        SampleTableBox(IMemoryContext* context)
        : m_description(context)
        , m_avcConfig(context)
        {}

        SampleDescriptionBox    m_description;
        AVCConfigurationBox     m_avcConfig;
    };

    /*Contains other boxes that define specific characteristics of video media data*/
    struct MediaInformationBox : public Box
    {
        struct VideoMediaInformationHeaderBox : public FullBox
        {
            struct OpCode
            {
                uint16_t    m_r;
                uint16_t    m_g;
                uint16_t    m_b;
            };
            
            uint16_t        m_graphicsMode;
            OpCode          m_opCode;
        };

        MediaInformationBox(IMemoryContext* context)
        : m_dataInformation(context)
        , m_sampleTable(context)
        {}

        VideoMediaInformationHeaderBox      m_vMinfHeader;
        HandlerReferenceBox                 m_handler;
        DataInformationBox                  m_dataInformation;
        SampleTableBox                      m_sampleTable;
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
        /*Edit Box is used to define protions of media that are used to build tracks of movies.
          Edits themselves are contained in EditTable, which contains Offset and durration of each segment.
          In absense of Edit Box, presentation starts immediately.
          Empty Edit is used to offset the start time of track*/
        struct EditBox : public Box
        {
            EditBox(IMemoryContext* context)
            : m_editLists(context, context)
            {}
            EditBox(const EditBox&) = default;
            EditBox& operator=(const EditBox&) = default;

            /*EditList entries maps from time in movie to time in media, and ultimately to media data */
            struct EditListBox : public FullBox
            {
                EditListBox(IMemoryContext* context)
                : m_entries(context)
                {}
                EditListBox(const EditListBox& other) = default;
                EditListBox& operator=(const EditListBox& other) = default;

                struct Entry
                {
                    /*Specified duration of this edit segment in terms of movie's timeScale*/
                    uint32_t    m_duration;
                    /*Contains starting time within the media of this edit segment*/
                    uint32_t    m_mediaTime;
                    /*Relative rate at which to play the media corresponsding to the edit segment*/
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

    /* Media Boxes contain following information:
        - MediaType, such as Video, audio or timed metadata
        - Media handler component used to interpret sample data
        - Media timescale and track duration
        - Media and track specific information, such as volume and graphics mode
        - Media data references, which typically specify the files where sample data is stored
        - Sample table Boxes, which, for each media sample, specify sample description, duration and byte offset from data reference
    */
        struct MediaBox : public Box
        {
            /*Specifies characteristics of media, including timescale and duration*/
            struct MediaHeaderBox : public FullBox
            {
                uint32_t        m_creationTime;
                uint32_t        m_modificationTime;
                uint32_t        m_timeScale;    /*Coordinate space measured in 1/timeScale of second (or) No of time units that pass per second*/
                uint32_t        m_duration;
                int16_t         m_language;
                uint16_t        m_quality;  /*Predefined according to standard*/
            };
            
            struct UserDataBox : public Box
            {

            };

            MediaBox(IMemoryContext* context)
            : m_mediaInfo(context)
            {}

            MediaHeaderBox              m_movieHeader;
            HandlerReferenceBox         m_handler;
            MediaInformationBox         m_mediaInfo;
        };
        struct UserDefinedDataBox : public Box
        {

        };

        TrackBox(IMemoryContext* context)
        : m_edit(context)
        , m_media(context)
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

        uint32_t getTimeScale() { return m_movieHeader.m_timeScale; }
        uint32_t getDuration() { return m_movieHeader.m_duration; }
        float getDurationInSec() { return (float)m_movieHeader.m_duration/(float)m_movieHeader.m_timeScale; }

        MovieHeader                                                 m_movieHeader;
        IMemoryContext*                                             m_context;
        Quaint::QArray<TrackBox>                                    m_tracks;
    };

}}

#endif //_H_BMF_STRUCTURES