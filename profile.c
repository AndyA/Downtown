/* profile.c */

#include "json.h"
#include "profile.h"
#include "resample.h"
#include "util.h"

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
  free(p->filename);
  free(p->baseline);
}

char *profile_signature(const profile *p, char *sig, const double *data, size_t len) {
  if (len != p->len) die("Data size incorrect: profile length: %u, data length: %u",
                           (unsigned) p->len, (unsigned) len);
  double delta[p->len];

  for (unsigned i = 0; i < len; i++)
    delta[i] = data[i] - p->baseline[i];

  double sig_data[profile_SIGNATURE_BITS];

  resample_double(sig_data, profile_SIGNATURE_BITS, delta, p->len); /* log? */

  for (unsigned i = 0; i < p->len; i++)
    sig[i] = sig_data[i] > 0 ? '1' : '0';

  return sig;
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
