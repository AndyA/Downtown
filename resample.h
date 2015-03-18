/* resample.h */

#ifndef RESAMPLE_H_
#define RESAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

  double *resample_double(double *out, size_t osize, const double *in, size_t isize);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
