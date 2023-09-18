
#define BMF_CHAR_TO_UINT32(c) (c[0] << 24 | c[1] << 16 | c[2] << 8 | c[3])
#define BMF_UINT32_TO_CHAR(c, ui) memcpy(c, &ui, 4)

#define BMF_CHAR_TO_UINT64(c) ((uint64_t)c[0] << 56 | (uint64_t)c[1] << 48 | (uint64_t)c[2] << 40 | (uint64_t)c[3] << 32 \
                                | (uint64_t)c[4] << 24 | (uint64_t)c[5] << 16 | (uint64_t)c[6] << 8 | (uint64_t)c[7])


#define BMF_BOX_ftyp_TO_UINT32 BMF_CHAR_TO_UINT32("ftyp")
#define BMF_BOX_etyp_TO_UINT32 BMF_CHAR_TO_UINT32("etyp")


/*Brands*/
#define BMF_BRAND_QT BMF_CHAR_TO_UINT32("qt  ") // qt followed by 2 spaces