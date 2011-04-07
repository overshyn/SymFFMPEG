
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schro.h>
#include <schroedinger/schrohistogram.h>
#include <liboil/liboil.h>
#include <string.h>
#include <math.h>

//#define DUMP_SUBBAND_CURVES

void schro_encoder_choose_quantisers_simple (SchroEncoderFrame *frame);
void schro_encoder_choose_quantisers_rate_distortion (SchroEncoderFrame *frame);
void schro_encoder_choose_quantisers_lossless (SchroEncoderFrame *frame);
void schro_encoder_choose_quantisers_lowdelay (SchroEncoderFrame *frame);
void schro_encoder_choose_quantisers_constant_lambda (SchroEncoderFrame *frame);
void schro_encoder_choose_quantisers_constant_error (SchroEncoderFrame *frame);

double schro_encoder_entropy_to_lambda (SchroEncoderFrame *frame, double entropy);
static double schro_encoder_lambda_to_entropy (SchroEncoderFrame *frame, double lambda);
static void schro_encoder_generate_subband_histograms (SchroEncoderFrame *frame);


static int schro_subband_pick_quant (SchroEncoderFrame *frame,
    int component, int i, double lambda);

#define CURVE_SIZE 128

double
schro_encoder_perceptual_weight_moo (double cpd)
{
  /* I pretty much pulled this out of my ass (ppd = pixels per degree) */
  if (cpd < 4) return 1;
  return 0.68 * cpd * exp (-0.25 * cpd);
}

double
schro_encoder_perceptual_weight_constant (double cpd)
{
  return 1;
}

double
schro_encoder_perceptual_weight_ccir959 (double cpd)
{
  double w;
  w = 0.255 * pow(1 + 0.2561 * cpd * cpd, -0.75);

  /* return normalized value */
  return w/0.255;
}

double
schro_encoder_perceptual_weight_manos_sakrison (double cpd)
{
  double w;

  if (cpd < 4) return 1;
  w = 2.6 * (0.0192 + 0.114*cpd) * exp(-pow(0.114*cpd,1.1));

  /* return normalized value */
  return w / 0.980779694777866;
}

static double
weighted_sum (const float *h1, const float *v1, double *weight)
{
  int i,j;
  double sum;
  double rowsum;

  sum = 0;
  for(j=0;j<CURVE_SIZE;j++){
    rowsum = 0;
    for(i=0;i<CURVE_SIZE;i++){
      rowsum += h1[i]*v1[j]*weight[CURVE_SIZE*j+i];
    }
    sum += rowsum;
  }
  return sum;
}

static double
dot_product (const float *h1, const float *v1, const float *h2,
    const float *v2, double *weight)
{
  int i,j;
  double sum;
  double rowsum;

  sum = 0;
  for(j=0;j<CURVE_SIZE;j++){
    rowsum = 0;
    for(i=0;i<CURVE_SIZE;i++){
      rowsum += h1[i]*v1[j]*h2[i]*v2[j]*weight[CURVE_SIZE*j+i]*weight[CURVE_SIZE*j+i];
    }
    sum += rowsum;
  }
  return sum;
}

static void
solve (double *matrix, double *column, int n)
{
  int i;
  int j;
  int k;
  double x;

  for(i=0;i<n;i++){
    x = 1/matrix[i*n+i];
    for(k=i;k<n;k++) {
      matrix[i*n+k] *= x;
    }
    column[i] *= x;

    for(j=i+1;j<n;j++){
      x = matrix[j*n+i];
      for(k=i;k<n;k++) {
        matrix[j*n+k] -= matrix[i*n+k] * x;
      }
      column[j] -= column[i] * x;
    }
  }

  for(i=n-1;i>0;i--) {
    for(j=i-1;j>=0;j--) {
      column[j] -= matrix[j*n+i] * column[i];
      matrix[j*n+i] = 0;
    }
  }
}

void
schro_encoder_set_default_subband_weights (SchroEncoder *encoder)
{
  schro_encoder_calculate_subband_weights (encoder,
      schro_encoder_perceptual_weight_constant);
}

void
schro_encoder_calculate_subband_weights (SchroEncoder *encoder,
    double (*perceptual_weight)(double))
{
  int wavelet;
  int n_levels;
  double *matrix;
  int n;
  int i,j;
  double column[SCHRO_LIMIT_SUBBANDS];
  double *weight;

  matrix = schro_malloc (sizeof(double)*SCHRO_LIMIT_SUBBANDS*SCHRO_LIMIT_SUBBANDS);
  weight = schro_malloc (sizeof(double)*CURVE_SIZE*CURVE_SIZE);

  for(j=0;j<CURVE_SIZE;j++){
    for(i=0;i<CURVE_SIZE;i++){
      double fv = j*encoder->cycles_per_degree_vert*(1.0/CURVE_SIZE);
      double fh = i*encoder->cycles_per_degree_horiz*(1.0/CURVE_SIZE);

      weight[j*CURVE_SIZE+i] = perceptual_weight (sqrt(fv*fv+fh*fh));
    }
  }

  for(wavelet=0;wavelet<SCHRO_N_WAVELETS;wavelet++) {
    for(n_levels=1;n_levels<=4;n_levels++){
      const float *h_curve[SCHRO_LIMIT_SUBBANDS];
      const float *v_curve[SCHRO_LIMIT_SUBBANDS];
      int hi[SCHRO_LIMIT_SUBBANDS];
      int vi[SCHRO_LIMIT_SUBBANDS];

      n = 3*n_levels+1;

      for(i=0;i<n;i++){
        int position = schro_subband_get_position(i);
        int n_transforms;

        n_transforms = n_levels - SCHRO_SUBBAND_SHIFT(position);
        if (position&1) {
          hi[i] = (n_transforms-1)*2;
        } else {
          hi[i] = (n_transforms-1)*2+1;
        }
        if (position&2) {
          vi[i] = (n_transforms-1)*2;
        } else {
          vi[i] = (n_transforms-1)*2+1;
        }
        h_curve[i] = schro_tables_wavelet_noise_curve[wavelet][hi[i]];
        v_curve[i] = schro_tables_wavelet_noise_curve[wavelet][vi[i]];
      }

      if (0) {
        for(i=0;i<n;i++){
          column[i] = weighted_sum(h_curve[i], v_curve[i], weight);
          matrix[i*n+i] = dot_product (h_curve[i], v_curve[i],
              h_curve[i], v_curve[i], weight);
          for(j=i+1;j<n;j++) {
            matrix[i*n+j] = dot_product (h_curve[i], v_curve[i],
                h_curve[j], v_curve[j], weight);
            matrix[j*n+i] = matrix[i*n+j];
          }
        }

        solve (matrix, column, n);

        for(i=0;i<n;i++){
          if (column[i] < 0) {
            SCHRO_ERROR("BROKEN wavelet %d n_levels %d", wavelet, n_levels);
            break;
          }
        }

        SCHRO_DEBUG("wavelet %d n_levels %d", wavelet, n_levels);
        for(i=0;i<n;i++){
          SCHRO_DEBUG("%g", 1.0/sqrt(column[i]));
          encoder->subband_weights[wavelet][n_levels-1][i] = sqrt(column[i]);
        }
      } else {
        for(i=0;i<n;i++){
          int position = schro_subband_get_position(i);
          int n_transforms;
          double size;

          n_transforms = n_levels - SCHRO_SUBBAND_SHIFT(position);
          size = (1.0/CURVE_SIZE)*(1<<n_transforms);
          encoder->subband_weights[wavelet][n_levels-1][i] = 1.0/(size *
            sqrt(weighted_sum(h_curve[i], v_curve[i], weight)));
        }
      }
    }
  }

#if 0
  for(wavelet=0;wavelet<8;wavelet++) {
    for(n_levels=1;n_levels<=4;n_levels++){
      double alpha, beta, shift;
      double gain;

      alpha = schro_tables_wavelet_gain[wavelet][0];
      beta = schro_tables_wavelet_gain[wavelet][1];
      shift = (1<<filtershift[wavelet]);

      n = 3*n_levels+1;

      gain = shift;
      for(i=n_levels-1;i>=0;i--){
        encoder->subband_weights[wavelet][n_levels-1][1+3*i+0] =
          sqrt(alpha*beta)*gain;
        encoder->subband_weights[wavelet][n_levels-1][1+3*i+1] =
          sqrt(alpha*beta)*gain;
        encoder->subband_weights[wavelet][n_levels-1][1+3*i+2] =
          sqrt(beta*beta)*gain;
        gain *= alpha;
        gain *= shift;
      }
      encoder->subband_weights[wavelet][n_levels-1][0] = gain / shift;
      if (wavelet == 3 && n_levels == 3) {
        for(i=0;i<10;i++){
          SCHRO_ERROR("%g",
              encoder->subband_weights[wavelet][n_levels-1][i]);
        }
      }
    }
  }
#endif

  schro_free(weight);
  schro_free(matrix);
}

void
schro_encoder_choose_quantisers (SchroEncoderFrame *frame)
{
  switch (frame->encoder->quantiser_engine) {
    case SCHRO_QUANTISER_ENGINE_SIMPLE:
      schro_encoder_choose_quantisers_simple (frame);
      break;
    case SCHRO_QUANTISER_ENGINE_RATE_DISTORTION:
      schro_encoder_choose_quantisers_rate_distortion (frame);
      break;
    case SCHRO_QUANTISER_ENGINE_LOSSLESS:
      schro_encoder_choose_quantisers_lossless (frame);
      break;
    case SCHRO_QUANTISER_ENGINE_LOWDELAY:
      schro_encoder_choose_quantisers_lowdelay (frame);
      break;
    case SCHRO_QUANTISER_ENGINE_CONSTANT_LAMBDA:
      schro_encoder_choose_quantisers_constant_lambda (frame);
      break;
    case SCHRO_QUANTISER_ENGINE_CONSTANT_ERROR:
      schro_encoder_choose_quantisers_constant_error (frame);
      break;
  }
}

void
schro_encoder_choose_quantisers_lossless (SchroEncoderFrame *frame)
{
  int i;
  int component;

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*frame->params.transform_depth; i++) {
      frame->quant_index[component][i] = 0;
    }
  }
}

void
schro_encoder_choose_quantisers_simple (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  double noise_amplitude;
  double a;
  double max;
  double *table;

  noise_amplitude = 255.0 * pow(0.1, frame->encoder->noise_threshold*0.05);
  SCHRO_DEBUG("noise %g", noise_amplitude);

  table = frame->encoder->subband_weights[params->wavelet_filter_index]
    [params->transform_depth-1];

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      a = noise_amplitude *
        frame->encoder->subband_weights[params->wavelet_filter_index]
          [params->transform_depth-1][i];

      frame->quant_index[component][i] = schro_utils_multiplier_to_quant_index (a);
    }
  }

#if 0
  max = table[0];
  for(i=0;i<1 + 3*params->transform_depth; i++) {
    if (table[i] > max) max = table[i];
  }
#else
  max = 1.0;
#endif

  for(i=0;i<1 + 3*params->transform_depth; i++) {
    params->quant_matrix[i] = schro_utils_multiplier_to_quant_index (max/table[i]);
    SCHRO_DEBUG("%g %g %d", table[i], max/table[i], params->quant_matrix[i]);
  }
}

void
schro_encoder_choose_quantisers_lowdelay (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  int base;
  const int *table;

  /* completely made up */
  base = 12 + (30 - frame->encoder->noise_threshold)/2;

  table = schro_tables_lowdelay_quants[params->wavelet_filter_index]
      [params->transform_depth-1];

  for(component=0;component<3;component++){
    frame->quant_index[component][0] = base - table[0];

    for(i=0;i<params->transform_depth; i++) {
      frame->quant_index[component][1+3*i+0] = base - table[1 + 2*i + 0];
      frame->quant_index[component][1+3*i+1] = base - table[1 + 2*i + 0];
      frame->quant_index[component][1+3*i+2] = base - table[1 + 2*i + 1];
    }
  }

}

#ifdef DUMP_SUBBAND_CURVES
static double pow2(double x)
{
  return x*x;
}
#endif

#ifdef DUMP_SUBBAND_CURVES
static double
measure_error_subband (SchroEncoderFrame *frame, int component, int index,
    int quant_index)
{
  int i;
  int j;
  int16_t *data;
  int16_t *line;
  int stride;
  int width;
  int height;
  int skip = 1;
  double error = 0;
  int q;
  int quant_factor;
  int quant_offset;
  int value;
  int position;

  position = schro_subband_get_position (index);
  schro_subband_get (frame->iwt_frame, component, position,
      &frame->params, &data, &stride, &width, &height);

  quant_factor = schro_table_quant[quant_index];
  if (frame->params.num_refs > 0) {
    quant_offset = schro_table_offset_3_8[quant_index];
  } else {
    quant_offset = schro_table_offset_1_2[quant_index];
  }

  error = 0;
  if (index == 0) {
    for(j=0;j<height;j+=skip){
      line = OFFSET(data, j*stride);
      for(i=1;i<width;i+=skip){
        q = schro_quantise(abs(line[i] - line[i-1]), quant_factor, quant_offset);
        value = schro_dequantise(q, quant_factor, quant_offset);
        error += pow2(value - abs(line[i] - line[i-1]));
      }
    }
  } else {
    for(j=0;j<height;j+=skip){
      line = OFFSET(data, j*stride);
      for(i=0;i<width;i+=skip){
        q = schro_quantise(line[i], quant_factor, quant_offset);
        value = schro_dequantise(q, quant_factor, quant_offset);
        error += pow2(value - line[i]);
      }
    }
  }
  error *= skip*skip;

  return error;
}
#endif

typedef struct _ErrorFuncInfo ErrorFuncInfo;
struct _ErrorFuncInfo {
  int quant_factor;
  int quant_offset;
  double power;
};

static double error_pow(int x, void *priv)
{
  ErrorFuncInfo *efi = priv;
  int q;
  int value;
  int y;

  q = schro_quantise (x, efi->quant_factor, efi->quant_offset);
  value = schro_dequantise (q, efi->quant_factor, efi->quant_offset);

  y = abs (value - x);

  return pow (y, efi->power);
}

void
schro_encoder_init_error_tables (SchroEncoder *encoder)
{
  int i;

  for(i=0;i<60;i++){
    ErrorFuncInfo efi;

    efi.quant_factor = schro_table_quant[i];
    efi.quant_offset = schro_table_offset_1_2[i];
    efi.power = encoder->magic_error_power;

    schro_histogram_table_generate (encoder->intra_hist_tables + i,
        error_pow, &efi);
  }

}

#if 0
static double
schro_histogram_estimate_error (SchroHistogram *hist, int quant_index,
    int num_refs)
{
  SchroHistogramTable *table;

  if (num_refs == 0) {
    table = (SchroHistogramTable *)(schro_table_error_hist_shift3_1_2[quant_index]);
  } else {
    /* FIXME the 3/8 table doesn't exist yet */
    //table = (SchroHistogramTable *)(schro_table_error_hist_shift3_3_8[quant_index]);
    table = (SchroHistogramTable *)(schro_table_error_hist_shift3_1_2[quant_index]);
  }
  return schro_histogram_apply_table (hist, table);
}
#endif

#ifdef DUMP_SUBBAND_CURVES
static double
schro_encoder_estimate_subband_arith (SchroEncoderFrame *frame, int component,
    int index, int quant_index)
{
  int i;
  int j;
  int16_t *data;
  int16_t *line;
  int stride;
  int width;
  int height;
  int q;
  int quant_factor;
  int quant_offset;
  int estimated_entropy;
  SchroArith *arith;
  int position;

  arith = schro_arith_new ();
  schro_arith_estimate_init (arith);

  position = schro_subband_get_position (index);
  schro_subband_get (frame->iwt_frame, component, position,
      &frame->params, &data, &stride, &width, &height);

  quant_factor = schro_table_quant[quant_index];
  quant_offset = schro_table_offset_1_2[quant_index];

  if (index == 0) {
    for(j=0;j<height;j++) {
      line = OFFSET(data, j*stride);
      schro_arith_estimate_sint (arith,
          SCHRO_CTX_ZPZN_F1, SCHRO_CTX_COEFF_DATA, SCHRO_CTX_SIGN_ZERO, 0);
      for(i=1;i<width;i++) {
        q = schro_quantise(line[i] - line[i-1], quant_factor, quant_offset);
        schro_arith_estimate_sint (arith,
            SCHRO_CTX_ZPZN_F1, SCHRO_CTX_COEFF_DATA, SCHRO_CTX_SIGN_ZERO, q);
      }
    }
  } else {
    for(j=0;j<height;j++) {
      line = OFFSET(data, j*stride);
      for(i=0;i<width;i++) {
        q = schro_quantise(line[i], quant_factor, quant_offset);
        schro_arith_estimate_sint (arith,
            SCHRO_CTX_ZPZN_F1, SCHRO_CTX_COEFF_DATA, SCHRO_CTX_SIGN_ZERO, q);
      }
    }
  }

  estimated_entropy = 0;

  estimated_entropy += arith->contexts[SCHRO_CTX_ZPZN_F1].n_bits;
  estimated_entropy += arith->contexts[SCHRO_CTX_ZP_F2].n_bits;
  estimated_entropy += arith->contexts[SCHRO_CTX_ZP_F3].n_bits;
  estimated_entropy += arith->contexts[SCHRO_CTX_ZP_F4].n_bits;
  estimated_entropy += arith->contexts[SCHRO_CTX_ZP_F5].n_bits;
  estimated_entropy += arith->contexts[SCHRO_CTX_ZP_F6p].n_bits;

  estimated_entropy += arith->contexts[SCHRO_CTX_COEFF_DATA].n_bits;

  estimated_entropy += arith->contexts[SCHRO_CTX_SIGN_ZERO].n_bits;

  schro_arith_free (arith);

  return estimated_entropy;
}
#endif

static void
schro_encoder_generate_subband_histogram (SchroEncoderFrame *frame,
    int component, int index, SchroHistogram *hist, int skip)
{
  int i;
  int j;
  int16_t *data;
  int16_t *line;
  int stride;
  int width;
  int height;
  int position;

  schro_histogram_init (hist);

  position = schro_subband_get_position (index);
  schro_subband_get (frame->iwt_frame, component, position,
      &frame->params, &data, &stride, &width, &height);

  if (index > 0) {
    for(j=0;j<height;j+=skip){
      schro_histogram_add_array_s16 (hist, OFFSET(data, j*stride), width);
    }
    schro_histogram_scale (hist, skip);
  } else {
    for(j=0;j<height;j+=skip){
      line = OFFSET(data, j*stride);
      for(i=1;i<width;i+=skip){
        schro_histogram_add(hist, (line[i] - line[i-1]));
      }
    }
    schro_histogram_scale (hist, skip*skip);
  }
}

static void
schro_encoder_generate_subband_histograms (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  int pos;
  int skip;

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      pos = schro_subband_get_position (i);
      skip = 1<<MAX(0,SCHRO_SUBBAND_SHIFT(pos)-1);
      schro_encoder_generate_subband_histogram (frame, component, i,
          &frame->subband_hists[component][i], skip);
    }
  }

  frame->have_histograms = TRUE;
}

#ifdef DUMP_SUBBAND_CURVES
static void
schro_encoder_dump_subband_curves (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  int pos;

  SCHRO_ASSERT(frame->have_histograms);

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      int vol;
      int16_t *data;
      int stride;
      int width, height;
      int j;
      SchroHistogram *hist;

      pos = schro_subband_get_position (i);
      schro_subband_get (frame->iwt_frame, component, pos,
          &frame->params, &data, &stride, &width, &height);
      vol = width * height;
      hist = &frame->subband_hists[component][i];

      for(j=0;j<60;j++){
        double est_entropy;
        double error;
        double est_error;
        double arith_entropy;

        error = measure_error_subband (frame, component, i, j);
        est_entropy = schro_histogram_estimate_entropy (
            &frame->subband_hists[component][i], j, params->is_noarith);
        est_error = schro_histogram_apply_table (hist,
            &frame->encoder->intra_hist_tables[j]);
        arith_entropy = schro_encoder_estimate_subband_arith (frame,
            component, i, j);

        schro_dump (SCHRO_DUMP_SUBBAND_CURVE, "%d %d %d %g %g %g %g\n",
            component, i, j, est_entropy/vol, arith_entropy/vol,
            est_error/vol, error/vol);
      }
    }
  }
}
#endif

static void
schro_encoder_calc_estimates (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int j;
  int component;

  SCHRO_ASSERT(frame->have_histograms);

#ifdef DUMP_SUBBAND_CURVES
  schro_encoder_dump_subband_curves (frame);
#endif

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      for(j=0;j<60;j++){
        int vol;
        int16_t *data;
        int stride;
        int width, height;
        int position;
        SchroHistogram *hist;

        position = schro_subband_get_position (i);
        schro_subband_get (frame->iwt_frame, component, position,
            &frame->params, &data, &stride, &width, &height);
        vol = width * height;

        hist = &frame->subband_hists[component][i];
        frame->est_entropy[component][i][j] =
          schro_histogram_estimate_entropy (hist, j, params->is_noarith);
        frame->est_error[component][i][j] = schro_histogram_apply_table (hist,
              &frame->encoder->intra_hist_tables[j]);
      }
    }
  }
  frame->have_estimate_tables = TRUE;
}

void
schro_encoder_choose_quantisers_rate_distortion (SchroEncoderFrame *frame)
{
  //SchroParams *params = &frame->params;
  //int i;
  //int component;
  double base_lambda;
  int bits;
  double ratio;

  schro_encoder_generate_subband_histograms (frame);
  schro_encoder_calc_estimates (frame);

  SCHRO_ASSERT(frame->have_estimate_tables);

  if (frame->num_refs == 0) {
    ratio = frame->encoder->average_arith_context_ratio_intra;
  } else {
    ratio = frame->encoder->average_arith_context_ratio_inter;
  }
  frame->estimated_arith_context_ratio = CLAMP(ratio, 0.5, 1.2);
  bits = frame->allocated_residual_bits;

  base_lambda = schro_encoder_entropy_to_lambda (frame, bits);
#if 0
  if (frame->is_ref) {
    base_lambda = schro_encoder_entropy_to_lambda (frame, bits);
  } else {
    if (frame->num_refs == 0) {
      base_lambda = schro_encoder_entropy_to_lambda (frame, bits);
    } else if (frame->num_refs == 1) {
      if (frame->is_ref) {
        base_lambda = schro_encoder_entropy_to_lambda (frame, bits);
      } else {
        base_lambda = frame->ref_frame0->base_lambda;
      }
    } else {
      base_lambda = 0.5 *
        (frame->ref_frame0->base_lambda + frame->ref_frame1->base_lambda);
    }
    if (!frame->is_ref) {
      base_lambda *= frame->encoder->magic_nonref_lambda_scale;
    }
  }
#endif
  frame->base_lambda = base_lambda;
  SCHRO_DEBUG("LAMBDA: %d %g %d", frame->frame_number, base_lambda, bits);

  schro_encoder_lambda_to_entropy (frame, base_lambda);
#if 0
  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      double lambda;
      double weight;

      lambda = base_lambda;
      if (i == 0) {
        lambda *= frame->encoder->magic_subband0_lambda_scale;
      }
      if (component > 0) {
        lambda *= frame->encoder->magic_chroma_lambda_scale;
      }

      weight = frame->encoder->subband_weights[frame->params.wavelet_filter_index]
        [frame->params.transform_depth-1][i];
      lambda /= weight*weight;
      
      frame->quant_index[component][i] = schro_subband_pick_quant (frame,
          component, i, lambda);
    }
  }
#endif
}

void
schro_encoder_choose_quantisers_constant_lambda (SchroEncoderFrame *frame)
{
  schro_encoder_generate_subband_histograms (frame);
  schro_encoder_calc_estimates (frame);

  SCHRO_ASSERT(frame->have_estimate_tables);

  frame->base_lambda = frame->encoder->magic_lambda;

  schro_encoder_lambda_to_entropy (frame, frame->base_lambda);
}


void
schro_encoder_estimate_entropy (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  int n = 0;

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
#if 0
      n += schro_histogram_estimate_entropy (
            &frame->subband_hists[component][i],
            frame->quant_index[component][i], params->is_noarith);
#else
      n += frame->est_entropy[component][i][frame->quant_index[component][i]];
#endif
    }
  }
  frame->estimated_residual_bits = n * frame->estimated_arith_context_ratio;

  if (frame->allocated_residual_bits > 0 &&
      frame->estimated_residual_bits >
      2 * frame->encoder->bits_per_picture + frame->allocated_residual_bits) {
    SCHRO_WARNING("%d: estimated entropy too big (%d vs %d)",
        frame->frame_number,
        frame->estimated_residual_bits,
        frame->allocated_residual_bits);
  }
}

static int
schro_subband_pick_quant (SchroEncoderFrame *frame, int component, int i,
    double lambda)
{
  double x;
  double min;
  int j;
  int j_min;
  double entropy;
  double error;

  SCHRO_ASSERT(frame->have_estimate_tables);

  j_min = -1;
  min = 0;
  for(j=0;j<60;j++){
    entropy = frame->est_entropy[component][i][j];
    error = frame->est_error[component][i][j];
    
    x = entropy + lambda * error;
    if (j == 0 || x < min) {
      j_min = j;
      min = x;
    }
  }

  return j_min;
}

#if 0
void
schro_encoder_rate_distortion_test (SchroEncoderFrame *frame)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  int j;
  double lambda_mult;
  double n;
  double base_lambda;
  int qsum;

base_lambda = 0.1;
  schro_encoder_generate_subband_histograms (frame);
  schro_encoder_calc_estimates (frame);

  for(j=0;j<40;j++){
    lambda_mult = pow(1.1, j-20);
    n = 0;
    qsum = 0;

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      int vol;
      int16_t *data;
      int stride;
      int width, height;
      double lambda;
      int position;
      double weight;
      int quant_index;

      position = schro_subband_get_position (i);
      schro_subband_get (frame->iwt_frame, component, position,
          &frame->params, &data, &stride, &width, &height);
      vol = width * height;

      lambda = base_lambda;
      lambda *= lambda_mult;
      if (i == 0) {
        //lambda *= 10;
      }
#if 0
      if (component > 0) {
        lambda *= 0.3;
      }
#endif
      if (frame->is_ref) {
        lambda *= 10;
      }

      weight = frame->encoder->subband_weights[frame->params.wavelet_filter_index]
        [frame->params.transform_depth-1][i];
      lambda /= weight*weight;
      
      quant_index = schro_subband_pick_quant (frame, component, i, lambda);
      n += frame->est_entropy[component][i][quant_index];
      qsum += quant_index;
    }
  }
    schro_dump (SCHRO_DUMP_LAMBDA_CURVE, "%d %g %g %d",
        j, lambda_mult * base_lambda, n, qsum);
  }
}
#endif

static double
schro_encoder_lambda_to_entropy (SchroEncoderFrame *frame, double base_lambda)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  double entropy = 0;

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      double lambda;
      double weight;
      int quant_index;

      lambda = base_lambda;

      if (i == 0) {
        lambda *= frame->encoder->magic_subband0_lambda_scale;
      }
      if (component > 0) {
        lambda *= frame->encoder->magic_chroma_lambda_scale;
      }

      weight = frame->encoder->subband_weights[frame->params.wavelet_filter_index]
        [frame->params.transform_depth-1][i];
      lambda /= weight*weight;
      
      quant_index = schro_subband_pick_quant (frame, component, i, lambda);
      entropy += frame->est_entropy[component][i][quant_index];
      frame->quant_index[component][i] = quant_index;
    }
  }

  return entropy * frame->estimated_arith_context_ratio;
}

double
schro_encoder_entropy_to_lambda (SchroEncoderFrame *frame, double entropy)
{
  int j;
  double log_lambda_hi, log_lambda_lo, log_lambda_mid;
  double entropy_hi, entropy_lo, entropy_mid;

  log_lambda_hi = log(1);
  entropy_hi = schro_encoder_lambda_to_entropy (frame, exp(log_lambda_hi));
  SCHRO_DEBUG("start target=%g log_lambda=%g entropy=%g",
      entropy, log_lambda_hi, entropy_hi, log_lambda_hi, entropy);

  if (entropy_hi < entropy) {
    entropy_lo = entropy_hi;
    log_lambda_lo = log_lambda_hi;

    for(j=0;j<5;j++) {
      log_lambda_hi = log_lambda_lo + log(100);
      entropy_hi = schro_encoder_lambda_to_entropy (frame, exp(log_lambda_hi));

      SCHRO_DEBUG("have: log_lambda=[%g,%g] entropy=[%g,%g] target=%g",
          log_lambda_lo, log_lambda_hi, entropy_lo, entropy_hi, entropy);
      if (entropy_hi > entropy) break;

      SCHRO_DEBUG("--> step up");

      entropy_lo = entropy_hi;
      log_lambda_lo = log_lambda_hi;
    }
    SCHRO_DEBUG("--> stopping");
  } else {
    for(j=0;j<5;j++) {
      log_lambda_lo = log_lambda_hi - log(100);
      entropy_lo = schro_encoder_lambda_to_entropy (frame, exp(log_lambda_lo));

      SCHRO_DEBUG("have: log_lambda=[%g,%g] entropy=[%g,%g] target=%g",
          log_lambda_lo, log_lambda_hi, entropy_lo, entropy_hi, entropy);

      SCHRO_DEBUG("--> step down");
      if (entropy_lo < entropy) break;

      entropy_hi = entropy_lo;
      log_lambda_hi = log_lambda_lo;
    }
    SCHRO_DEBUG("--> stopping");
  }
  if (entropy_lo == entropy_hi) {
    return exp(0.5*(log_lambda_lo + log_lambda_hi));
  }

  if (entropy_lo > entropy || entropy_hi < entropy) {
    SCHRO_ERROR("entropy not bracketed");
  }

  for(j=0;j<14;j++){
    double x;

    if (entropy_hi == entropy_lo) break;

    SCHRO_DEBUG("have: log_lambda=[%g,%g] entropy=[%g,%g] target=%g",
        log_lambda_lo, log_lambda_hi, entropy_lo, entropy_hi, entropy);

#if 0
    x = (entropy - entropy_lo) / (entropy_hi - entropy_lo);
    if (x < 0.2) x = 0.2;
    if (x > 0.8) x = 0.8;
#else
    x = 0.5;
#endif
    log_lambda_mid = log_lambda_lo + (log_lambda_hi - log_lambda_lo) * x;
    entropy_mid = schro_encoder_lambda_to_entropy (frame, exp(log_lambda_mid));

    SCHRO_DEBUG("picking x=%g log_lambda_mid=%g entropy=%g", x,
        log_lambda_mid, entropy_mid);

    if (entropy_mid > entropy) {
      log_lambda_hi = log_lambda_mid;
      entropy_hi = entropy_mid;
      SCHRO_DEBUG("--> focus up");
    } else {
      log_lambda_lo = log_lambda_mid;
      entropy_lo = entropy_mid;
      SCHRO_DEBUG("--> focus down");
    }
  }

  log_lambda_mid = 0.5*(log_lambda_hi + log_lambda_lo);
  SCHRO_DEBUG("done %g", exp(log_lambda_mid));
  return exp(log_lambda_mid);
}

static double
schro_encoder_lambda_to_error (SchroEncoderFrame *frame, double base_lambda)
{
  SchroParams *params = &frame->params;
  int i;
  int component;
  double error = 0;

  for(component=0;component<3;component++){
    for(i=0;i<1 + 3*params->transform_depth; i++) {
      double lambda;
      double weight;
      int quant_index;

      lambda = base_lambda;

      if (i == 0) {
        lambda *= frame->encoder->magic_subband0_lambda_scale;
      }
      if (component > 0) {
        lambda *= frame->encoder->magic_chroma_lambda_scale;
      }

      weight = frame->encoder->subband_weights[frame->params.wavelet_filter_index]
        [frame->params.transform_depth-1][i];
      lambda /= weight*weight;
      
      quant_index = schro_subband_pick_quant (frame, component, i, lambda);
      error += frame->est_error[component][i][quant_index];
      frame->quant_index[component][i] = quant_index;
    }
  }

  return error;
}

double
schro_encoder_error_to_lambda (SchroEncoderFrame *frame, double error)
{
  int j;
  double log_lambda_hi, log_lambda_lo, log_lambda_mid;
  double error_hi, error_lo, error_mid;

  log_lambda_lo = log(1);
  error_lo = schro_encoder_lambda_to_error (frame, exp(log_lambda_lo));
  SCHRO_DEBUG("start target=%g log_lambda=%g error=%g",
      error, log_lambda_lo, error_lo, log_lambda_lo, error);

  if (error < error_lo) {
    error_hi = error_lo;
    log_lambda_hi = log_lambda_lo;

    for(j=0;j<5;j++) {
      log_lambda_lo = log_lambda_hi + log(100);
      error_lo = schro_encoder_lambda_to_error (frame, exp(log_lambda_lo));

      SCHRO_DEBUG("have: log_lambda=[%g,%g] error=[%g,%g] target=%g",
          log_lambda_lo, log_lambda_hi, error_lo, error_hi, error);
      if (error > error_lo) break;

      SCHRO_DEBUG("--> step up");

      error_hi = error_lo;
      log_lambda_hi = log_lambda_lo;
    }
    SCHRO_DEBUG("--> stopping");
  } else {
    for(j=0;j<5;j++) {
      log_lambda_hi = log_lambda_lo - log(100);
      error_hi = schro_encoder_lambda_to_error (frame, exp(log_lambda_hi));

      SCHRO_DEBUG("have: log_lambda=[%g,%g] error=[%g,%g] target=%g",
          log_lambda_lo, log_lambda_hi, error_lo, error_hi, error);

      SCHRO_DEBUG("--> step down");
      if (error < error_hi) break;

      error_lo = error_hi;
      log_lambda_lo = log_lambda_hi;
    }
    SCHRO_DEBUG("--> stopping");
  }
  if (error_lo == error_hi) {
    return exp(0.5*(log_lambda_lo + log_lambda_hi));
  }

  if (error_lo > error || error_hi < error) {
    SCHRO_DEBUG("error not bracketed");
  }

  for(j=0;j<14;j++){
    double x;

    if (error_hi == error_lo) break;

    SCHRO_DEBUG("have: log_lambda=[%g,%g] error=[%g,%g] target=%g",
        log_lambda_lo, log_lambda_hi, error_lo, error_hi, error);

#if 0
    x = (error - error_lo) / (error_hi - error_lo);
    if (x < 0.2) x = 0.2;
    if (x > 0.8) x = 0.8;
#else
    x = 0.5;
#endif
    log_lambda_mid = log_lambda_lo + (log_lambda_hi - log_lambda_lo) * x;
    error_mid = schro_encoder_lambda_to_error (frame, exp(log_lambda_mid));

    SCHRO_DEBUG("picking x=%g log_lambda_mid=%g error=%g", x,
        log_lambda_mid, error_mid);

    if (error_mid > error) {
      log_lambda_hi = log_lambda_mid;
      error_hi = error_mid;
      SCHRO_DEBUG("--> focus up");
    } else {
      log_lambda_lo = log_lambda_mid;
      error_lo = error_mid;
      SCHRO_DEBUG("--> focus down");
    }
  }

  log_lambda_mid = 0.5*(log_lambda_hi + log_lambda_lo);
  SCHRO_DEBUG("done %g", exp(log_lambda_mid));
  return exp(log_lambda_mid);
}

void
schro_encoder_choose_quantisers_constant_error (SchroEncoderFrame *frame)
{
  double base_lambda;
  double error;

  schro_encoder_generate_subband_histograms (frame);
  schro_encoder_calc_estimates (frame);

  SCHRO_ASSERT(frame->have_estimate_tables);

  error = 255.0 * pow(0.1, frame->encoder->noise_threshold*0.05);
  error *= frame->params.video_format->width *
    frame->params.video_format->height;

  base_lambda = schro_encoder_error_to_lambda (frame, error);

  frame->base_lambda = base_lambda;
  SCHRO_DEBUG("LAMBDA: %d %g", frame->frame_number, base_lambda);
}

