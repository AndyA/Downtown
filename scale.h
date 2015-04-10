/* scale.h */

#ifndef SCALE_H_
#define SCALE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *scale_filter(y4m2_output *next, unsigned width, unsigned height);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
