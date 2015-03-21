/* timebend.h */

#ifndef TIMEBEND_H_
#define TIMEBEND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

#define timebend_RATE_NOTE "timebend_rate"

typedef struct {
  double rate;
} timebend_rate_note;

y4m2_output *timebend_filter(y4m2_output *next, double rate);

void timebend_set_rate(y4m2_frame *frame, double rate);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
