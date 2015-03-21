/* delay.h */

#ifndef DELAY_H_
#define DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *delay_filter(y4m2_output *next, unsigned length);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
