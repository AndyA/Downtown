/* profile.c */

#include <jd_pretty.h>
#include <math.h>
#include <stdlib.h>

#include "json.h"
#include "profile.h"
#include "resample.h"
#include "sampler.h"
#include "util.h"

/*#define RESAMPLE_LOG*/

#ifdef RESAMPLE_LOG
#define RESAMPLE resample_log
#else
#define RESAMPLE resample_double
#endif

static void unpack(profile *p) {
  p->baseline = json_get_real(jd_get_ks(&p->config, "baseline", 0), &p->len);
}

profile *profile_load(const char *filename) {
  profile *p = alloc(sizeof(profile));
  p->filename = sstrdup(filename);
  json_load_file(&p->config, p->filename);
  unpack(p);
  return p;
}

void profile_free(profile *p) {
  jd_release(&p->config);
  sampler_free(p->sam);
  free(p->filename);
  free(p->baseline);
}

double *profile__log2lin(double *out, const double *in, size_t len) {
  for (unsigned i = 0; i < len; i++)
    out[i] = exp(in[i]);
  return out;
}

double *profile__lin2log(double *out, const double *in, size_t len) {
  for (unsigned i = 0; i < len; i++)
    out[i] = log(in[i]);
  return out;
}

#ifdef RESAMPLE_LOG
static double *resample_log(double *out, size_t olen, const double *in, size_t ilen) {
  double in_log[ilen];
  double out_log[olen];

  profile__lin2log(in_log, in, ilen);
  resample_double(out_log, olen, in_log, ilen);
  profile__log2lin(out, out_log, olen);
  return out;
}
#endif

char *profile_signature(const profile *p, char *sig, const double *data, size_t len) {
  if (len != p->len) die("Data size incorrect: profile length: %u, data length: %u",
                           (unsigned) p->len, (unsigned) len);
  double delta[p->len];

  for (unsigned i = 0; i < len; i++)
    delta[i] = data[i] / p->baseline[i];

  double sig_data[profile_SIGNATURE_BITS];

  RESAMPLE(sig_data, profile_SIGNATURE_BITS, delta, p->len);

  for (unsigned i = 0; i < p->len; i++)
    sig[i] = sig_data[i] > 1 ? '1' : '0';

  sig[profile_SIGNATURE_BITS] = '\0';

  return sig;
}

void profile_frame_size(profile *p, unsigned *wp, unsigned *hp) {
  if (wp) *wp = (unsigned) jd_get_int(jd_get_ks(&p->config, "width", 0));
  if (hp) *hp = (unsigned) jd_get_int(jd_get_ks(&p->config, "height", 0));
}

sampler_context *profile_sampler(profile *p, size_t *lenp) {
  if (!p->sam) {
    const char *spec = jd_bytes(jd_get_ks(&p->config, "sampler", 0), NULL);
    if (!spec) die("'sampler' missing in %s", p->filename);
    p->sam = sampler_new(spec, p->filename);
    unsigned w, h;
    profile_frame_size(p, &w, &h);
    p->sam_len = sampler_init(p->sam, w, h);
  }
  if (lenp) *lenp = p->sam_len;
  return p->sam;
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
