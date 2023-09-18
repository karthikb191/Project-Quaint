
#define TO_U32(c) uint32_t((uint8_t)c)
#define TO_U64(c) uint64_t((uint8_t)c)
#define BMF_CHAR_TO_UINT32(c) (TO_U32(c[0]) << 24 | TO_U32(c[1]) << 16 | TO_U32(c[2]) << 8 | TO_U32(c[3]))
#define BMF_UINT32_TO_CHAR(c, ui) memcpy(c, &ui, 4)

#define BMF_CHAR_TO_UINT64(c) (TO_U64(c[0]) << 56 | TO_U64(c[1]) << 48 | TO_U64(c[2]) << 40 | TO_U64(c[3]) << 32 \
                                | TO_U64(c[4]) << 24 | TO_U64(c[5]) << 16 | TO_U64(c[6]) << 8 | TO_U64(c[7]))


#define BMF_BOX_ftyp BMF_CHAR_TO_UINT32("ftyp")
#define BMF_BOX_etyp BMF_CHAR_TO_UINT32("etyp")
#define BMF_BOX_mdat BMF_CHAR_TO_UINT32("mdat")
#define BMF_BOX_moov BMF_CHAR_TO_UINT32("moov")

/*Brands*/
#define BMF_BRAND_QT BMF_CHAR_TO_UINT32("qt  ") // qt followed by 2 spaces