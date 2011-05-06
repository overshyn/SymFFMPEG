
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <schroedinger/schro.h>
#include <schroedinger/schrodebug.h>

#include <liboil/liboilrandom.h>

static void
frame_free (SchroFrame *frame, void *priv)
{
  free (priv);
}

static void
test (int w, int h)
{
  int size;
  uint8_t *picture;
  SchroEncoder *encoder;
  SchroBuffer *buffer;
  SchroFrame *frame;
  SchroVideoFormat *format;
  int n_frames;
  int go;

  encoder = schro_encoder_new();
  format = schro_encoder_get_video_format(encoder);
  format->width = w;
  format->height = h;
  schro_encoder_set_video_format (encoder, format);
  free (format);
  schro_encoder_start (encoder);

  size = ROUND_UP_4 (w) * ROUND_UP_2 (h);
  size += (ROUND_UP_8 (w)/2) * (ROUND_UP_2 (h)/2);
  size += (ROUND_UP_8 (w)/2) * (ROUND_UP_2 (h)/2);

  n_frames = 0;
  go = 1;
  while (go) {
    int x;

    switch (schro_encoder_wait (encoder)) {
      case SCHRO_STATE_NEED_FRAME:
        if (n_frames < 100) {
          //SCHRO_ERROR("frame %d", n_frames);

          picture = malloc(size);
          //oil_random_u8(picture, size);
          memset (picture, 128, size);

          frame = schro_frame_new_from_data_I420 (picture, w, h);

          schro_frame_set_free_callback (frame, frame_free, picture);

          schro_encoder_push_frame (encoder, frame);

          n_frames++;
        } else {
          schro_encoder_end_of_stream (encoder);
        }
        break;
      case SCHRO_STATE_HAVE_BUFFER:
        buffer = schro_encoder_pull (encoder, &x);
        //printf("%d\n", x);
        schro_buffer_unref (buffer);
        break;
      case SCHRO_STATE_AGAIN:
        break;
      case SCHRO_STATE_END_OF_STREAM:
        go = 0;
        break;
      default:
        break;
    }
  }

  schro_encoder_free (encoder);
}

int
main (int argc, char *argv[])
{
  int h, w;

  schro_init();

  test(128,128);
  if (0) {
    for(w=64;w<64+16;w++){
      for(h=64;h<64+16;h++){
        test(w,h);
      }
    }
  }

  return 0;
}


