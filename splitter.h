/* splitter.h */

#ifndef SPLITTER_H_
#define SPLITTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *splitter_filter(y4m2_output **nexts, size_t n_next);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
