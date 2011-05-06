/*
 * Image Scaling Functions
 * Copyright (c) 2005 David A. Schleef <ds@schleef.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "vs_scanline.h"

#include <liboil/liboil.h>

/* greyscale, i.e., single componenet */

void
vs_scanline_downsample_Y (uint8_t * dest, uint8_t * src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = (src[i * 2] + src[i * 2 + 1]) / 2;
  }
}

void
vs_scanline_resample_nearest_Y (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i] = (x < 32768) ? src[j] : src[j + 1];

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_Y (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  uint32_t vals[2];

  vals[0] = *accumulator;
  vals[1] = increment;

  oil_resample_linear_u8 (dest, src, n, vals);

  *accumulator = vals[0];
}

void
vs_scanline_merge_linear_Y (uint8_t * dest, uint8_t * src1, uint8_t * src2,
    int n, int x)
{
  uint32_t value = x >> 8;

  oil_merge_linear_u8 (dest, src1, src2, &value, n);
}


/* RGBA */

void
vs_scanline_downsample_RGBA (uint8_t * dest, uint8_t * src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i * 4 + 0] = (src[i * 8 + 0] + src[i * 8 + 4]) / 2;
    dest[i * 4 + 1] = (src[i * 8 + 1] + src[i * 8 + 5]) / 2;
    dest[i * 4 + 2] = (src[i * 8 + 2] + src[i * 8 + 6]) / 2;
    dest[i * 4 + 3] = (src[i * 8 + 3] + src[i * 8 + 7]) / 2;
  }
}

void
vs_scanline_resample_nearest_RGBA (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 0] = (x < 32768) ? src[j * 4 + 0] : src[j * 4 + 4];
    dest[i * 4 + 1] = (x < 32768) ? src[j * 4 + 1] : src[j * 4 + 5];
    dest[i * 4 + 2] = (x < 32768) ? src[j * 4 + 2] : src[j * 4 + 6];
    dest[i * 4 + 3] = (x < 32768) ? src[j * 4 + 3] : src[j * 4 + 7];

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_RGBA (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  uint32_t vals[2];

  vals[0] = *accumulator;
  vals[1] = increment;

  oil_resample_linear_argb ((uint32_t *) dest, (uint32_t *) src, n, vals);

  *accumulator = vals[0];
}

void
vs_scanline_merge_linear_RGBA (uint8_t * dest, uint8_t * src1, uint8_t * src2,
    int n, int x)
{
  uint32_t value = x >> 8;

  oil_merge_linear_argb ((uint32_t *) dest, (uint32_t *) src1, (uint32_t *) src2,
      &value, n);
}


/* RGB */

void
vs_scanline_downsample_RGB (uint8_t * dest, uint8_t * src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i * 3 + 0] = (src[i * 6 + 0] + src[i * 6 + 3]) / 2;
    dest[i * 3 + 1] = (src[i * 6 + 1] + src[i * 6 + 4]) / 2;
    dest[i * 3 + 2] = (src[i * 6 + 2] + src[i * 6 + 5]) / 2;
  }
}

void
vs_scanline_resample_nearest_RGB (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 3 + 0] = (x < 32768) ? src[j * 3 + 0] : src[j * 3 + 3];
    dest[i * 3 + 1] = (x < 32768) ? src[j * 3 + 1] : src[j * 3 + 4];
    dest[i * 3 + 2] = (x < 32768) ? src[j * 3 + 2] : src[j * 3 + 5];

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_RGB (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 3 + 0] = (src[j * 3 + 0] * (65536 - x) + src[j * 3 + 3] * x) >> 16;
    dest[i * 3 + 1] = (src[j * 3 + 1] * (65536 - x) + src[j * 3 + 4] * x) >> 16;
    dest[i * 3 + 2] = (src[j * 3 + 2] * (65536 - x) + src[j * 3 + 5] * x) >> 16;

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_merge_linear_RGB (uint8_t * dest, uint8_t * src1, uint8_t * src2,
    int n, int x)
{
  uint32_t value = x >> 8;

  oil_merge_linear_u8 (dest, src1, src2, &value, n * 3);
}


/* YUYV */

/* n is the number of bi-pixels */
/* increment is per Y pixel */

void
vs_scanline_downsample_YUYV (uint8_t * dest, uint8_t * src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i * 4 + 0] = (src[i * 8 + 0] + src[i * 8 + 2]) / 2;
    dest[i * 4 + 1] = (src[i * 8 + 1] + src[i * 8 + 5]) / 2;
    dest[i * 4 + 2] = (src[i * 8 + 4] + src[i * 8 + 6]) / 2;
    dest[i * 4 + 3] = (src[i * 8 + 3] + src[i * 8 + 7]) / 2;
  }
}

void
vs_scanline_resample_nearest_YUYV (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 0] = (x < 32768) ? src[j * 2 + 0] : src[j * 2 + 2];

    j = acc >> 17;
    x = acc & 0x1ffff;
    dest[i * 4 + 1] = (x < 65536) ? src[j * 4 + 1] : src[j * 4 + 5];
    dest[i * 4 + 3] = (x < 65536) ? src[j * 4 + 3] : src[j * 4 + 7];

    acc += increment;

    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 2] = (x < 32768) ? src[j * 2 + 0] : src[j * 2 + 2];
    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_YUYV (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 0] = (src[j * 2 + 0] * (65536 - x) + src[j * 2 + 2] * x) >> 16;

    j = acc >> 17;
    x = acc & 0x1ffff;
    dest[i * 4 + 1] =
        (src[j * 4 + 1] * (131072 - x) + src[j * 4 + 5] * x) >> 17;
    dest[i * 4 + 3] =
        (src[j * 4 + 3] * (131072 - x) + src[j * 4 + 7] * x) >> 17;

    acc += increment;

    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 2] = (src[j * 2 + 0] * (65536 - x) + src[j * 2 + 2] * x) >> 16;
    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_merge_linear_YUYV (uint8_t * dest, uint8_t * src1, uint8_t * src2,
    int n, int x)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i * 4 + 0] =
        (src1[i * 4 + 0] * (65536 - x) + src2[i * 4 + 0] * x) >> 16;
    dest[i * 4 + 1] =
        (src1[i * 4 + 1] * (65536 - x) + src2[i * 4 + 1] * x) >> 16;
    dest[i * 4 + 2] =
        (src1[i * 4 + 2] * (65536 - x) + src2[i * 4 + 2] * x) >> 16;
    dest[i * 4 + 3] =
        (src1[i * 4 + 3] * (65536 - x) + src2[i * 4 + 3] * x) >> 16;
  }
}


/* UYVY */

/* n is the number of bi-pixels */
/* increment is per Y pixel */

void
vs_scanline_downsample_UYVY (uint8_t * dest, uint8_t * src, int n)
{
  int i;

  for (i = 0; i < n; i++) {
    dest[i * 4 + 0] = (src[i * 8 + 0] + src[i * 8 + 4]) / 2;
    dest[i * 4 + 1] = (src[i * 8 + 1] + src[i * 8 + 3]) / 2;
    dest[i * 4 + 2] = (src[i * 8 + 2] + src[i * 8 + 6]) / 2;
    dest[i * 4 + 3] = (src[i * 8 + 5] + src[i * 8 + 7]) / 2;
  }
}

void
vs_scanline_resample_nearest_UYVY (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 1] = (x < 32768) ? src[j * 2 + 1] : src[j * 2 + 3];

    j = acc >> 17;
    x = acc & 0x1ffff;
    dest[i * 4 + 0] = (x < 65536) ? src[j * 4 + 0] : src[j * 4 + 4];
    dest[i * 4 + 2] = (x < 65536) ? src[j * 4 + 2] : src[j * 4 + 6];

    acc += increment;

    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 3] = (x < 32768) ? src[j * 2 + 1] : src[j * 2 + 3];
    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_UYVY (uint8_t * dest, uint8_t * src, int n,
    int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 1] = (src[j * 2 + 1] * (65536 - x) + src[j * 2 + 3] * x) >> 16;

    j = acc >> 17;
    x = acc & 0x1ffff;
    dest[i * 4 + 0] =
        (src[j * 4 + 0] * (131072 - x) + src[j * 4 + 4] * x) >> 17;
    dest[i * 4 + 2] =
        (src[j * 4 + 2] * (131072 - x) + src[j * 4 + 6] * x) >> 17;

    acc += increment;

    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 3] = (src[j * 2 + 1] * (65536 - x) + src[j * 2 + 3] * x) >> 16;
    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_merge_linear_UYVY (uint8_t * dest, uint8_t * src1, uint8_t * src2,
    int n, int x)
{
  uint32_t value = x >> 8;

  oil_merge_linear_argb ((uint32_t *) dest, (uint32_t *) src1, (uint32_t *) src2,
      &value, n);
}


/* RGB565 */

/* note that src and dest are uint16_t, and thus endian dependent */

#define RGB565_R(x) (((x)&0xf800)>>8 | ((x)&0xf800)>>13)
#define RGB565_G(x) (((x)&0x07e0)>>3 | ((x)&0x07e0)>>9)
#define RGB565_B(x) (((x)&0x001f)<<3 | ((x)&0x001f)>>2)

#define RGB565(r,g,b) \
  ((((r)<<8)&0xf800) | (((g)<<3)&0x07e0) | (((b)>>3)&0x001f))


void
vs_scanline_downsample_RGB565 (uint8_t * dest_u8, uint8_t * src_u8, int n)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src = (uint16_t *) src_u8;
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = RGB565 (
        (RGB565_R (src[i * 2]) + RGB565_R (src[i * 2 + 1])) / 2,
        (RGB565_G (src[i * 2]) + RGB565_G (src[i * 2 + 1])) / 2,
        (RGB565_B (src[i * 2]) + RGB565_B (src[i * 2 + 1])) / 2);
  }
}

void
vs_scanline_resample_nearest_RGB565 (uint8_t * dest_u8, uint8_t * src_u8, int n,
    int *accumulator, int increment)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src = (uint16_t *) src_u8;
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i] = (x < 32768) ? src[j] : src[j + 1];

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_RGB565 (uint8_t * dest_u8, uint8_t * src_u8, int n,
    int *accumulator, int increment)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src = (uint16_t *) src_u8;
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i] = RGB565 (
        (RGB565_R (src[j]) * (65536 - x) + RGB565_R (src[j + 1]) * x) >> 16,
        (RGB565_G (src[j]) * (65536 - x) + RGB565_G (src[j + 1]) * x) >> 16,
        (RGB565_B (src[j]) * (65536 - x) + RGB565_B (src[j + 1]) * x) >> 16);

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_merge_linear_RGB565 (uint8_t * dest_u8, uint8_t * src1_u8,
    uint8_t * src2_u8, int n, int x)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src1 = (uint16_t *) src1_u8;
  uint16_t *src2 = (uint16_t *) src2_u8;
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = RGB565 (
        (RGB565_R (src1[i]) * (65536 - x) + RGB565_R (src2[i]) * x) >> 16,
        (RGB565_G (src1[i]) * (65536 - x) + RGB565_G (src2[i]) * x) >> 16,
        (RGB565_B (src1[i]) * (65536 - x) + RGB565_B (src2[i]) * x) >> 16);
  }
}


/* RGB555 */

/* note that src and dest are uint16_t, and thus endian dependent */

#define RGB555_R(x) (((x)&0x7c00)>>8 | ((x)&0x7c00)>>13)
#define RGB555_G(x) (((x)&0x03e0)>>3 | ((x)&0x03e0)>>9)
#define RGB555_B(x) (((x)&0x001f)<<3 | ((x)&0x001f)>>2)

#define RGB555(r,g,b) \
  ((((r)<<7)&0x7c00) | (((g)<<3)&0x03e0) | (((b)>>3)&0x001f))


void
vs_scanline_downsample_RGB555 (uint8_t * dest_u8, uint8_t * src_u8, int n)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src = (uint16_t *) src_u8;
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = RGB555 (
        (RGB555_R (src[i * 2]) + RGB555_R (src[i * 2 + 1])) / 2,
        (RGB555_G (src[i * 2]) + RGB555_G (src[i * 2 + 1])) / 2,
        (RGB555_B (src[i * 2]) + RGB555_B (src[i * 2 + 1])) / 2);
  }
}

void
vs_scanline_resample_nearest_RGB555 (uint8_t * dest_u8, uint8_t * src_u8, int n,
    int *accumulator, int increment)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src = (uint16_t *) src_u8;
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i] = (x < 32768) ? src[j] : src[j + 1];

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_resample_linear_RGB555 (uint8_t * dest_u8, uint8_t * src_u8, int n,
    int *accumulator, int increment)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src = (uint16_t *) src_u8;
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i] = RGB555 (
        (RGB555_R (src[j]) * (65536 - x) + RGB555_R (src[j + 1]) * x) >> 16,
        (RGB555_G (src[j]) * (65536 - x) + RGB555_G (src[j + 1]) * x) >> 16,
        (RGB555_B (src[j]) * (65536 - x) + RGB555_B (src[j + 1]) * x) >> 16);

    acc += increment;
  }

  *accumulator = acc;
}

void
vs_scanline_merge_linear_RGB555 (uint8_t * dest_u8, uint8_t * src1_u8,
    uint8_t * src2_u8, int n, int x)
{
  uint16_t *dest = (uint16_t *) dest_u8;
  uint16_t *src1 = (uint16_t *) src1_u8;
  uint16_t *src2 = (uint16_t *) src2_u8;
  int i;

  for (i = 0; i < n; i++) {
    dest[i] = RGB555 (
        (RGB555_R (src1[i]) * (65536 - x) + RGB555_R (src2[i]) * x) >> 16,
        (RGB555_G (src1[i]) * (65536 - x) + RGB555_G (src2[i]) * x) >> 16,
        (RGB555_B (src1[i]) * (65536 - x) + RGB555_B (src2[i]) * x) >> 16);
  }
}
