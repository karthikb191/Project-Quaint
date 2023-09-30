/*All conversions here respect Big-Endianness*/

#define TO_U16(c) uint16_t((uint8_t)c)
#define TO_U32(c) uint32_t((uint8_t)c)
#define TO_U64(c) uint64_t((uint8_t)c)
#define BMF_CHAR_TO_UINT8(c) (uint8_t)c[0]
#define BMF_CHAR_TO_UINT16(c) (TO_U16(c[0] << 8) | TO_U16(c[1]))
#define BMF_CHAR_TO_UINT32(c) (TO_U32(c[0]) << 24 | TO_U32(c[1]) << 16 | TO_U32(c[2]) << 8 | TO_U32(c[3]))
#define BMF_UINT32_TO_CHAR(c, ui) c[0] = (ui >> 24 & 0xFF);\
        c[1] = (ui >> 16 & 0xFF);\
        c[2] = (ui >> 8 & 0xFF);\
        c[3] = (ui & 0xFF);\

#define BMF_CHAR_TO_UINT64(c) (TO_U64(c[0]) << 56 | TO_U64(c[1]) << 48 | TO_U64(c[2]) << 40 | TO_U64(c[3]) << 32 \
                                | TO_U64(c[4]) << 24 | TO_U64(c[5]) << 16 | TO_U64(c[6]) << 8 | TO_U64(c[7]))

#define BMF_CHAR_TO_FIXED_POINT32(c, x, y) ((float)(BMF_CHAR_TO_UINT32(c)) / (float)(1 << y))
#define BMF_CHAR_TO_FIXED_POINT16(c, x, y) ((float)(BMF_CHAR_TO_UINT16(c)) / (float)(1 << y))


#define BMF_EXPAND(...) __VA_ARGS__
#define BMF_READ(BUFFER, LENGTH, FILE_HANDLE, TYPE_OR_CONV_MACRO, VARIABLE) \
        FILE_HANDLE.read(BUFFER, LENGTH);\
        VARIABLE = TYPE_OR_CONV_MACRO(BUFFER);

#define BMF_READ_FP32(BUFFER, LENGTH, FILE_HANDLE, VARIABLE, x, y) \
        FILE_HANDLE.read(BUFFER, LENGTH);\
        VARIABLE = BMF_CHAR_TO_FIXED_POINT32(BUFFER, x, y);\

#define BMF_READ_FP16(BUFFER, LENGTH, FILE_HANDLE, VARIABLE, x, y) \
        FILE_HANDLE.read(BUFFER, LENGTH);\
        VARIABLE = BMF_CHAR_TO_FIXED_POINT16(BUFFER, x, y);\



#define BMF_BOX_ftyp BMF_CHAR_TO_UINT32("ftyp")
#define BMF_BOX_etyp BMF_CHAR_TO_UINT32("etyp")
#define BMF_BOX_mdat BMF_CHAR_TO_UINT32("mdat")
#define BMF_BOX_moov BMF_CHAR_TO_UINT32("moov")
#define BMF_BOX_prfl BMF_CHAR_TO_UINT32("prfl")
#define BMF_BOX_mvhd BMF_CHAR_TO_UINT32("mvhd")

/*Brands*/
#define BMF_BRAND_QT BMF_CHAR_TO_UINT32("qt  ") // qt followed by 2 spaces