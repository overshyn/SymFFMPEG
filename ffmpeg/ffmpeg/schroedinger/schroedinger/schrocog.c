
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schrocog.h>
#include <schroedinger/schrooil.h>
#include <liboil/liboil.h>


void
schro_cog_mas8_u8_edgeextend (uint8_t *d, const uint8_t *s,
    const int16_t *taps, int offset, int shift, int index_offset, int n)
{
  int i,j;
  int x;
#ifdef __CW32__
  int16_t offsetshift[2];
  offsetshift[0] = offset;
  offsetshift[1] = shift;
#else
  int16_t offsetshift[] = { offset, shift };
#endif

  if (n <= 8) {
    for(i=0;i<n;i++){
      x = 0;
      for(j=0;j<8;j++) {
        x += s[CLAMP(i+j-index_offset,0,n-1)]*taps[j];
      }
      d[i] = CLAMP((x + offset)>>shift,0,255);
    }
  } else {
    for(i=0;i<index_offset;i++){
      x = 0;
      for(j=0;j<8;j++) {
        x += s[CLAMP(i+j-index_offset,0,n-1)]*taps[j];
      }
      d[i] = CLAMP((x + offset)>>shift,0,255);
    }
    oil_mas8_u8_sym_l15 (d+index_offset, s, taps, offsetshift, n - 8);
    for(i=n-8+index_offset;i<n;i++){
      x = 0;
      for(j=0;j<8;j++) {
        x += s[CLAMP(i+j-index_offset,0,n-1)]*taps[j];
      }
      d[i] = CLAMP((x + offset)>>shift,0,255);
    }
  }
}

void
schro_cog_mas10_u8_edgeextend (uint8_t *d, const uint8_t *s,
    const int16_t *taps, int offset, int shift, int index_offset, int n)
{
  int i,j;
  int x;
#ifdef __CW32__
  int16_t offsetshift[2];
  offsetshift[0] = offset;
  offsetshift[1] = shift;
#else
  int16_t offsetshift[] = { offset, shift };
#endif

  if (n < 10) {
    for(i=0;i<n;i++){
      x = 0;
      for(j=0;j<10;j++) {
        x += s[CLAMP(i+j-index_offset,0,n-1)]*taps[j];
      }
      d[i] = CLAMP((x + offset)>>shift,0,255);
    }
  } else {
    for(i=0;i<index_offset;i++){
      x = 0;
      for(j=0;j<10;j++) {
        x += s[CLAMP(i+j-index_offset,0,n-1)]*taps[j];
      }
      d[i] = CLAMP((x + offset)>>shift,0,255);
    }
    oil_mas10_u8 (d+index_offset, s, taps, offsetshift, n - 10);
    for(i=n-10+index_offset;i<n;i++){
      x = 0;
      for(j=0;j<10;j++) {
        x += s[CLAMP(i+j-index_offset,0,n-1)]*taps[j];
      }
      d[i] = CLAMP((x + offset)>>shift,0,255);
    }
  }
}

