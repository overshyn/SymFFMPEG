
#ifndef __SCHRO_DECODER_H__
#define __SCHRO_DECODER_H__

#include <schroedinger/schrobuffer.h>
#include <schroedinger/schroparams.h>
#include <schroedinger/schroframe.h>
#include <schroedinger/schromotion.h>
#include <schroedinger/schrounpack.h>
#include <schroedinger/schrobitstream.h>
#include <schroedinger/schroqueue.h>
#include <schroedinger/schroasync.h>
#include <schroedinger/schroarith.h>

SCHRO_BEGIN_DECLS

typedef struct _SchroDecoder SchroDecoder;
typedef struct _SchroPicture SchroPicture;

#ifdef SCHRO_ENABLE_UNSTABLE_API
struct _SchroDecoder {
  /*< private >*/

  /* the list of reference pictures */
  SchroQueue *reference_queue;

  /* a list of frames provided by the app that we'll decode into */
  SchroQueue *output_queue;

  SchroMemoryDomain *cpu_domain;
  SchroMemoryDomain *cuda_domain;
  SchroAsync *async;
  int use_cuda;

  SchroBuffer *input_buffer;

  SchroPictureNumber next_frame_number;

  SchroPicture *picture;

  int major_version;
  int minor_version;
  int profile;
  int level;
  SchroVideoFormat video_format;

  SchroQueue *frame_queue;
  SchroQueue *picture_queue;

  int queue_depth;
  int end_of_stream;
  int flushing;
  int coded_order;

  SchroPictureNumber earliest_frame;

  SchroUnpack unpack;
  int parse_code;
  int next_parse_offset;
  int prev_parse_offset;

  int have_sequence_header;
  SchroBuffer *sequence_header_buffer;
  int have_frame_number;

  double skip_value;
  double skip_ratio;

  int error;
  char *error_message;

  int has_md5;
  uint8_t md5_checksum[32];
};

struct _SchroPicture {
  int refcount;

  SchroDecoder *decoder;

  unsigned int state;
  unsigned int needed_state;
  unsigned int working;
  int busy;
  int skip;
  int error;

  SchroBuffer *input_buffer;
  SchroParams params;
  SchroPictureNumber picture_number;
  SchroPictureNumber reference1;
  SchroPictureNumber reference2;
  SchroPictureNumber retired_picture_number;
  SchroPicture *ref0;
  SchroPicture *ref1;
  SchroFrame *planar_output_frame;

  int is_ref;

  int16_t *tmpbuf;

  int zero_residual;

  SchroFrame *transform_frame;
  SchroFrame *frame;
  SchroFrame *mc_tmp_frame;
  SchroMotion *motion;
  SchroFrame *output_picture;
  SchroUpsampledFrame *upsampled_frame;

  int subband_length[3][SCHRO_LIMIT_SUBBANDS];
  int subband_quant_index[3][SCHRO_LIMIT_SUBBANDS];
  SchroBuffer *subband_buffer[3][SCHRO_LIMIT_SUBBANDS];
  SchroFrameData subband_data[3][SCHRO_LIMIT_SUBBANDS];

  SchroBuffer *motion_buffers[9];

  SchroBuffer *lowdelay_buffer;

  int has_md5;
  uint8_t md5_checksum[32];
};
#endif

enum {
  SCHRO_DECODER_OK,
  SCHRO_DECODER_ERROR,
  SCHRO_DECODER_EOS,
  SCHRO_DECODER_FIRST_ACCESS_UNIT,
  SCHRO_DECODER_NEED_BITS,
  SCHRO_DECODER_NEED_FRAME,
  SCHRO_DECODER_WAIT,
  SCHRO_DECODER_STALLED
};

enum {
  SCHRO_DECODER_PICTURE_ORDER_PRESENTATION = 0,
  SCHRO_DECODER_PICTURE_ORDER_CODED
};

SchroDecoder * schro_decoder_new (void);
void schro_decoder_free (SchroDecoder *decoder);
void schro_decoder_reset (SchroDecoder *decoder);
SchroVideoFormat * schro_decoder_get_video_format (SchroDecoder *decoder);
void schro_decoder_add_output_picture (SchroDecoder *decoder, SchroFrame *frame);
int schro_decoder_push_ready (SchroDecoder *decoder);
int schro_decoder_push (SchroDecoder *decoder, SchroBuffer *buffer);
int schro_decoder_set_flushing (SchroDecoder *decoder, int flushing);
void schro_decoder_set_picture_order (SchroDecoder *decoder, int picture_order);
int schro_decoder_push_end_of_stream (SchroDecoder *decoder);
SchroFrame *schro_decoder_pull (SchroDecoder *decoder);
int schro_decoder_wait (SchroDecoder *decoder);

void schro_decoder_set_earliest_frame (SchroDecoder *decoder, SchroPictureNumber earliest_frame);
void schro_decoder_set_skip_ratio (SchroDecoder *decoder, double ratio);
SchroPictureNumber schro_decoder_get_picture_number (SchroDecoder *decoder);
int schro_decoder_need_output_frame (SchroDecoder *decoder);

#ifdef SCHRO_ENABLE_UNSTABLE_API

void schro_decoder_decode_parse_header (SchroDecoder *decoder);
void schro_decoder_parse_sequence_header (SchroDecoder *decoder);
int schro_decoder_compare_sequence_header_buffer (SchroBuffer *a, SchroBuffer *b);

void schro_decoder_subband_dc_predict (SchroFrameData *fd);

/* SchroPicture */

SchroPicture * schro_picture_new (SchroDecoder *decoder);
SchroPicture * schro_picture_ref (SchroPicture *picture);
void schro_picture_unref (SchroPicture *picture);

void schro_decoder_decode_picture (SchroPicture *picture);
void schro_decoder_x_check_references (SchroPicture *picture);
void schro_decoder_x_decode_motion (SchroPicture *picture);
void schro_decoder_x_render_motion (SchroPicture *picture);
void schro_decoder_x_decode_residual (SchroPicture *picture);
void schro_decoder_x_wavelet_transform (SchroPicture *picture);
void schro_decoder_x_combine (SchroPicture *picture);
void schro_decoder_x_upsample (SchroPicture *picture);

int schro_decoder_iterate_picture (SchroDecoder *decoder);
void schro_decoder_parse_picture (SchroPicture *picture);
void schro_decoder_parse_picture_header (SchroPicture *picture);
void schro_decoder_parse_picture_prediction_parameters (SchroPicture *picture);
void schro_decoder_parse_block_data (SchroPicture *picture);
void schro_decoder_parse_transform_parameters (SchroPicture *picture);
void schro_decoder_parse_transform_data (SchroPicture *picture);
void schro_decoder_parse_lowdelay_transform_data (SchroPicture *picture);
void schro_decoder_init_subband_frame_data_interleaved (SchroPicture *picture);
void schro_decoder_decode_block_data (SchroPicture *picture);
void schro_decoder_decode_transform_data (SchroPicture *picture);
void schro_decoder_decode_lowdelay_transform_data (SchroPicture *picture);
void schro_decoder_decode_macroblock(SchroPicture *picture,
    SchroArith **arith, SchroUnpack *unpack, int i, int j);
void schro_decoder_decode_prediction_unit(SchroPicture *picture,
    SchroArith **arith, SchroUnpack *unpack, SchroMotionVector *motion_vectors,
    int x, int y);


#endif

SCHRO_END_DECLS

#endif

