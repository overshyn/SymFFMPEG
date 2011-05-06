
#ifndef __SCHRO_MOTIONEST_H__
#define __SCHRO_MOTIONEST_H__

#include <schroedinger/schroencoder.h>

SCHRO_BEGIN_DECLS

typedef struct _SchroMotionEst SchroMotionEst;
typedef struct _SchroBlock SchroBlock;

#ifdef SCHRO_ENABLE_UNSTABLE_API

struct _SchroMotionEst {
  SchroEncoderFrame *encoder_frame;
  SchroParams *params;

  double lambda;

  SchroUpsampledFrame *src0;
  SchroFrame *downsampled_src0[5];
  SchroUpsampledFrame *src1;
  SchroFrame *downsampled_src1[5];

  SchroMotion *motion;

  SchroBlock *sblocks;

  SchroMotionField *downsampled_mf[2][5];

  int badblocks;
  double hier_score;
};

struct _SchroBlock {
  int valid;
  int error;
  int entropy;

  double score;

  SchroMotionVector mv[4][4];
};

SchroMotionEst *schro_motionest_new (SchroEncoderFrame *frame);
void schro_motionest_free (SchroMotionEst *me);


void schro_encoder_motion_predict (SchroEncoderFrame *frame);

void schro_encoder_global_estimation (SchroMotionEst *me);

SchroMotionField * schro_motion_field_new (int x_num_blocks, int y_num_blocks);
void schro_motion_field_free (SchroMotionField *field);
void schro_motion_field_scan (SchroMotionField *field, SchroParams *params, SchroFrame *frame, SchroFrame *ref, int dist);
void schro_motion_field_inherit (SchroMotionField *field, SchroMotionField *parent);
void schro_motion_field_copy (SchroMotionField *field, SchroMotionField *parent);
void schro_motion_field_global_estimation (SchroMotionField *mf,
    SchroGlobalMotion *gm, int mv_precision);

int schro_frame_get_metric (SchroFrame *frame1, int x1, int y1,
    SchroFrame *frame2, int x2, int y2);
void schro_motion_field_lshift (SchroMotionField *mf, int n);

int schro_motion_estimate_entropy (SchroMotion *motion);
int schro_motion_block_estimate_entropy (SchroMotion *motion, int i, int j);
int schro_motion_superblock_estimate_entropy (SchroMotion *motion, int i, int j);
int schro_motion_superblock_try_estimate_entropy (SchroMotion *motion, int i,
    int j, SchroBlock *block);
int schro_motionest_superblock_get_metric (SchroMotionEst *me,
    SchroBlock *block, int i, int j);
void schro_motion_copy_from (SchroMotion *motion, int i, int j, SchroBlock *block);
void schro_motion_copy_to (SchroMotion *motion, int i, int j, SchroBlock *block);

void schro_block_fixup (SchroBlock *block);
int schro_block_check (SchroBlock *block);

#endif

SCHRO_END_DECLS

#endif

