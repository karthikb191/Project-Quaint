#include <BMF.h>
#include <VideoModule.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <QuaintLogger.h>
#include <iostream>

namespace Quaint {namespace Media{
    DECLARE_LOG_CATEGORY(BMF);
    DEFINE_LOG_CATEGORY(BMF);

    /*Forward Declares*/
    void parseTillEndOfBox(const Box& box, const uint64_t bytesRead, char** buffer, std::fstream& handle);
    
    bool BMF::open()
    {
        m_handle = std::fstream(m_path.getBuffer(), std::ios::in | std::ios::binary);
        return m_handle.is_open();
    }
    bool BMF::close()
    {
        m_handle.close();
        return true;
    }

    void BMF::parse()
    {
        QLOG_I(BMF, "Started Parsing");

        m_handle.seekg(std::ios::beg);
        startParsing();

        QLOG_I(BMF, "Finished Parsing");
    }

    void BMF::startParsing()
    {
        while(m_handle.peek() != EOF)
        {
            parseBox();
        }
    }

    void BMF::parseBox()
    {
        //Get the size and type of box and move to a more specific parsing function
        uint64_t bytesRead = 0;
        char buf[8];
        m_handle.read(buf, 4);
        uint32_t size = BMF_CHAR_TO_UINT32(buf);
        std::cout << std::hex << size;
        m_handle.read(buf, 4);
        bytesRead += 8;

        Box box(size, buf);

        //Read Extended size
        if(size == 1)
        {
            m_handle.read(buf, 8);
            uint64_t extSize = BMF_CHAR_TO_UINT64(buf);
            bytesRead += 8;
        }

        switch (box.m_hdr.m_ty)
        {
        case BMF_BOX_ftyp:
            parseFileTypeBox(box, bytesRead);
            break;
        case BMF_BOX_mdat:
            parseMediaDataBox(box, bytesRead);
            break;
        case BMF_BOX_moov:
            parseMovieBox(box, bytesRead);
            break;
        default:
            break;
        }

    }

    void BMF::parseFileTypeBox(const Box& box, uint64_t bytesRead)
    {
        char buf[8];
        m_fileTypeBox = FileTypeBox(box);
        m_handle.read(buf, 4);
        m_fileTypeBox.m_majorBrand = BMF_CHAR_TO_UINT32(buf);
        m_handle.read(buf, 4);
        m_fileTypeBox.m_minor_version = BMF_CHAR_TO_UINT32(buf);

        if(m_fileTypeBox.m_majorBrand == BMF_BRAND_QT)
        {
            QLOG_I(BMF, "Major Brand is 'qt'");
        }
        
        bytesRead += 8;

        char* resBuffer = nullptr;
        parseTillEndOfBox(box, bytesRead, &resBuffer, m_handle);
        if(resBuffer != nullptr)
        {
            m_fileTypeBox.m_compatibleBrands = (uint32_t*)resBuffer;
        }

    }

    void BMF::parseMediaDataBox(const Box& box, uint64_t bytesRead)
    {
        m_mediDataBox = MediaDataBox(box);
        
        //Skipping media box for now
        if(box.m_hdr.m_sz != 1)
        {
            m_handle.seekg(box.m_hdr.m_sz - bytesRead, std::ios::cur);
        }
        else
        {
            m_handle.seekg(box.m_hdr.m_lSz - bytesRead, std::ios::cur);
        }
    }

    void BMF::parseMovieBox(const Box& box, uint64_t bytesRead)
    {

    }
    

    /*Helpers*/
    void parseTillEndOfBox(const Box& box, const uint64_t bytesRead, char** buffer, std::fstream& handle)
    {        
        uint64_t remaining = 0;
        if(box.m_hdr.m_sz == 1)
        {
            remaining = box.m_hdr.m_lSz - bytesRead;
        }
        else
        {
            remaining = box.m_hdr.m_sz - bytesRead;
        }

        if(remaining != 0)
        {
            *buffer = (char*)QUAINT_ALLOC_MEMORY(VideoModule::get().getVideoMemoryContext(), (size_t)remaining);
            handle.read(*buffer, remaining);
        }
    }
}}