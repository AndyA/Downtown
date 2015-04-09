/* profile.h */

#ifndef PROFILE_H_
#define PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "jsondata.h"
#include "sampler.h"

#define profile_SIGNATURE_BITS  256

typedef struct {
  char *filename;
  jd_var config;

  size_t len;
  double *baseline;

  unsigned smooth_span;

  sampler_context *sam;
  size_t sam_len;

} profile;

profile *profile_load(const char *filename);
void profile_free(profile *p);
double *profile_smooth(const profile *p, double *dst, const double *src, size_t len);
char *profile_signature(const profile *p, char *sig, const double *data, size_t len);
void profile_frame_size(profile *p, unsigned *wp, unsigned *hp);
sampler_context *profile_sampler(profile *p, size_t *lenp);

double *profile__log2lin(double *out, const double *in, size_t len);
double *profile__lin2log(double *out, const double *in, size_t len);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
