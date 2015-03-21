/* injector.h */

#ifndef INJECTOR_H_
#define INJECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

typedef y4m2_frame *(*injector_callback)(y4m2_frame *frame, void *ctx);

y4m2_output *injector_filter(y4m2_output *next, injector_callback cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
