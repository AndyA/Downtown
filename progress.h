/* progress.h */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *progress_filter(y4m2_output *next, int every);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
