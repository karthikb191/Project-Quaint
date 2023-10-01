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
    
    BMF::BMF()
    : m_movieBox(VideoModule::get().getVideoMemoryContext())
    {
    }

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
                parseFileTypeBox(box, m_fileTypeBox, bytesRead);
                break;
            case BMF_BOX_mdat:
                parseMediaDataBox(box, m_mediDataBox, bytesRead);
                break;
            case BMF_BOX_moov:
                parseMovieBox(box, m_movieBox, bytesRead);
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

    void BMF::parseFileTypeBox(const Box& box, FileTypeBox& ftypBox, uint64_t bytesRead)
    {
        char buf[8];
        ftypBox = FileTypeBox(box);
        m_handle.read(buf, 4);
        ftypBox.m_majorBrand = BMF_CHAR_TO_UINT32(buf);
        m_handle.read(buf, 4);
        ftypBox.m_minor_version = BMF_CHAR_TO_UINT32(buf);

        if(ftypBox.m_majorBrand == BMF_BRAND_QT)
        {
            QLOG_I(BMF, "Major Brand is 'qt'");
        }
        
        bytesRead += 8;

        char* resBuffer = nullptr;
        parseTillEndOfBox(box, bytesRead, &resBuffer, m_handle);
        if(resBuffer != nullptr)
        {
            ftypBox.m_compatibleBrands = (uint32_t*)resBuffer;
        }

    }

    void BMF::parseMediaDataBox(const Box& box, MediaDataBox& mediaDataBox, uint64_t bytesRead)
    {
        mediaDataBox = MediaDataBox(box);
        
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
    void BMF::parseMovieBox(const Box& box, MovieBox& movieBox, uint64_t bytesRead)
    {
        assert(box.m_hdr.m_sz > 1 && "Extended sizes must be handled");
        
        while(bytesRead < box.m_hdr.m_sz)
        {
            BoxParseRes<Box> res = parseBox();
            Box& box = res.box;
            switch (box.m_hdr.m_ty)
            {
            case BMF_BOX_mvhd:
                parseMovieHeader(box, movieBox.m_movieHeader, res.bytesParsed);
                bytesRead += box.m_hdr.m_sz;
                break;

            case BMF_BOX_trak:
                movieBox.m_tracks.pushBack(TrackBox());
                parseTrackBox(box, movieBox.m_tracks[movieBox.m_tracks.getSize() - 1], bytesRead);
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

        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, res.box.m_fHdr.m_ver);
        BMF_READ_VAR(buf, 3, m_handle, BMF_CHAR_TO_UINT32, res.box.m_fHdr.m_flgs);
        res.bytesParsed += 4;

        return res;
    }

    void BMF::parseMovieHeader(const Box& box, MovieHeader& header, uint64_t bytesRead)
    {
        BoxParseRes<FullBox> res = parseFullBox(box);
        bytesRead += res.bytesParsed;
        char buf[36] = "\0";

        //TODO: if Version = 1, the format of a few params will change

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_creationTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_modificationTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_timeScale);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_duration);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_preferredRate, 16, 16);
        BMF_READ_FIXED_POINT(buf, 2, m_handle, BMF_CHAR_TO_FP16, header.m_preferredVolume, 8, 8);

        //TODO: This reserved location might be specific to MOV files.
        BMF_READ(header.m_reserved, 10, m_handle);
        
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col0.x, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col1.x, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col2.x, 2, 30);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col0.y, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col1.y, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col2.y, 2, 30);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col0.z, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col1.z, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matStructure.col2.z, 2, 30);

        //TODO: Fields below upto NextTrackID might be specific to QuickTime files
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_previewTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_previewDuration);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_posterTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_selectionTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_selectionDuration)
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_currentTime)

        //Conforms to Base Media File(BMF) file format
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_nextTrackID)
    }

    void BMF::parseTrackBox(const Box& box, TrackBox& trackBox, uint64_t bytesRead)
    {
        
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