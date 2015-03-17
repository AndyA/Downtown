/* passthru.h */

#ifndef PASSTHRU_H_
#define PASSTHRU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *passthru_filter(y4m2_output *next);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
