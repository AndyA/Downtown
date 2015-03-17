/* dumpframe.h */

#ifndef DUMPFRAME_H_
#define DUMPFRAME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "yuv4mpeg2.h"

y4m2_output *dumpframe_filter(y4m2_output *next, const char *name_pattern, unsigned every);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
