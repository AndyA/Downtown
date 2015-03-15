/* frameinfo.h */

#ifndef FRAMEINFO_H_
#define FRAMEINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

  typedef struct {
    double average;
    double variance;
  } frameinfo;

  y4m2_output *filter_frameinfo(y4m2_output *next);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
