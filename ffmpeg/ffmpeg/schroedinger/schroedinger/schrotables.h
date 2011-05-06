
#ifndef __SCHRO_SCHRO_TABLES_H__
#define __SCHRO_SCHRO_TABLES_H__

#include <schroedinger/schroutils.h>
#include <schroedinger/schrobitstream.h>

SCHRO_BEGIN_DECLS

#ifdef SCHRO_ENABLE_UNSTABLE_API

extern const uint32_t schro_table_offset_3_8[61];
extern const uint32_t schro_table_offset_1_2[61];
extern const uint32_t schro_table_quant[61];
extern const uint32_t schro_table_inverse_quant[61];
extern const uint16_t schro_table_division_factor[257];
extern const double schro_table_error_hist_shift3_1_2[60][104];
extern const double schro_table_onebits_hist_shift3_1_2[60][104];
extern const double schro_table_zerobits_hist_shift3_1_2[60][104];

extern const float schro_tables_wavelet_noise_curve[SCHRO_N_WAVELETS][8][128];
extern const double schro_tables_wavelet_gain[SCHRO_N_WAVELETS][2];

extern const int schro_table_unpack_sint[256][17];

#endif

SCHRO_END_DECLS

#endif

