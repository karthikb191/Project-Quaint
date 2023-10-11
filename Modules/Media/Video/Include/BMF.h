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
        BoxParseRes<Box> parseBox();
        BoxParseRes<FullBox> parseFullBox(const Box& box);

        void parseFileTypeBox(const Box& box, FileTypeBox& ftypBox, uint64_t bytesRead);
        void parseMediaDataBox(const Box& box, MediaDataBox& mediaDataBox, uint64_t bytesRead);
        void parseMovieBox(const Box& box, MovieBox& movieBox, uint64_t bytesRead);
        void parseMovieHeader(const Box& box, MovieBox::MovieHeader& header, uint64_t bytesRead);

        void parseTrackBox(const Box& box, TrackBox& trackBox, uint64_t bytesRead);
        void parseTrackHeaderBox(const Box& box, TrackBox::TrackHeaderBox& header, uint64_t bytesRead);
        void parseEditBox(const Box& box, TrackBox::EditBox& edit, uint64_t bytesRead);
        void parseEditListBox(const Box& box, TrackBox::EditBox::EditListBox& editList, uint64_t bytesRead);


        QPath               m_path;
        std::fstream        m_handle;
        
        FileTypeBox         m_fileTypeBox;
        MediaDataBox        m_mediDataBox; //TODO: Handle this. There can be multiple of these boxes
        MovieBox            m_movieBox;
    };
}}

#endif //_H_BMF_H