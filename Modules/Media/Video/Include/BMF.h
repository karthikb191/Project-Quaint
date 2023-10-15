#ifndef _H_BMF_H
#define _H_BMF_H

#include <Types/QStaticString.h>
#include <BMFStructures.h>
#include <Types/QArray.h>
#include <fstream>

namespace Quaint {namespace Media{
    template<typename T>
    struct BoxParseRes
    {
        T           box;
        uint64_t    bytesParsed = 0;
    };

    class BMF
    {
    public:
        BMF();
        BMF(const QPath& path) : BMF()
        {
            m_path = path;
            open();
        }
        ~BMF()
        {
            if(isOpen())
            {
                close();
            }
        }

        bool open();
        bool close();
        bool isOpen() { return m_handle.is_open();}

        void parse();

    private:

        void startParsing();
        uint64_t parseBox(Box& box);
        uint64_t parseFullBox(FullBox& fullBox, uint64_t bytesRead);

        void parseFileTypeBox(FileTypeBox& ftypBox, uint64_t bytesRead);
        void parseMediaDataBox(MediaDataBox& mediaDataBox, uint64_t bytesRead);
        void parseMovieBox(MovieBox& movieBox, uint64_t bytesRead);
        void parseMovieHeader(MovieBox::MovieHeader& header, uint64_t bytesRead);

        void parseTrackBox(TrackBox& trackBox, uint64_t bytesRead);
        void parseTrackHeaderBox(TrackBox::TrackHeaderBox& header, uint64_t bytesRead);
        void parseEditBox(TrackBox::EditBox& edit, uint64_t bytesRead);
        void parseEditListBox(TrackBox::EditBox::EditListBox& editList, uint64_t bytesRead);

        void parseMediaBox(TrackBox::MediaBox& mediaBox, uint64_t bytesRead);
        void parseMediaHeaderBox(TrackBox::MediaBox::MediaHeaderBox& headerBox, uint64_t bytesRead);
        void parseMediaHandlerReferenceBox(HandlerReferenceBox& handlerBox, uint64_t bytesRead);
        void parseVideoMediaInformationBox(MediaInformationBox& videoMinf, uint64_t bytesRead);
        void parseVideoMediaInformationHeaderBox(MediaInformationBox::VideoMediaInformationHeaderBox& vMinfHeader, uint64_t bytesRead);
        void parseDataInformationBox(DataInformationBox& dataInformation, uint64_t bytesRead);
        void parseSampleTableBox(SampleTableBox& sampleTable, uint64_t bytesRead);

        QPath               m_path;
        std::fstream        m_handle;
        
        FileTypeBox         m_fileTypeBox;
        MediaDataBox        m_mediDataBox; //TODO: Handle this. There can be multiple of these boxes
        MovieBox            m_movieBox;
    };
}}

#endif //_H_BMF_H