#include <BMF.h>
#include <VideoModule.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <QuaintLogger.h>
#include <iostream>

namespace Quaint {namespace Media{
    DECLARE_LOG_CATEGORY(BMF);
    DEFINE_LOG_CATEGORY(BMF);

    /*Forward Declares*/
    void parseTillEndOfBox(const Box& box, const uint64_t bytesRead, char** buffer, std::fstream& handle, bool skip = false);
    
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
            BoxParseRes<Box> res = parseBox();
            Box& box = res.box;
            uint64_t bytesRead = res.bytesParsed;
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
    }

    BoxParseRes<Box> BMF::parseBox()
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
            box.m_hdr.m_lSz = extSize;
            bytesRead += 8;
        }
        return {box, bytesRead};
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

    /*This is the most important box that contains all the info needed to sample videos*/
    void BMF::parseMovieBox(const Box& box, uint64_t bytesRead)
    {
        assert(box.m_hdr.m_sz > 1 && "Extended sizes must be handled");
        
        while(bytesRead < box.m_hdr.m_sz)
        {
            BoxParseRes<Box> res = parseBox();
            Box& box = res.box;
            switch (box.m_hdr.m_ty)
            {
            case BMF_BOX_mvhd:
                m_movieBox.m_movieHeader = parseMovieHeader(box, res.bytesParsed);
                break;
            
            default:
                //Box of unknown type encountered. Skip it.
                QLOG_W(BMF, "Encountered box of unkonwn type when parsing Movie box. Skipping it!")
                parseTillEndOfBox(box, res.bytesParsed, nullptr, m_handle, true);
                bytesRead += box.m_hdr.m_sz;
                break;
            }
        }
    }

    BoxParseRes<FullBox> BMF::parseFullBox(const Box& box)
    {
        BoxParseRes<FullBox> res;
        res.box = Box(box);
        char buf[4] = "\0";

        BMF_READ(buf, 1, m_handle, BMF_CHAR_TO_UINT8, res.box.m_fHdr.m_ver);
        BMF_READ(buf, 3, m_handle, BMF_CHAR_TO_UINT32, res.box.m_fHdr.m_flgs);
        res.bytesParsed += 4;

        return res;
    }

    MovieHeader BMF::parseMovieHeader(const Box& box, uint64_t bytesRead)
    {
        MovieHeader header;
        
        BoxParseRes<FullBox> res = parseFullBox(box);
        bytesRead += res.bytesParsed;
        char buf[36] = "\0";

        BMF_READ(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_creationTime);
        BMF_READ(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_modificationTime);
        BMF_READ(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_timeScale);
        BMF_READ(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_duration);
        BMF_READ_FP32(buf, 4, m_handle, header.m_preferredRate, 16, 16);
        BMF_READ_FP16(buf, 2, m_handle, header.m_preferredVolume, 8, 8);
        
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col0.x, 16, 16);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col1.x, 16, 16);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col2.x, 2, 30);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col0.y, 16, 16);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col1.y, 16, 16);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col2.y, 2, 30);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col0.z, 16, 16);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col1.z, 16, 16);
        BMF_READ_FP32(buf, 4, m_handle, header.m_matStructure.col2.z, 2, 30);

        

        return header;
    }
    

    /*Helpers*/
    void parseTillEndOfBox(const Box& box, const uint64_t bytesRead, char** buffer, std::fstream& handle, bool skip)
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

        if(remaining != 0 && !skip)
        {
            *buffer = (char*)QUAINT_ALLOC_MEMORY(VideoModule::get().getVideoMemoryContext(), (size_t)remaining);
            handle.read(*buffer, remaining);
        }
    }
}}