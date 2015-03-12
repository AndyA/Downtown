/* merge.h */

#ifndef MERGE_H_
#define MERGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *filter_merge(const y4m2_output *next, int frames);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
