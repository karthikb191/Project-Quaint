#include <BMF.h>
#include <VideoModule.h>
#include <MemCore/GlobalMemoryOverrides.h>
#include <QuaintLogger.h>
#include <BMFParsers.h>
#include <iostream>

namespace Quaint {namespace Media{
    DECLARE_LOG_CATEGORY(BMF);
    DEFINE_LOG_CATEGORY(BMF);

    /*Forward Declares*/
    void parseTillEndOfBox(const Box& box, const uint64_t bytesRead, char** buffer, std::fstream& handle, bool skip = false);
    void skip(uint64_t numBytesToSkip, std::fstream& handle);
    
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
            Box box;
            uint64_t bytesRead = parseBox(box);
            switch (box.m_hdr.m_ty)
            {
            case BMF_BOX_ftyp:
                m_fileTypeBox.setBox(box);
                parseFileTypeBox(m_fileTypeBox, bytesRead);
                break;
            case BMF_BOX_mdat:
                m_mediDataBox.setBox(box);
                parseMediaDataBox(m_mediDataBox, bytesRead);
                break;
            case BMF_BOX_moov:
                m_movieBox.setBox(box);
                parseMovieBox(m_movieBox, bytesRead);
                break;
            default:
                break;
            }
        }
    }

    uint64_t BMF::parseBox(Box& box)
    {
        //Get the size and type of box and move to a more specific parsing function
        uint64_t bytesRead = 0;
        char buf[8];
        m_handle.read(buf, 4);
        uint32_t size = BMF_CHAR_TO_UINT32(buf);
        std::cout << std::hex << size;
        m_handle.read(buf, 4);
        bytesRead += 8;

        box = Box(size, buf);

        //Read Extended size
        if(size == 1)
        {
            m_handle.read(buf, 8);
            uint64_t extSize = BMF_CHAR_TO_UINT64(buf);
            box.m_hdr.m_lSz = extSize;
            bytesRead += 8;
        }
        return bytesRead;
    }

    void BMF::parseFileTypeBox(FileTypeBox& ftypBox, uint64_t bytesRead)
    {
        char buf[8];
        m_handle.read(buf, 4);
        ftypBox.m_majorBrand = BMF_CHAR_TO_UINT32(buf);
        m_handle.read(buf, 4);
        ftypBox.m_minor_version = BMF_CHAR_TO_UINT32(buf);

        if(ftypBox.m_majorBrand == BMF_BRAND_QT)
        {
            QLOG_I(BMF, "Major Brand is 'qt'");
        }
        
        bytesRead += 8;

        uint32_t remainingBytes = ftypBox.m_hdr.m_sz - (uint32_t)bytesRead;
        assert(remainingBytes % 4 == 0 && "Parse failed");
        uint32_t numCompatiblBrands = remainingBytes / 4;
        for (uint32_t i = 0; i < numCompatiblBrands; i++)
        {
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT8, ftypBox.m_compatibleBrands[i]);
        }
        //parseTillEndOfBox(box, bytesRead, &resBuffer, m_handle);
        //if(resBuffer != nullptr)
        //{
        //    ftypBox.m_compatibleBrands = (uint32_t*)resBuffer;
        //}
    }

    void BMF::parseMediaDataBox(MediaDataBox& mediaDataBox, uint64_t bytesRead)
    {
        //Skipping media box for now
        if(mediaDataBox.m_hdr.m_sz != 1)
        {
            m_handle.seekg(mediaDataBox.m_hdr.m_sz - bytesRead, std::ios::cur);
        }
        else
        {
            m_handle.seekg(mediaDataBox.m_hdr.m_lSz - bytesRead, std::ios::cur);
        }
    }

    /*This is the most important box that contains all the info needed to sample videos*/
    void BMF::parseMovieBox(MovieBox& movieBox, uint64_t bytesRead)
    {
        assert(movieBox.m_hdr.m_sz > 1 && "Extended sizes must be handled");
        
        //TODO: Hack to only parse movie track for now. Remove this later
        bool parsedVideoTrack = false;

        while(bytesRead < movieBox.m_hdr.m_sz)
        {
            Box box;
            uint64_t bytesParsed = parseBox(box);
            switch (box.m_hdr.m_ty)
            {
            case BMF_BOX_mvhd:
                movieBox.m_movieHeader.setBox(box);
                parseMovieHeader(movieBox.m_movieHeader, bytesParsed);
                break;

            case BMF_BOX_trak:
            {
                if(parsedVideoTrack)
                {
                    parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
                    break;
                }

                TrackBox tb(VideoModule::get().getVideoMemoryContext());
                tb.setBox(box);
                movieBox.m_tracks.pushBack(tb);
                parseTrackBox(movieBox.m_tracks[movieBox.m_tracks.getSize() - 1], bytesParsed);
                parsedVideoTrack = true;
                break;
            }
            
            default:
                //Box of unknown type encountered. Skip it.
                QLOG_W(BMF, "Encountered box of unkonwn type when parsing Movie box. Skipping it!")
                parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
                break;
            }
            bytesRead += box.m_hdr.m_sz;
        }
    }

    uint64_t BMF::parseFullBox(FullBox& fullBox)
    {
        //BoxParseRes<FullBox> res;
        //res.box = Box(box);
        uint64_t bytesRead = 0;
        char buf[4] = "\0";

        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, fullBox.m_fHdr.m_ver);
        BMF_READ_VAR(buf, 3, m_handle, BMF_CHAR_TO_UINT32, fullBox.m_fHdr.m_flags.uiFlags);

        return bytesRead;
    }

    void BMF::parseMovieHeader(MovieBox::MovieHeader& header, uint64_t bytesRead)
    {
        uint64_t bytesParsed = parseFullBox(header);
        bytesRead += bytesParsed;
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

    void BMF::parseTrackHeaderBox(TrackBox::TrackHeaderBox& header, uint64_t bytesRead)
    {
        assert(header.m_hdr.m_sz > 1 && "Extended sizes must be handled");

        uint64_t bytesParsed = parseFullBox(header);
        bytesRead += bytesParsed;
        char buf[36] = "\0";

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_creationTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_modificationTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_trackId);
        BMF_READ(header.m_reserved1, 4, m_handle);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, header.m_duration);
        BMF_READ(header.m_reserved2, 8, m_handle);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_INT16, header.m_layer);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_INT16, header.m_alternateGroup);
        BMF_READ_FIXED_POINT(buf, 2, m_handle, BMF_CHAR_TO_FP16, header.m_volume, 8, 8);
        BMF_READ(header.m_reserved3, 2, m_handle);

        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col0.x, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col1.x, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col2.x, 2, 30);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col0.y, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col1.y, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col2.y, 2, 30);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col0.z, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col1.z, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_matrix.col2.z, 2, 30);

        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_width, 16, 16);
        BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, header.m_height, 16, 16);

        bool enabled = header.isTrackEnabled();
        bool canPresent = header.canTrackOrAnySubTrackPresentDirectly();
        canPresent = header.canTrackOrAnySubTrackPresentDirectly();

    }

    void BMF::parseEditBox(TrackBox::EditBox& edit, uint64_t bytesRead)
    {
        while(bytesRead < edit.m_hdr.m_sz)
        {
            Box box;
            uint64_t bytesParsed = parseBox(box);
            if(box.m_hdr.m_ty == BMF_BOX_elst)
            {
                TrackBox::EditBox::EditListBox ed = TrackBox::EditBox::EditListBox(VideoModule::get().getVideoMemoryContext());
                ed.setBox(box); 
                edit.m_editLists.pushBack(ed);
                parseEditListBox(edit.m_editLists[edit.m_editLists.getSize() - 1], bytesParsed);
            }
            else
            {
                parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
            }
            bytesRead += box.m_hdr.m_sz;
        }
    }

    void BMF::parseEditListBox(TrackBox::EditBox::EditListBox& editList, uint64_t bytesRead)
    {
        uint64_t bytesParsed = parseFullBox(editList);
        bytesRead += bytesParsed;
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, editList.m_numEntries);
        for(uint32_t i = 0; i < editList.m_numEntries; ++i)
        {
            TrackBox::EditBox::EditListBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_duration);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_mediaTime);
            BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_FP32, entry.m_mediaRate, 16, 16);

            editList.m_entries.pushBack(entry);
        }
    }

    void BMF::parseTrackBox(TrackBox& trackBox, uint64_t bytesRead)
    {
        assert(trackBox.m_hdr.m_sz > 1 && "Extended sizes must be handled");   
        
        while(bytesRead < trackBox.m_hdr.m_sz)
        {
            Box box;
            uint64_t bytesParsed = parseBox(box);
            switch (box.m_hdr.m_ty)
            {
            case BMF_BOX_tkhd:
                trackBox.m_trackHeader.setBox(box);
                parseTrackHeaderBox(trackBox.m_trackHeader, bytesParsed);
                break;

            case BMF_BOX_edts:
                trackBox.m_edit.setBox(box);
                parseEditBox(trackBox.m_edit, bytesParsed);
                break;
            
            case BMF_BOX_mdia:
                trackBox.m_media.setBox(box);
                parseMediaBox(trackBox.m_media, bytesParsed);
                break;
            
            default:
                //Box of unknown type encountered. Skip it.
                QLOG_W(BMF, "Encountered box of unkonwn type when parsing Track box. Skipping it!")
                parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
                
                break;
            }
            bytesRead += box.m_hdr.m_sz;
        }

    }

    void BMF::parseMediaBox(TrackBox::MediaBox& mediaBox, uint64_t bytesRead)
    {
        while(bytesRead < mediaBox.m_hdr.m_sz)
        {
            Box box;
            uint64_t bytesParsed = parseBox(box);
            switch(box.m_hdr.m_ty)
            {
            case BMF_BOX_mdhd:
                mediaBox.m_movieHeader.setBox(box);
                parseMediaHeaderBox(mediaBox.m_movieHeader, bytesParsed);
                break;
            
            case BMF_BOX_hdlr:
                mediaBox.m_handler.setBox(box);
                parseMediaHandlerReferenceBox(mediaBox.m_handler, bytesParsed);
                break;

            case BMF_BOX_minf:
                //TODO: Add a condition to check if we are parsing a video track
                mediaBox.m_mediaInfo.setBox(box);
                parseVideoMediaInformationBox(mediaBox.m_mediaInfo, bytesParsed);
                break;

            default:
                QLOG_W(BMF, "Encountered box of unkonwn type when parsing Media box. Skipping it!")
                parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
                break;
            }
            bytesRead += box.m_hdr.m_sz;
        }
    }

    void BMF::parseMediaHeaderBox(TrackBox::MediaBox::MediaHeaderBox& headerBox, uint64_t bytesRead)
    {
        uint64_t bytesParsed = parseFullBox(headerBox);
        bytesRead += bytesParsed;
        assert(headerBox.m_fHdr.m_ver == 0 && "Version other than 0 is not yet supported");
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, headerBox.m_creationTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, headerBox.m_modificationTime);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, headerBox.m_timeScale);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, headerBox.m_duration);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, headerBox.m_language);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, headerBox.m_quality);
    }

    void BMF::parseMediaHandlerReferenceBox(HandlerReferenceBox& handlerBox, uint64_t bytesRead)
    {
        uint64_t bytesParsed = parseFullBox(handlerBox);
        bytesRead += bytesParsed;
        assert(handlerBox.m_fHdr.m_ver == 0 && "Version other than 0 is not yet supported");
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, handlerBox.m_componentType.m_val);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, handlerBox.m_componentSubType.m_val);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, handlerBox.m_manufacturer);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, handlerBox.m_flags);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, handlerBox.m_flagsMask);
        
        uint64_t remainingBytes = handlerBox.m_hdr.m_sz - bytesRead;
        assert(remainingBytes < handlerBox.m_name.availableSize() && "Handler name is unexpectedly large");
        BMF_READ(handlerBox.m_name.getBuffer_NonConst(), remainingBytes, m_handle);
    }

    void BMF::parseVideoMediaInformationBox(MediaInformationBox& videoMinf, uint64_t bytesRead)
    {
        while(bytesRead < videoMinf.m_hdr.m_sz)
        {
            Box box;
            uint64_t bytesParsed = parseBox(box);
            switch(box.m_hdr.m_ty)
            {
            case BMF_BOX_vmhd:
                videoMinf.m_vMinfHeader.setBox(box);
                parseVideoMediaInformationHeaderBox(videoMinf.m_vMinfHeader, bytesParsed);
                break;

            case BMF_BOX_hdlr:
                videoMinf.m_handler.setBox(box);
                parseMediaHandlerReferenceBox(videoMinf.m_handler, bytesParsed);
                break;
            
            case BMF_BOX_dinf:
                videoMinf.m_dataInformation.setBox(box);
                parseDataInformationBox(videoMinf.m_dataInformation, bytesParsed);
                break;

            case BMF_BOX_stbl:
                videoMinf.m_sampleTable.setBox(box);
                parseSampleTableBox(videoMinf.m_sampleTable, bytesParsed);
                break;

            default:
                QLOG_W(BMF, "Encountered box of unkonwn type when parsing Video Media Information box. Skipping it!")
                parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
                break;
            }
            bytesRead += box.m_hdr.m_sz;
        }
    }

    void BMF::parseVideoMediaInformationHeaderBox(MediaInformationBox::VideoMediaInformationHeaderBox& vMinfHeader, uint64_t bytesRead)
    {
        uint64_t bytesParsed = parseFullBox(vMinfHeader);
        bytesRead += bytesParsed;
        assert(vMinfHeader.m_fHdr.m_ver == 0 && "Version other than 0 is not yet supported");
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, vMinfHeader.m_graphicsMode);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, vMinfHeader.m_opCode.m_r);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, vMinfHeader.m_opCode.m_g);
        BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, vMinfHeader.m_opCode.m_b);
    }

    void BMF::parseDataInformationBox(DataInformationBox& dataInformation, uint64_t bytesRead)
    {
        //Parsing Data Reference box
        Box box;
        bytesRead += parseBox(box);
        dataInformation.m_dataRef.setBox(box);
        bytesRead += parseFullBox(dataInformation.m_dataRef);
        
        char buf[8] = {'\0'};
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT64, dataInformation.m_dataRef.m_numEntries);
        
        assert(dataInformation.m_dataRef.m_numEntries == 0 && "Valid Data Reference entries is not handled yet!");

        for(uint32_t i = 0; i < dataInformation.m_dataRef.m_numEntries; ++i)
        {
            DataReferenceEntry entry;
            uint64_t currentEntryBytes = 0;
            currentEntryBytes += parseBox(entry);
            currentEntryBytes += parseFullBox(entry);

            uint64_t remaining = entry.m_hdr.m_sz - currentEntryBytes;

            skip(remaining, m_handle);

            bytesRead += entry.m_hdr.m_sz;
        }

    }

    void BMF::parseSampleTableBox(SampleTableBox& sampleTable, uint64_t bytesRead)
    {
        while(bytesRead < sampleTable.m_hdr.m_sz)
        {
            Box box;
            uint64_t bytesParsed = parseBox(box);
            switch(box.m_hdr.m_ty)
            {
            case BMF_BOX_stsd:
                sampleTable.m_description.setBox(box);
                parseSampleDescriptionBox(sampleTable.m_description, bytesParsed);
                break;
            
            case BMF_BOX_avcc:
                sampleTable.m_avcConfig.setBox(box);
                parseAVCCBox(sampleTable.m_avcConfig, bytesParsed);
                break;

            case BMF_BOX_stts:
                sampleTable.m_sampleToTime.setBox(box);
                parseSampleToTimeBox(sampleTable.m_sampleToTime, bytesParsed);
                break;
            
            case BMF_BOX_stss:
                sampleTable.m_syncSample.setBox(box);
                parseSyncSampleBox(sampleTable.m_syncSample, bytesParsed);
                break;
            
            case BMF_BOX_ctts:
                sampleTable.m_compOffset.setBox(box);
                parseCompositionOffsetBox(sampleTable.m_compOffset, bytesParsed);
                break;
                
            case BMF_BOX_stsc:
                sampleTable.m_sampleToChunk.setBox(box);
                parseSampleToChunkBox(sampleTable.m_sampleToChunk, bytesParsed);
                break;

            case BMF_BOX_stsz:
                sampleTable.m_sampleSize.setBox(box);
                parseSampleSizeBox(sampleTable.m_sampleSize, bytesParsed);
                break;
            
            case BMF_BOX_stco:
                sampleTable.m_chunkOffset.setBox(box);
                parseChunkOffsetBox(sampleTable.m_chunkOffset, bytesParsed);
                break;

            default:
                QLOG_W(BMF, "Encountered box of unkonwn type when parsing Video Media Information box. Skipping it!")
                parseTillEndOfBox(box, bytesParsed, nullptr, m_handle, true);
                break;
            }
            bytesRead += box.m_hdr.m_sz;
        }
    }

    void BMF::parseSampleDescriptionBox(SampleTableBox::SampleDescriptionBox& description, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(description);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, description.m_numEntries);
        for(uint32_t i = 0; i < description.m_numEntries; ++i)
        {
            VideoSampleDescription sample;

            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sample.m_sampleDescriptionSize);

            //AVC1 is being registered here. TODO: Remove this comment. The sample type in data might depend on this. Investigate
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sample.m_dataFormat.m_val);
            BMF_READ(sample.m_reserved1, 6, m_handle);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_dataRefIndex);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_version);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_revisionLevel);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sample.m_vendor.m_val);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sample.m_temporalQuality);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sample.m_spatialQuality);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_width);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_height);
            BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_UFP32, sample.m_horRes, 16, 16);
            BMF_READ_FIXED_POINT(buf, 4, m_handle, BMF_CHAR_TO_UFP32, sample.m_vertRes, 16, 16);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sample.m_dataSize);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_frameCount);
            BMF_READ(sample.m_compressorName.getBuffer_NonConst(), 32, m_handle);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_pixelDepth);
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, sample.m_colorTableId);
            description.m_samples.pushBack(sample);
        }
    }

    void BMF::parseSampleToTimeBox(SampleToTimeBox& sampleToTime, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(sampleToTime);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sampleToTime.m_numEntries);
        for(uint32_t i = 0; i < sampleToTime.m_numEntries; i++)
        {
            SampleToTimeBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_sampleCount);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_sampleDuration);
            sampleToTime.m_entries.pushBack(entry);
        }

        assert(sampleToTime.m_hdr.m_sz == bytesRead && "Invalid parse");
    }

    void BMF::parseSyncSampleBox(SyncSampleBox& syncSample, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(syncSample);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, syncSample.m_numEntries);
        for(uint32_t i = 0; i < syncSample.m_numEntries; i++)
        {
            SyncSampleBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_sampleNum);
            syncSample.m_entries.pushBack(entry);
        }

        assert(syncSample.m_hdr.m_sz == bytesRead && "Invalid parse");
    }

    void BMF::parseCompositionOffsetBox(CompositionOffsetBox& compOffset, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(compOffset);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, compOffset.m_numEntries);
        for(uint32_t i = 0; i < compOffset.m_numEntries; i++)
        {
            CompositionOffsetBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_sampleCount);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_compOffset);
            compOffset.m_entries.pushBack(entry);
        }

        assert(compOffset.m_hdr.m_sz == bytesRead && "Invalid parse");
    }

    void BMF::parseSampleToChunkBox(SampleToChunkBox& sampleToChunk, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(sampleToChunk);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sampleToChunk.m_numEntries);
        for(uint32_t i = 0; i < sampleToChunk.m_numEntries; i++)
        {
            SampleToChunkBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_firstChunk);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_samplesPerChunk);
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_sampleDescriptionId);
            sampleToChunk.m_entries.pushBack(entry);
        }

        assert(sampleToChunk.m_hdr.m_sz == bytesRead && "Invalid parse");
    }

    void BMF::parseSampleSizeBox(SampleSizeBox& sampleSize, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(sampleSize);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sampleSize.m_sampleSize);
        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, sampleSize.m_numEntries);
        for(uint32_t i = 0; i < sampleSize.m_numEntries; i++)
        {
            SampleSizeBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_sampleSize);
            sampleSize.m_entries.pushBack(entry);
        }

        assert(sampleSize.m_hdr.m_sz == bytesRead && "Invalid parse");
    }
    
    void BMF::parseChunkOffsetBox(ChunkOffsetBox& chunkOffset, uint64_t bytesRead)
    {
        bytesRead += parseFullBox(chunkOffset);
        
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, chunkOffset.m_numEntries);
        for(uint32_t i = 0; i < chunkOffset.m_numEntries; i++)
        {
            ChunkOffsetBox::Entry entry;
            BMF_READ_VAR(buf, 4, m_handle, BMF_CHAR_TO_UINT32, entry.m_chunkOffset);
            chunkOffset.m_entries.pushBack(entry);
        }

        assert(chunkOffset.m_hdr.m_sz == bytesRead && "Invalid parse");
    }

    void BMF::parseAVCCBox(AVCConfigurationBox& avcConfigBox, uint64_t bytesRead)
    {
        char buf[8] = {'\0'};

        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_version);
        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_avcProfileIndication);
        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_profileCompatibility);
        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_avcLevelIndication);
        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_nalUnitLength);
        avcConfigBox.m_decoderRecord.m_nalUnitLength &= 0b11 + 1;
        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_numSequenceParamSets);
        avcConfigBox.m_decoderRecord.m_numSequenceParamSets &= 0b11111;
        
        for(int i = 0; i < avcConfigBox.m_decoderRecord.m_numSequenceParamSets; i++)
        {
            uint16_t paramSetLength = 0;
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, paramSetLength);

            SequenceParameterSetNALUnit unit(VideoModule::get().getVideoMemoryContext(), avcConfigBox.m_decoderRecord.m_nalUnitLength);
            
            BitParser parser(VideoModule::get().getVideoMemoryContext(), paramSetLength);
            BMF_READ(parser.getBuffer_NonConst(), paramSetLength, m_handle);

            //TODO: PERF IMPROVEMENT
            unit.dump(parser.getBuffer_NonConst(), paramSetLength);
            avcConfigBox.m_decoderRecord.m_sequenceParamSets.pushBack(unit);
            uint32_t index = avcConfigBox.m_decoderRecord.m_sequenceParamSets.getSize() - 1;
            SequenceParameterSetNALUnit& uu = avcConfigBox.m_decoderRecord.m_sequenceParamSets[index];
            uu.parse(parser, &avcConfigBox.m_decoderRecord);
            //avcConfigBox.m_decoderRecord.m_sequenceParamSets[index].parse(parser, &avcConfigBox.m_decoderRecord);
        }

        BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_numPictureParamSets);

        for(int i = 0; i < avcConfigBox.m_decoderRecord.m_numPictureParamSets; i++)
        {
            uint16_t paramSetLength = 0;
            BMF_READ_VAR(buf, 2, m_handle, BMF_CHAR_TO_UINT16, paramSetLength);

            PictureParameterSetNALUnit unit(VideoModule::get().getVideoMemoryContext(), avcConfigBox.m_decoderRecord.m_nalUnitLength);
            
            BitParser parser(VideoModule::get().getVideoMemoryContext(), paramSetLength);
            BMF_READ(parser.getBuffer_NonConst(), paramSetLength, m_handle);
            
            unit.dump(parser.getBuffer_NonConst(), paramSetLength);
            //TODO: PERF IMPROVEMENT
            avcConfigBox.m_decoderRecord.m_pictureParamSets.pushBack(unit);
            avcConfigBox.m_decoderRecord.m_pictureParamSets[avcConfigBox.m_decoderRecord.m_pictureParamSets.getSize() - 1].parse(parser, &avcConfigBox.m_decoderRecord);
        }

        if(bytesRead == avcConfigBox.m_hdr.m_sz) return;
        
        uint32_t profile_idc = avcConfigBox.m_decoderRecord.m_avcProfileIndication;
        if( profile_idc == 100 || profile_idc == 110 ||
            profile_idc == 122 || profile_idc == 144 )
        {
            BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_chroma_format)
            BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_bit_depth_luma)
            BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_bit_depth_chroma)
            BMF_READ_VAR(buf, 1, m_handle, BMF_CHAR_TO_UINT8, avcConfigBox.m_decoderRecord.m_numSequenceParamSetExt)
            
            avcConfigBox.m_decoderRecord.m_chroma_format = avcConfigBox.m_decoderRecord.m_chroma_format & 0b11;
            avcConfigBox.m_decoderRecord.m_bit_depth_luma = (avcConfigBox.m_decoderRecord.m_bit_depth_luma & 0b111) + 8;
            avcConfigBox.m_decoderRecord.m_bit_depth_chroma = (avcConfigBox.m_decoderRecord.m_bit_depth_chroma & 0b111) + 8;

            assert(avcConfigBox.m_decoderRecord.m_numSequenceParamSetExt == 1 && "No support for EXT param sets yet");
            // for (uint32_t i=0; i < avcConfigBox.m_decoderRecord.m_numSequenceParamSetExt; i++) 
            // {
            //     unsigned int(16) sequenceParameterSetExtLength;
            //     bit(8*sequenceParameterSetExtLength) sequenceParameterSetExtNALUnit;
            // }
        }

        assert(avcConfigBox.m_hdr.m_sz == bytesRead && "Invalid parse");
    }
    

    /*Helpers*/
    void skip(uint64_t numBytesToSkip, std::fstream& handle)
    {
        if(numBytesToSkip <= 0) return;

        handle.seekg(numBytesToSkip, std::ios::cur);
    }
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

        if(remaining != 0)
        {
            handle.seekg(remaining, std::ios::cur);
            //*buffer = (char*)QUAINT_ALLOC_MEMORY(VideoModule::get().getVideoMemoryContext(), (size_t)remaining);
            //handle.read(*buffer, remaining);
        }
    }
}}