
#ifndef __SCHRO_PHASECORRELATION_H__
#define __SCHRO_PHASECORRELATION_H__

#include <schroedinger/schroencoder.h>
#include <schroedinger/schromotionest.h>

SCHRO_BEGIN_DECLS

#ifdef SCHRO_ENABLE_UNSTABLE_API

void schro_encoder_phasecorr_estimation (SchroMotionEst *me);

#endif

SCHRO_END_DECLS

#endif

