/*All conversions here respect Big-Endianness*/
#ifndef _H_BMF_HELPERS
#define _H_BMF_HELPERS

#define TO_U16(c) uint16_t((uint8_t)c)
#define TO_I16(c) int16_t((uint8_t)c)
#define TO_U32(c) uint32_t((uint8_t)c)
#define TO_I32(c) int32_t((uint8_t)c)
#define TO_U64(c) uint64_t((uint8_t)c)
#define BMF_CHAR_TO_UINT8(c) (uint8_t)c[0]
#define BMF_CHAR_TO_UINT16(c) (TO_U16(c[0] << 8) | TO_U16(c[1]))
#define BMF_CHAR_TO_INT16(c) (TO_I16(c[0] << 8) | TO_I16(c[1]))
#define BMF_CHAR_TO_UINT32(c) (TO_U32(c[0]) << 24 | TO_U32(c[1]) << 16 | TO_U32(c[2]) << 8 | TO_U32(c[3]))
#define BMF_CHAR_TO_INT32(c) (TO_I32(c[0]) << 24 | TO_I32(c[1]) << 16 | TO_I32(c[2]) << 8 | TO_I32(c[3]))
#define BMF_UINT32_TO_CHAR(c, ui) c[0] = (ui >> 24 & 0xFF);\
        c[1] = (ui >> 16 & 0xFF);\
        c[2] = (ui >> 8 & 0xFF);\
        c[3] = (ui & 0xFF);\

#define BMF_CHAR_TO_UINT64(c) (TO_U64(c[0]) << 56 | TO_U64(c[1]) << 48 | TO_U64(c[2]) << 40 | TO_U64(c[3]) << 32 \
                                | TO_U64(c[4]) << 24 | TO_U64(c[5]) << 16 | TO_U64(c[6]) << 8 | TO_U64(c[7]))

#define BMF_CHAR_TO_UFP32(c, x, y) ((float)(BMF_CHAR_TO_UINT32(c)) / (float)(1 << y))
#define BMF_CHAR_TO_FP32(c, x, y) ((float)(BMF_CHAR_TO_INT32(c)) / (float)(1 << y))
#define BMF_CHAR_TO_UFP16(c, x, y) ((float)(BMF_CHAR_TO_UINT16(c)) / (float)(1 << y))
#define BMF_CHAR_TO_FP16(c, x, y) ((float)(BMF_CHAR_TO_INT16(c)) / (float)(1 << y))

#define BMF_READ(BUFFER, LENGTH, FILE_HANDLE)\
        FILE_HANDLE.read(BUFFER, LENGTH);\
        bytesRead += LENGTH;

#define BMF_READ_VAR(BUFFER, LENGTH, FILE_HANDLE, TYPE_OR_CONV_MACRO, VARIABLE) \
        FILE_HANDLE.read(BUFFER, LENGTH);\
        VARIABLE = TYPE_OR_CONV_MACRO(BUFFER);\
        bytesRead += LENGTH;

#define BMF_READ_FIXED_POINT(BUFFER, LENGTH, FILE_HANDLE, TYPE_OR_CONV_MACRO, VARIABLE, x, y) \
        FILE_HANDLE.read(BUFFER, LENGTH);\
        VARIABLE = TYPE_OR_CONV_MACRO(BUFFER, x, y);\
        bytesRead += LENGTH;

//#define BMF_READ_U(BITPARSER, N)


#define BMF_BOX_ftyp BMF_CHAR_TO_UINT32("ftyp")
#define BMF_BOX_etyp BMF_CHAR_TO_UINT32("etyp")
#define BMF_BOX_mdat BMF_CHAR_TO_UINT32("mdat")
#define BMF_BOX_moov BMF_CHAR_TO_UINT32("moov")
#define BMF_BOX_prfl BMF_CHAR_TO_UINT32("prfl")
#define BMF_BOX_mvhd BMF_CHAR_TO_UINT32("mvhd")
#define BMF_BOX_trak BMF_CHAR_TO_UINT32("trak")
#define BMF_BOX_tkhd BMF_CHAR_TO_UINT32("tkhd")
#define BMF_BOX_edts BMF_CHAR_TO_UINT32("edts")
#define BMF_BOX_elst BMF_CHAR_TO_UINT32("elst")
#define BMF_BOX_mdia BMF_CHAR_TO_UINT32("mdia")
#define BMF_BOX_mdhd BMF_CHAR_TO_UINT32("mdhd")
#define BMF_BOX_hdlr BMF_CHAR_TO_UINT32("hdlr")
#define BMF_BOX_minf BMF_CHAR_TO_UINT32("minf")
#define BMF_BOX_vmhd BMF_CHAR_TO_UINT32("vmhd")
#define BMF_BOX_dinf BMF_CHAR_TO_UINT32("dinf")
#define BMF_BOX_stbl BMF_CHAR_TO_UINT32("stbl")
#define BMF_BOX_stsd BMF_CHAR_TO_UINT32("stsd")
#define BMF_BOX_stts BMF_CHAR_TO_UINT32("stts")

#define BMF_BOX_avcc BMF_CHAR_TO_UINT32("avcC")

/*Brands*/
#define BMF_BRAND_QT BMF_CHAR_TO_UINT32("qt  ") // qt followed by 2 spaces

#endif