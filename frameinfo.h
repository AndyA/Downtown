/* frameinfo.h */

#ifndef FRAMEINFO_H_
#define FRAMEINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

  typedef struct {
    double average;
    double min, max;

    double rms;
    double energy;
  } frameinfo;

  y4m2_output *frameinfo_filter(y4m2_output *next);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
