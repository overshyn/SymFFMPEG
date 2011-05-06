
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <schroedinger/schro.h>
#include <liboil/liboil.h>
#include <stdlib.h>
#include <schroedinger/schrocuda.h>
#ifdef HAVE_ORC
#include <orc/orc.h>
#endif

extern int _schro_decode_prediction_only;
extern int _schro_dump_enable;
extern int _schro_motion_ref;

/**
 * schro_init:
 *
 * Intializes the Schroedinger library.  This function must be called
 * before any other function in the library.
 */
void
schro_init(void)
{
  const char *s;
  static int inited = FALSE;

  if (inited) return;

  inited = TRUE;

  oil_init();
#ifdef HAVE_ORC
  orc_init();
#endif

  s = getenv ("SCHRO_DEBUG");
  if (s && s[0]) {
    char *end;
    int level;

    level = strtoul (s, &end, 0);
    if (end[0] == 0) {
      schro_debug_set_level (level);
    }
  }

  s = getenv ("SCHRO_DECODE_PREDICTION_ONLY");
  if (s && s[0]) {
    _schro_decode_prediction_only = TRUE;
  }

  s = getenv ("SCHRO_MOTION_REF");
  if (s && s[0]) {
    _schro_motion_ref = TRUE;
  }

  s = getenv ("SCHRO_DUMP");
  if (s && s[0]) {
    _schro_dump_enable = TRUE;
  }

  schro_async_init ();
#ifdef HAVE_CUDA
  schro_cuda_init ();
#endif
}

