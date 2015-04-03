/* profile.h */

#ifndef PROFILE_H_
#define PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "jsondata.h"
#include "sampler.h"

#define profile_SIGNATURE_BITS  256

typedef struct {
  char *filename;
  jd_var config;

  size_t len;
  double *baseline;

  sampler_context *sam;
  size_t sam_len;

} profile;

profile *profile_load(const char *filename);
void profile_free(profile *p);
char *profile_signature(const profile *p, char *sig, const double *data, size_t len);
sampler_context *profile_sampler(profile *p, size_t *lenp);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
