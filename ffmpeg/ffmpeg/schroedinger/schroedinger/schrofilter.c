
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schrofilter.h>
#include <schroedinger/schrodebug.h>
#include <schroedinger/schrowavelet.h>
#include <schroedinger/schrotables.h>
#include <schroedinger/schrobitstream.h>
#include <schroedinger/schrohistogram.h>
#include <schroedinger/schroparams.h>
#include <schroedinger/schrooil.h>

#include <liboil/liboil.h>
#include <string.h>
#include <math.h>

static void
sort_u8 (uint8_t *d, int n)
{
  int start = 0;
  int end = n;
  int i;
  int x;

  /* OMG bubble sort! */
  while(start < end) {
    for(i=start;i<end-1;i++){
      if (d[i] > d[i+1]) {
        x = d[i];
        d[i] = d[i+1];
        d[i+1] = x;
      }
    }
    end--;
    for(i=end-2;i>=start;i--){
      if (d[i] > d[i+1]) {
        x = d[i];
        d[i] = d[i+1];
        d[i+1] = x;
      }
    }
    start++;
  }
}

/* reference */
void
schro_filter_cwmN_ref (uint8_t *d, uint8_t *s1, uint8_t *s2, uint8_t *s3, int n, int weight)
{
  int i;
  int j;
  uint8_t list[8+12];

  for(i=0;i<n;i++){
    list[0] = s1[i+0];
    list[1] = s1[i+1];
    list[2] = s1[i+2];
    list[3] = s2[i+0];
    list[4] = s2[i+2];
    list[5] = s3[i+0];
    list[6] = s3[i+1];
    list[7] = s3[i+2];
    for(j=0;j<weight;j++){
      list[8+j] = s2[i+1];
    }

    sort_u8 (list, 8+weight);

    d[i] = list[(8+weight)/2];
  }
}

void
schro_filter_cwmN (uint8_t *d, uint8_t *s1, uint8_t *s2, uint8_t *s3, int n, int weight)
{
  int i;
  int j;
  uint8_t list[8+12];
  int low, hi;

  for(i=0;i<n;i++){
    list[0] = s1[i+0];
    list[1] = s1[i+1];
    list[2] = s1[i+2];
    list[3] = s2[i+0];
    list[4] = s2[i+2];
    list[5] = s3[i+0];
    list[6] = s3[i+1];
    list[7] = s3[i+2];

    low = 0;
    hi = 0;
    for(j=0;j<8;j++){
      if (list[j] < s2[i+1]) low++;
      if (list[j] > s2[i+1]) hi++;
    }

    if (low < ((9-weight)/2) || hi < ((9-weight)/2)) {
      for(j=0;j<weight;j++){
        list[8+j] = s2[i+1];
      }

      sort_u8 (list, 8+weight);

      d[i] = list[(8+weight)/2];
    } else {
      d[i] = s2[i+1];
    }
  }
}

static void
schro_frame_component_filter_cwmN (SchroFrameData *comp, int weight)
{
  int i;
  uint8_t *tmp;
  uint8_t *tmp1;
  uint8_t *tmp2;

  tmp1 = schro_malloc(comp->width);
  tmp2 = schro_malloc(comp->width);

  schro_filter_cwmN (tmp1,
      OFFSET(comp->data, comp->stride * 0),
      OFFSET(comp->data, comp->stride * 1),
      OFFSET(comp->data, comp->stride * 2), comp->width - 2, weight);
  schro_filter_cwmN (tmp2,
      OFFSET(comp->data, comp->stride * 1),
      OFFSET(comp->data, comp->stride * 2),
      OFFSET(comp->data, comp->stride * 3), comp->width - 2, weight);

  for(i=3;i<comp->height - 1;i++) {
    memcpy (OFFSET(comp->data, comp->stride * (i-2) + 1),
        tmp1, comp->width - 2);
    tmp = tmp1; tmp1 = tmp2; tmp2 = tmp;

    schro_filter_cwmN (tmp2,
        OFFSET(comp->data, comp->stride * (i-1)),
        OFFSET(comp->data, comp->stride * (i+0)),
        OFFSET(comp->data, comp->stride * (i+1)), comp->width - 2, weight);
  }
  memcpy (OFFSET(comp->data, comp->stride * (i-2) + 1),
      tmp1, comp->width - 2);
  memcpy (OFFSET(comp->data, comp->stride * (i-1) + 1),
      tmp2, comp->width - 2);

  schro_free (tmp1);
  schro_free (tmp2);
}

void
schro_frame_filter_cwmN (SchroFrame *frame, int weight)
{
  schro_frame_component_filter_cwmN (&frame->components[0], weight);
  schro_frame_component_filter_cwmN (&frame->components[1], weight);
  schro_frame_component_filter_cwmN (&frame->components[2], weight);
}


static void
schro_frame_component_filter_cwmN_ref (SchroFrameData *comp, int weight)
{
  int i;
  uint8_t *tmp;
  uint8_t *tmp1;
  uint8_t *tmp2;

  tmp1 = schro_malloc(comp->width);
  tmp2 = schro_malloc(comp->width);

  schro_filter_cwmN_ref (tmp1,
      OFFSET(comp->data, comp->stride * 0),
      OFFSET(comp->data, comp->stride * 1),
      OFFSET(comp->data, comp->stride * 2), comp->width - 2, weight);
  schro_filter_cwmN_ref (tmp2,
      OFFSET(comp->data, comp->stride * 1),
      OFFSET(comp->data, comp->stride * 2),
      OFFSET(comp->data, comp->stride * 3), comp->width - 2, weight);

  for(i=3;i<comp->height - 1;i++) {
    memcpy (OFFSET(comp->data, comp->stride * (i-2) + 1),
        tmp1, comp->width - 2);
    tmp = tmp1; tmp1 = tmp2; tmp2 = tmp;

    schro_filter_cwmN_ref (tmp2,
        OFFSET(comp->data, comp->stride * (i-1)),
        OFFSET(comp->data, comp->stride * (i+0)),
        OFFSET(comp->data, comp->stride * (i+1)), comp->width - 2, weight);
  }
  memcpy (OFFSET(comp->data, comp->stride * (i-2) + 1),
      tmp1, comp->width - 2);
  memcpy (OFFSET(comp->data, comp->stride * (i-1) + 1),
      tmp2, comp->width - 2);

  schro_free (tmp1);
  schro_free (tmp2);
}

void
schro_frame_filter_cwmN_ref (SchroFrame *frame, int weight)
{
  schro_frame_component_filter_cwmN_ref (&frame->components[0], weight);
  schro_frame_component_filter_cwmN_ref (&frame->components[1], weight);
  schro_frame_component_filter_cwmN_ref (&frame->components[2], weight);
}


#if 0
/* reference */
void
schro_filter_cwm7 (uint8_t *d, uint8_t *s1, uint8_t *s2, uint8_t *s3, int n)
{
  int i;
  int min, max;

  for(i=0;i<n;i++){
    min = MIN(s1[i+0],s1[i+1]);
    max = MAX(s1[i+0],s1[i+1]);
    min = MIN(min,s1[i+2]);
    max = MAX(max,s1[i+2]);
    min = MIN(min,s2[i+0]);
    max = MAX(max,s2[i+0]);
    min = MIN(min,s2[i+2]);
    max = MAX(max,s2[i+2]);
    min = MIN(min,s3[i+0]);
    max = MAX(max,s3[i+0]);
    min = MIN(min,s3[i+1]);
    max = MAX(max,s3[i+1]);
    min = MIN(min,s3[i+2]);
    max = MAX(max,s3[i+2]);

    d[i] = MIN(max,MAX(min,s2[i+1]));
  }
}
#endif

/* FIXME move to schrooil */
void
schro_filter_cwm7 (uint8_t *d, uint8_t *s1, uint8_t *s2, uint8_t *s3, int n)
{
  int i;
  int min, max;

  for(i=0;i<n;i++){
    if (s1[i+0] < s2[i+1]) {
      max = MAX(s1[i+0],s1[i+1]);
      max = MAX(max,s1[i+2]);
      max = MAX(max,s2[i+0]);
      max = MAX(max,s2[i+2]);
      max = MAX(max,s3[i+0]);
      max = MAX(max,s3[i+1]);
      max = MAX(max,s3[i+2]);
      d[i] = MIN(max,s2[i+1]);
    } else if (s1[i+0] > s2[i+1]) {
      min = MIN(s1[i+0],s1[i+1]);
      min = MIN(min,s1[i+2]);
      min = MIN(min,s2[i+0]);
      min = MIN(min,s2[i+2]);
      min = MIN(min,s3[i+0]);
      min = MIN(min,s3[i+1]);
      min = MIN(min,s3[i+2]);
      d[i] = MAX(min,s2[i+1]);
    } else {
      d[i] = s2[i+1];
    }
  }
}

static void
schro_frame_component_filter_cwm7 (SchroFrameData *comp)
{
  int i;
  uint8_t *tmp;
  uint8_t *tmp1;
  uint8_t *tmp2;

  tmp1 = schro_malloc(comp->width);
  tmp2 = schro_malloc(comp->width);

  schro_filter_cwm7 (tmp1,
      OFFSET(comp->data, comp->stride * 0),
      OFFSET(comp->data, comp->stride * 1),
      OFFSET(comp->data, comp->stride * 2), comp->width - 2);
  schro_filter_cwm7 (tmp2,
      OFFSET(comp->data, comp->stride * 1),
      OFFSET(comp->data, comp->stride * 2),
      OFFSET(comp->data, comp->stride * 3), comp->width - 2);

  for(i=3;i<comp->height - 1;i++) {
    memcpy (OFFSET(comp->data, comp->stride * (i-2) + 1),
        tmp1, comp->width - 2);
    tmp = tmp1; tmp1 = tmp2; tmp2 = tmp;

    schro_filter_cwm7 (tmp2,
        OFFSET(comp->data, comp->stride * (i-1)),
        OFFSET(comp->data, comp->stride * (i+0)),
        OFFSET(comp->data, comp->stride * (i+1)), comp->width - 2);
  }
  memcpy (OFFSET(comp->data, comp->stride * (i-2) + 1),
      tmp1, comp->width - 2);
  memcpy (OFFSET(comp->data, comp->stride * (i-1) + 1),
      tmp2, comp->width - 2);

  schro_free (tmp1);
  schro_free (tmp2);
}

void
schro_frame_filter_cwm7 (SchroFrame *frame)
{
  schro_frame_component_filter_cwm7 (&frame->components[0]);
  schro_frame_component_filter_cwm7 (&frame->components[1]);
  schro_frame_component_filter_cwm7 (&frame->components[2]);
}



static void
lowpass_u8 (uint8_t *d, uint8_t *s, int n)
{
  int i;
  int j;
  int x;
  const int16_t taps[] = { 2, 9, 28, 55, 68, 55, 28, 9, 2, 0 };
  const int16_t offsetshift[] = { 128, 8 };

  for(i=0;i<4;i++){
    x = 0;
    for(j=0;j<9;j++) {
      x += s[CLAMP(i+j-4,0,n-1)]*taps[j];
    }
    d[i] = (x + 128)>>8;
  }
  oil_mas10_u8 (d+4, s, taps, offsetshift, n - 9);
  for(i=n-6;i<n;i++){
    x = 0;
    for(j=0;j<9;j++) {
      x += s[CLAMP(i+j-4,0,n-1)]*taps[j];
    }
    d[i] = (x + 128)>>8;
  }
}

/* FIXME move to schrooil */
static void
lowpass_vert_u8 (uint8_t *d, uint8_t *s, int n)
{
  int i;
  int j;
  int x;
  static int taps[] = { 2, 9, 28, 55, 68, 55, 28, 9, 2, 0 };

  for(i=0;i<n;i++){
    x = 0;
    for(j=0;j<9;j++) {
      x += s[j*n + i]*taps[j];
    }
    d[i] = (x + 128)>>8;
  }
}


static void
schro_frame_component_filter_lowpass (SchroFrameData *comp)
{
  int i;
  uint8_t *tmp;

  tmp = schro_malloc(comp->width*9);

  lowpass_u8 (tmp + 0*comp->width,
      OFFSET(comp->data, comp->stride * 0), comp->width);
  memcpy (tmp + 1*comp->width, tmp + 0*comp->width, comp->width);
  memcpy (tmp + 2*comp->width, tmp + 0*comp->width, comp->width);
  memcpy (tmp + 3*comp->width, tmp + 0*comp->width, comp->width);
  memcpy (tmp + 4*comp->width, tmp + 0*comp->width, comp->width);
  lowpass_u8 (tmp + 5*comp->width,
      OFFSET(comp->data, comp->stride * 1), comp->width);
  lowpass_u8 (tmp + 6*comp->width,
      OFFSET(comp->data, comp->stride * 2), comp->width);
  lowpass_u8 (tmp + 7*comp->width,
      OFFSET(comp->data, comp->stride * 2), comp->width);
  for(i=0;i<comp->height;i++){
    lowpass_u8 (tmp + 8*comp->width,
        OFFSET(comp->data, comp->stride * CLAMP(i+4,0,comp->height-1)),
        comp->width);
    lowpass_vert_u8 (OFFSET(comp->data, comp->stride * i),
        tmp, comp->width);
    memmove (tmp, tmp + comp->width * 1, comp->width * 8);
  }

  schro_free (tmp);
}

void
schro_frame_filter_lowpass (SchroFrame *frame)
{
  schro_frame_component_filter_lowpass (&frame->components[0]);
  schro_frame_component_filter_lowpass (&frame->components[1]);
  schro_frame_component_filter_lowpass (&frame->components[2]);
}



static void
lowpass_s16 (int16_t *d, int16_t *s, int n)
{
  int i;
  int j;
  int x;
  const int32_t taps[] = { 2, 9, 28, 55, 68, 55, 28, 9, 2, 0 };
  const int32_t offsetshift[] = { 128, 8 };

  for(i=0;i<4;i++){
    x = 0;
    for(j=0;j<9;j++) {
      x += s[CLAMP(i+j-4,0,n-1)]*taps[j];
    }
    d[i] = (x + 128)>>8;
  }
  oil_mas10_s16 (d+4, s, taps, offsetshift, n - 9);
  for(i=n-6;i<n;i++){
    x = 0;
    for(j=0;j<9;j++) {
      x += s[CLAMP(i+j-4,0,n-1)]*taps[j];
    }
    d[i] = (x + 128)>>8;
  }
}

/* FIXME move to schrooil */
static void
lowpass_vert_s16 (int16_t *d, int16_t *s, int n)
{
  int i;
  int j;
  int x;
  static int taps[] = { 2, 9, 28, 55, 68, 55, 28, 9, 2, 0 };

  for(i=0;i<n;i++){
    x = 0;
    for(j=0;j<9;j++) {
      x += s[j*n + i]*taps[j];
    }
    d[i] = (x + 128)>>8;
  }
}


static void
schro_frame_component_filter_lowpass_16 (SchroFrameData *comp)
{
  int i;
  int16_t *tmp;

  tmp = schro_malloc(comp->width*9*sizeof(int16_t));

  lowpass_s16 (tmp + 0*comp->width,
      OFFSET(comp->data, comp->stride * 0), comp->width);
  memcpy (tmp + 1*comp->width, tmp + 0*comp->width, comp->width*2);
  memcpy (tmp + 2*comp->width, tmp + 0*comp->width, comp->width*2);
  memcpy (tmp + 3*comp->width, tmp + 0*comp->width, comp->width*2);
  memcpy (tmp + 4*comp->width, tmp + 0*comp->width, comp->width*2);
  lowpass_s16 (tmp + 5*comp->width,
      OFFSET(comp->data, comp->stride * 1), comp->width);
  lowpass_s16 (tmp + 6*comp->width,
      OFFSET(comp->data, comp->stride * 2), comp->width);
  lowpass_s16 (tmp + 7*comp->width,
      OFFSET(comp->data, comp->stride * 3), comp->width);
  for(i=0;i<comp->height;i++){
    lowpass_s16 (tmp + 8*comp->width,
        OFFSET(comp->data, comp->stride * CLAMP(i+4,0,comp->height-1)),
        comp->width);
    lowpass_vert_s16 (OFFSET(comp->data, comp->stride * i),
        tmp, comp->width);
    memmove (tmp, tmp + comp->width * 1,
        comp->width * 8 * sizeof(int16_t));
  }

  schro_free (tmp);
}

void
schro_frame_filter_lowpass_16 (SchroFrame *frame)
{
  schro_frame_component_filter_lowpass_16 (&frame->components[0]);
  schro_frame_component_filter_lowpass_16 (&frame->components[1]);
  schro_frame_component_filter_lowpass_16 (&frame->components[2]);
}


static void
lowpass2_u8 (uint8_t *d, uint8_t *s, double *coeff, int n)
{
  double state[3];

  state[0] = s[0];
  state[1] = s[0];
  state[2] = s[0];
  oil_iir3_u8_f64 (d, s, state, coeff, n);

  state[0] = d[n-1];
  state[1] = d[n-1];
  state[2] = d[n-1];
  oil_iir3_rev_u8_f64 (d, s, state, coeff, n);
}

static void
lowpass2_s16 (int16_t *d, int16_t *s, double *coeff, int n)
{
  double state[3];

  state[0] = s[0];
  state[1] = s[0];
  state[2] = s[0];
  oil_iir3_s16_f64 (d, s, state, coeff, n);

  state[0] = d[n-1];
  state[1] = d[n-1];
  state[2] = d[n-1];
  oil_iir3_rev_s16_f64 (d, s, state, coeff, n);
}

static void
generate_coeff (double *coeff, double sigma)
{
  double q;
  double b0, b0inv, b1, b2, b3, B;
  
  if (sigma >= 2.5) {
    q = 0.98711 * sigma - 0.96330;
  } else { 
    q = 3.97156 - 4.41554 * sqrt (1 - 0.26891 * sigma);
  } 
  
  b0 = 1.57825 + 2.44413*q + 1.4281*q*q + 0.422205*q*q*q;
  b0inv = 1.0/b0;
  b1 = 2.44413*q + 2.85619*q*q + 1.26661*q*q*q;
  b2 = -1.4281*q*q - 1.26661*q*q*q;
  b3 = 0.422205*q*q*q;
  B = 1 - (b1 + b2 + b3)/b0;
  
  coeff[0] = B;
  coeff[1] = b1 * b0inv;
  coeff[2] = b2 * b0inv;
  coeff[3] = b3 * b0inv;
}

static void
schro_frame_component_filter_lowpass2_u8 (SchroFrameData *comp,
    double h_sigma, double v_sigma)
{
  int i;
  double h_coeff[4];
  double v_coeff[4];
  double *i1, *i2, *i3;

  generate_coeff (h_coeff, h_sigma);
  generate_coeff (v_coeff, v_sigma);

  i1 = schro_malloc (sizeof(double)*comp->width);
  i2 = schro_malloc (sizeof(double)*comp->width);
  i3 = schro_malloc (sizeof(double)*comp->width);

  for(i=0;i<comp->height;i++){
    lowpass2_u8 (OFFSET(comp->data, comp->stride * i),
        OFFSET(comp->data, comp->stride * i), h_coeff, comp->width);
  }

  oil_convert_f64_u8 (i1, OFFSET(comp->data, comp->stride * 0), comp->width);
  memcpy (i2, i1, sizeof(double)*comp->width);
  memcpy (i3, i1, sizeof(double)*comp->width);
  for(i=0;i<comp->height;i++){
    oil_iir3_across_u8_f64 (
        OFFSET(comp->data, comp->stride * i),
        OFFSET(comp->data, comp->stride * i),
        i1, i2, i3, v_coeff, comp->width);
  }

  oil_convert_f64_u8 (i1,OFFSET(comp->data, comp->stride * (comp->height-1)),
      comp->width);
  memcpy (i2, i1, sizeof(double)*comp->width);
  memcpy (i3, i1, sizeof(double)*comp->width);
  for(i=comp->height-1;i>=0;i--){
    oil_iir3_across_u8_f64 (
        OFFSET(comp->data, comp->stride * i),
        OFFSET(comp->data, comp->stride * i),
        i1, i2, i3, v_coeff, comp->width);
  }

  schro_free (i1);
  schro_free (i2);
  schro_free (i3);
}

static void
schro_frame_component_filter_lowpass2_s16 (SchroFrameData *comp,
    double h_sigma, double v_sigma)
{
  int i;
  double h_coeff[4];
  double v_coeff[4];
  double *i1, *i2, *i3;

  generate_coeff (h_coeff, h_sigma);
  generate_coeff (v_coeff, v_sigma);

  i1 = schro_malloc (sizeof(double)*comp->width);
  i2 = schro_malloc (sizeof(double)*comp->width);
  i3 = schro_malloc (sizeof(double)*comp->width);

  for(i=0;i<comp->height;i++){
    lowpass2_s16 (OFFSET(comp->data, comp->stride * i),
        OFFSET(comp->data, comp->stride * i), h_coeff, comp->width);
  }

  oil_convert_f64_s16 (i1, OFFSET(comp->data, comp->stride * 0), comp->width);
  memcpy (i2, i1, sizeof(double)*comp->width);
  memcpy (i3, i1, sizeof(double)*comp->width);
  for(i=0;i<comp->height;i++){
    oil_iir3_across_s16_f64 (
        OFFSET(comp->data, comp->stride * i),
        OFFSET(comp->data, comp->stride * i),
        i1, i2, i3, v_coeff, comp->width);
  }

  oil_convert_f64_s16 (i1,OFFSET(comp->data, comp->stride * (comp->height-1)),
      comp->width);
  memcpy (i2, i1, sizeof(double)*comp->width);
  memcpy (i3, i1, sizeof(double)*comp->width);
  for(i=comp->height-1;i>=0;i--){
    oil_iir3_across_s16_f64 (
        OFFSET(comp->data, comp->stride * i),
        OFFSET(comp->data, comp->stride * i),
        i1, i2, i3, v_coeff, comp->width);
  }



  schro_free (i1);
  schro_free (i2);
  schro_free (i3);
}

void
schro_frame_filter_lowpass2 (SchroFrame *frame, double sigma)
{
  double chroma_sigma_h;
  double chroma_sigma_v;

  chroma_sigma_h = sigma / (1<<SCHRO_FRAME_FORMAT_H_SHIFT(frame->format));
  chroma_sigma_v = sigma / (1<<SCHRO_FRAME_FORMAT_V_SHIFT(frame->format));

  switch (SCHRO_FRAME_FORMAT_DEPTH(frame->format)) {
    case SCHRO_FRAME_FORMAT_DEPTH_U8:
      schro_frame_component_filter_lowpass2_u8 (&frame->components[0], sigma, sigma);
      schro_frame_component_filter_lowpass2_u8 (&frame->components[1], chroma_sigma_h, chroma_sigma_v);
      schro_frame_component_filter_lowpass2_u8 (&frame->components[2], chroma_sigma_h, chroma_sigma_v);
      break;
    case SCHRO_FRAME_FORMAT_DEPTH_S16:
      schro_frame_component_filter_lowpass2_s16 (&frame->components[0], sigma, sigma);
      schro_frame_component_filter_lowpass2_s16 (&frame->components[1], chroma_sigma_h, chroma_sigma_v);
      schro_frame_component_filter_lowpass2_s16 (&frame->components[2], chroma_sigma_h, chroma_sigma_v);
      break;
    default:
      SCHRO_ASSERT(0);
      break;
  }
}


void
schro_frame_filter_wavelet (SchroFrame *frame)
{
  SchroFrame *tmpframe;
  SchroFrameData *comp;
  SchroHistogram hist;
  int component;
  int16_t *tmp;
  SchroParams params;
  int i;

  tmp = schro_malloc(2*frame->width*sizeof(int16_t));

  tmpframe = schro_frame_new_and_alloc (NULL,
      SCHRO_FRAME_FORMAT_S16_444 | frame->format,
      ROUND_UP_POW2(frame->width,5), ROUND_UP_POW2(frame->height,5));
  schro_frame_convert (tmpframe, frame);

  params.transform_depth = 1;
  params.iwt_luma_width = frame->width;
  params.iwt_luma_height = frame->height;
  params.iwt_chroma_width = frame->components[1].width;
  params.iwt_chroma_height = frame->components[1].height;

  for(component=0;component<3;component++){
    comp = &tmpframe->components[component];

    schro_wavelet_transform_2d (comp, SCHRO_WAVELET_LE_GALL_5_3, tmp);

    for(i=1;i<4;i++){
      int width;
      int height;
      int stride;
      int16_t *data;
      int y;
      int cutoff;

      schro_subband_get (tmpframe, component, i, &params, &data, &stride,
          &width, &height);
      schro_histogram_init (&hist);

      for(y=0;y<height;y++){
        schro_histogram_add_array_s16 (&hist, OFFSET(data, y*stride),
            width);
      }

      cutoff = 100;
      for(y=0;y<height;y++){
        int16_t *line = OFFSET(data, stride*y);
        int x;
        for(x=0;x<width;x++){
          if (line[x] > -cutoff && line[x] < cutoff) line[x] = 0;
        }
      }
    }

    schro_wavelet_inverse_transform_2d (comp, SCHRO_WAVELET_LE_GALL_5_3, tmp);
  }

  schro_frame_convert (frame, tmpframe);
  schro_frame_unref (tmpframe);
}


static double
random_std (void)
{
  double x;
  double y;

  while (1) {
    x = -5.0 + rand () * (1.0/RAND_MAX) * 10;
    y = rand () * (1.0/RAND_MAX);

    if (y < exp(-x*x*0.5)) return x;
  }
}

static void
addnoise_u8 (uint8_t *dest, int n, double sigma)
{
  int i;
  int x;

  for(i=0;i<n;i++){
    x = rint(random_std() * sigma) + dest[i];
    dest[i] = CLAMP(x,0,255);
  }
}

static void
schro_frame_component_filter_addnoise (SchroFrameData *comp, double sigma)
{
  int i;

  for(i=0;i<comp->height;i++){
    addnoise_u8 ( OFFSET(comp->data, comp->stride * i), comp->width, sigma);
  }
}

void
schro_frame_filter_addnoise (SchroFrame *frame, double sigma)
{
  schro_frame_component_filter_addnoise (&frame->components[0], sigma);
  schro_frame_component_filter_addnoise (&frame->components[1], sigma);
  schro_frame_component_filter_addnoise (&frame->components[2], sigma);
}


static int
ilogx_size (int i)
{
  if (i < (1<<SCHRO_HISTOGRAM_SHIFT)) return 1;
  return 1 << ((i>>SCHRO_HISTOGRAM_SHIFT)-1);
}

static int
iexpx (int x)
{
  if (x < (1<<SCHRO_HISTOGRAM_SHIFT)) return x;

  return ((1<<SCHRO_HISTOGRAM_SHIFT)|(x&((1<<SCHRO_HISTOGRAM_SHIFT)-1))) << ((x>>SCHRO_HISTOGRAM_SHIFT)-1);
}


void
schro_frame_filter_adaptive_lowpass (SchroFrame *frame)
{
  SchroHistogram hist;
  double slope;
  SchroFrame *tmp;
  int16_t tmpdata[2048];
  double sigma;
  int j;
  int i;

  tmp = schro_frame_new_and_alloc (NULL,
      SCHRO_FRAME_FORMAT_S16_444 | frame->format,
      frame->width, frame->height);
  schro_frame_convert (tmp, frame);

  schro_wavelet_transform_2d (&tmp->components[0], SCHRO_WAVELET_LE_GALL_5_3,
      tmpdata);

  schro_histogram_init (&hist);
  for(j=0;j<tmp->height/2;j++){
    schro_histogram_add_array_s16 (&hist,
        OFFSET(tmp->components[0].data, tmp->components[0].stride * (2*j+1)),
        tmp->width/2);
  }

  schro_frame_unref(tmp);

  slope = schro_histogram_estimate_slope (&hist);

  for(i=0;i<SCHRO_HISTOGRAM_SIZE;i++) {
    schro_dump (SCHRO_DUMP_HIST_TEST, "%d %g\n",
        iexpx(i), hist.bins[i]/ilogx_size(i));
  }

  /* good for 2 Mb DVD intra-only rip */
  sigma = -1.0/slope;
  if (sigma > 1.0) {
    SCHRO_ERROR("enabling filtering (slope %g)", slope);

    schro_frame_filter_lowpass2 (frame, sigma);
  }
}



