
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schro.h>
#include <schroedinger/schrowavelet.h>
#include <schroedinger/schrooil.h>

#define OIL_ENABLE_UNSTABLE_API
#include <liboil/liboil.h>
#include <liboil/liboilprofile.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define WIDTH 2400

int16_t tmp[WIDTH+100];

int oil_profile_get_min (OilProfile *prof)
{
  int i;
  int min;
  min = prof->hist_time[0];
  for(i=0;i<10;i++){
    if (prof->hist_count[i] > 0) {
      if (prof->hist_time[i] < min) {
        min = prof->hist_time[i];
      }
    }
  }
  return min;
}

void
wavelet_speed (SchroFrame *frame, int filter)
{
  OilProfile prof1;
  OilProfile prof2;
  double ave_fwd, ave_rev;
  int i;
  SchroFrameData *fd = frame->components + 0;

  oil_profile_init (&prof1);
  oil_profile_init (&prof2);

  for(i=0;i<10;i++){
    oil_profile_start (&prof1);
    schro_wavelet_transform_2d (fd, filter, tmp);
    oil_profile_stop (&prof1);

    oil_profile_start (&prof2);
    schro_wavelet_inverse_transform_2d (fd, filter, tmp);
    oil_profile_stop (&prof2);
  }

  //oil_profile_get_ave_std (&prof1, &ave_fwd, &std);
  //printf("fwd %g (%g)\n", ave, std);

  //oil_profile_get_ave_std (&prof2, &ave_rev, &std);
  //printf("rev %g (%g)\n", ave, std);

  ave_fwd = oil_profile_get_min (&prof1);
  ave_rev = oil_profile_get_min (&prof2);
  printf("%d %d %g %g %g %g\n", frame->width, frame->height, ave_fwd, ave_rev,
      ave_fwd/(frame->width*frame->height), ave_rev/(frame->width*frame->height));
}


int
main (int argc, char *argv[])
{
  int i;
  SchroFrame *frame;
  int width, height;

  width = 1920;
  height = 1080;

  oil_init();

  frame = schro_frame_new_and_alloc (NULL, SCHRO_FRAME_FORMAT_S16_444,
      width, height);

  for(i=0;i<7;i++) {
    wavelet_speed (frame, i);
  }

  schro_frame_unref (frame);

  return 0;
}

