#ifndef _H_BMF_H
#define _H_BMF_H

#include <Types/QStaticString.h>
#include <BMFStructures.h>
#include <Types/QArray.h>
#include <fstream>

namespace Quaint {namespace Media{
    class BMF
    {
    public:
        BMF(){}
        BMF(const QPath& path)
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
        void parseBox();

        void parseFileTypeBox(const Box& box, uint64_t bytesRead);
        void parseMediaDataBox(const Box& box, uint64_t bytesRead);
        void parseMovieBox(const Box& box, uint64_t bytesRead);

        QPath               m_path;
        std::fstream        m_handle;
        
        FileTypeBox         m_fileTypeBox;
        MediaDataBox        m_mediDataBox; //TODO: Handle this. There can be multiple of these boxes
    };
}}

#endif //_H_BMF_H