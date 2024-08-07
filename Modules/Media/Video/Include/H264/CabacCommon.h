#ifndef _H_CABAC_TABLES
#define _H_CABAC_TABLES

#include <cstdint>

namespace Quaint { namespace Media{

#define CLIP3(x, y, z) (z < x) ? x\
                        : (z > y) ? y : z


//CABAC context tables for initialization
extern const int8_t C_cabac_context_init_I[1024][2];
extern const int8_t C_cabac_context_init_PB[3][1024][2];
extern const uint8_t C_range_table_lps[64][4];
extern const uint8_t C_state_transition_table[64][2];
}}
#endif //_H_CABAC_TABLES