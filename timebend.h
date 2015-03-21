/* timebend.h */

#ifndef TIMEBEND_H_
#define TIMEBEND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

typedef double (*timebend_rate_cb)(void *ctx);

y4m2_output *timebend_filter(y4m2_output *next, double rate);
y4m2_output *timebend_filter_cb(y4m2_output *next, timebend_rate_cb cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
