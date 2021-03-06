
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schro.h>
#include <schroedinger/schrowavelet.h>
#include <schroedinger/schrofft.h>
#include <schroedinger/schrooil.h>
#include <liboil/liboil.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"

int16_t tmp[2000];
int16_t tmp2[2000];

int filtershift[] = { 1, 1, 1, 0, 1, 0, 1 };

void synth(int16_t *a, int filter, int n);
void split (int16_t *a, int filter, int n);
void synth_schro_ext (int16_t *a, int filter, int n);
void split_schro_ext (int16_t *a, int n, int filter);
void deinterleave (int16_t *a, int n);
void interleave (int16_t *a, int n);
void dump (int16_t *a, int n);
void dump_cmp (int16_t *a, int16_t *b, int n);
void solve (double *matrix, double *col, int n);

#define SHIFT 8
#define N (1<<SHIFT)

void
random_test(double *dest, int filter, int n_levels, int hl)
{
  int16_t *a = tmp+10;
  float sintable[N];
  float costable[N];
  float dr[N];
  float di[N];
  float sr[N];
  float si[N];
  double power[N];
  int i;
  int j;
  int n_cycles = 100000;
  int amplitude = 100;
  double fs;

  for(i=0;i<N;i++){
    power[i] = 0;
  }

  schro_fft_generate_tables_f32 (costable, sintable, SHIFT);

  for(j=0;j<n_cycles;j++){
    for(i=0;i<N;i++){
      a[i] = 0;
    }
    if (hl) {
      for(i=0;i<(N>>n_levels);i++){
        a[i]=rint(amplitude*random_std());
      }
    } else {
      for(i=(N>>n_levels);i<(N>>(n_levels-1));i++){
        a[i]=rint(amplitude*random_std());
      }
    }

    for(i=n_levels-1;i>=0;i--){
      interleave(a,N>>i);
      synth_schro_ext(a,N>>i,filter);
    }

    for(i=0;i<N;i++){
      si[i] = 0;
      sr[i] = a[i];
    }

    schro_fft_fwd_f32 (dr, di, sr, si, costable, sintable, SHIFT);

    for(i=0;i<N;i++){
      power[i] += dr[i]*dr[i]+di[i]*di[i];
    }

  }

  //fs = 1.0/sqrt(1<<(filtershift[filter]*n_levels));
  fs = 1.0/(1<<(filtershift[filter]*n_levels));
  for(i=0;i<(N>>1);i++){
    double x;

    x = power[i]/n_cycles;
    x /= (amplitude*amplitude);
    /* divide by FFT normalization */
    x /= (1<<SHIFT);

    /* filtershift */
    x *= fs;

    dest[i] = x;
  }
}

int
quant_index (double x)
{
  int i = 0;

  if (x > 1e10) return 60;

  x *= x;
  x *= x;
  while (x*x > 2) {
    x *= 0.5;
    i++;
    if (i >= 60) break;
  }

  return i;
}

float wavelet_noise_curve[SCHRO_N_WAVELETS][8][N/2];
double wavelet_gain[SCHRO_N_WAVELETS][2];

int
main (int argc, char *argv[])
{
  int filter;
  double curve[N];
  int i;
  int j;

  schro_init();

  printf("/* This file is autogenerated */\n");
  printf("\n");
  printf("#include <schroedinger/schrotables.h>\n");
  printf("\n");
  printf("const float schro_tables_wavelet_noise_curve[SCHRO_N_WAVELETS][8][%d] = {\n", N/2);
  for(filter=0;filter<SCHRO_N_WAVELETS;filter++){
    
    printf("{\n");
    for(i=0;i<8;i++){
      random_test(curve, filter, 1+(i>>1), i&1);
      printf("  { /* wavelet type %d, n_levels %d, %s */\n",
          filter, 1+(i>>1), (i&1)?"low":"high");
      for(j=0;j<N/2;j++){
        if ((j&3) == 0) printf("   ");
        printf(" %8.6f,", curve[j]);
        if ((j&3) == 3) printf("\n");
        wavelet_noise_curve[filter][i][j] = curve[j];
      }
      printf((i<7)?"  },\n":"}\n"); 
    }
    printf((filter<(SCHRO_N_WAVELETS-1))?"},\n":"}\n"); 
  }
  printf("};\n"); 
  printf("\n"); 

  printf("const double schro_tables_wavelet_gain[SCHRO_N_WAVELETS][2] = {\n");
  for(filter=0;filter<SCHRO_N_WAVELETS;filter++){
    double sum1 = 0;
    double sum2 = 0;
    for(j=0;j<N/2;j++){
      sum1 += wavelet_noise_curve[filter][0][j];
      sum2 += wavelet_noise_curve[filter][1][j];
    }
    sum1 *= 1<<filtershift[filter];
    sum1 /= N/2;
    sum2 *= 1<<filtershift[filter];
    sum2 /= N/2;
sum1 *= 4;
sum2 *= 4;
    printf("  { %g, %g },\n", sum1, sum2);
    wavelet_gain[filter][0] = sum1;
    wavelet_gain[filter][1] = sum2;
  }
  printf("};\n"); 
  printf("\n"); 

  printf("#if 0\n");
  printf("const int schro_tables_lowdelay_quants[SCHRO_N_WAVELETS][4][9] = {\n");
  for(filter=0;filter<SCHRO_N_WAVELETS;filter++){
    printf("  { /* wavelet %d */\n", filter);
    for(i=1;i<=4;i++){
      double alpha, beta;
      double gains[10];
      double min;
      int n;
      double shift;

      n = 1 + 2*i;
      alpha = wavelet_gain[filter][1];
      beta = wavelet_gain[filter][0];
      shift = alpha / (1<<filtershift[filter]);

      switch(i) {
        case 1:
          gains[0] = sqrt(alpha*alpha);
          gains[1] = sqrt(alpha*beta);
          gains[2] = sqrt(beta*beta);
          break;
        case 2:
          gains[0] = sqrt(alpha*alpha)*shift;
          gains[1] = sqrt(alpha*beta)*shift;
          gains[2] = sqrt(beta*beta)*shift;
          gains[3] = sqrt(alpha*beta);
          gains[4] = sqrt(beta*beta);
          break;
        case 3:
          gains[0] = sqrt(alpha*alpha)*shift*shift;
          gains[1] = sqrt(alpha*beta)*shift*shift;
          gains[2] = sqrt(beta*beta)*shift*shift;
          gains[3] = sqrt(alpha*beta)*shift;
          gains[4] = sqrt(beta*beta)*shift;
          gains[5] = sqrt(alpha*beta);
          gains[6] = sqrt(beta*beta);
          break;
        case 4:
          gains[0] = sqrt(alpha*alpha)*shift*shift*shift;
          gains[1] = sqrt(alpha*beta)*shift*shift*shift;
          gains[2] = sqrt(beta*beta)*shift*shift*shift;
          gains[3] = sqrt(alpha*beta)*shift*shift;
          gains[4] = sqrt(beta*beta)*shift*shift;
          gains[5] = sqrt(alpha*beta)*shift;
          gains[6] = sqrt(beta*beta)*shift;
          gains[7] = sqrt(alpha*beta);
          gains[8] = sqrt(beta*beta);
          break;
      }

      min = gains[0];
      for(j=0;j<n;j++){
        if (gains[j] < min) min = gains[j];
      }
      for(j=0;j<n;j++){
        gains[j] /= min;
      }

      printf("    { ");
      for(j=0;j<n-1;j++){
        printf("%2d, ", quant_index(gains[j]));
      }
      printf("%d },\n", quant_index(gains[j]));
    }
    printf("  },\n");
  }
  printf("};\n"); 
  printf("#endif\n");
  printf("\n"); 




  return 0;
}


void
interleave (int16_t *a, int n)
{
  int i;
  for(i=0;i<n/2;i++){
    tmp2[i*2] = a[i];
    tmp2[i*2 + 1] = a[n/2 + i];
  }
  for(i=0;i<n;i++){
    a[i] = tmp2[i];
  }
}

void
deinterleave (int16_t *a, int n)
{
  int i;
  for(i=0;i<n/2;i++){
    tmp2[i] = a[i*2];
    tmp2[n/2 + i] = a[i*2+1];
  }
  for(i=0;i<n;i++){
    a[i] = tmp2[i];
  }
}

void
split_schro_ext (int16_t *a, int n, int filter)
{
  int16_t tmp1[2000], *hi;
  int16_t tmp2[2000], *lo;

  hi = tmp1 + 4;
  lo = tmp2 + 4;

  oil_deinterleave2_s16 (hi, lo, a, n/2);

  switch (filter) {
    case SCHRO_WAVELET_DESLAURIES_DUBUC_9_7:
      schro_split_ext_desl93 (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_LE_GALL_5_3:
      schro_split_ext_53 (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_DESLAURIES_DUBUC_13_7:
      schro_split_ext_135 (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_HAAR_0:
    case SCHRO_WAVELET_HAAR_1:
      schro_split_ext_haar (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_FIDELITY:
      schro_split_ext_fidelity (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_DAUBECHIES_9_7:
      schro_split_ext_daub97(hi, lo, n/2);
      break;
  }
  oil_interleave2_s16 (a, hi, lo, n/2);

}

void
synth_schro_ext (int16_t *a, int n, int filter)
{
  int16_t tmp1[2000], *hi;
  int16_t tmp2[2000], *lo;

  hi = tmp1 + 4;
  lo = tmp2 + 4;

  oil_deinterleave2_s16 (hi, lo, a, n/2);

  switch (filter) {
    case SCHRO_WAVELET_DESLAURIES_DUBUC_9_7:
      schro_synth_ext_desl93 (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_LE_GALL_5_3:
      schro_synth_ext_53 (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_DESLAURIES_DUBUC_13_7:
      schro_synth_ext_135 (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_HAAR_0:
    case SCHRO_WAVELET_HAAR_1:
      schro_synth_ext_haar (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_FIDELITY:
      schro_synth_ext_fidelity (hi, lo, n/2);
      break;
    case SCHRO_WAVELET_DAUBECHIES_9_7:
      schro_synth_ext_daub97(hi, lo, n/2);
      break;
  }

  oil_interleave2_s16 (a, hi, lo, n/2);
}

