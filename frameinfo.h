/* frameinfo.h */

#ifndef FRAMEINFO_H_
#define FRAMEINFO_H_

#include "yuv4mpeg2.h"

#define FRAMEINFO_FIELDS \
  X(average) \
  X(min)     \
  X(max)     \
  X(rms)     \
  X(energy)

#define X(x) double x;
typedef struct {
  FRAMEINFO_FIELDS
} frameinfo;
#undef X

y4m2_output *frameinfo_filter(y4m2_output *next);
y4m2_output *frameinfo_grapher(y4m2_output *next, const char *note, const char *field);

frameinfo *frameinfo_get(const y4m2_frame *frame, const char *name);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
