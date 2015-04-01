/* signature.c */

#include <jd_pretty.h>
#include <math.h>
#include <stdlib.h>

#include "average.h"
#include "resample.h"
#include "signature.h"

/*#define DUMP_JSON*/

#define PLP_M           -0.45
#define PLP_P            1.2
#define PLP_C            4.15

#define AVERAGE_SPAN    15

/* TODO: currently this is tuned for spiral samples of a particular
 * length. Need something generic, automatic
 * */
double sig_normalise(double x) {
  return exp(PLP_M * pow(log(x), PLP_P) + PLP_C);
}

double *sig_scale_length(double *dst, const double *src, size_t len) {
  return resample_double(dst, sig_SIGNATURE_BITS, src, len);
}

double *sig_smooth(double *dst, const double *src, size_t len) {
  average *avg = average_new_log(AVERAGE_SPAN);

  unsigned dpos = 0;
  for (unsigned i = 0; i < len; i++) {
    if (average_ready(avg)) dst[dpos++] = average_get(avg);
    average_push(avg, src[i]);
  }

  while (dpos != len) {
    dst[dpos++] = average_get(avg);
    average_pop(avg);
  }

  average_free(avg);

  return dst;
}

#ifdef DUMP_JSON
static jd_var *stuff_nums(jd_var *out, const double *in, size_t len) {
  jd_var *slot = jd_push(jd_set_array(out, len), len);
  for (unsigned i = 0; i < len; i++)
    jd_set_real(slot + i, in[i]);
  return out;
}
#endif

char *sig_signature(char *sig, const double *src, size_t len) {
  double scaled[sig_SIGNATURE_BITS];
  double smoothed[sig_SIGNATURE_BITS];

  sig_scale_length(scaled, src, len);
  sig_smooth(smoothed, scaled, sig_SIGNATURE_BITS);

  for (unsigned i = 0; i < sig_SIGNATURE_BITS; i++)
    sig[i] = scaled[i] > smoothed[i] ? '1' : '0';
  sig[sig_SIGNATURE_BITS] = '\0';

#ifdef DUMP_JSON
  scope {
    jd_var *hash = jd_nhv(10);
    stuff_nums(jd_get_ks(hash, "data", 1), src, len);
    stuff_nums(jd_get_ks(hash, "scaled", 1), scaled, sig_SIGNATURE_BITS);
    stuff_nums(jd_get_ks(hash, "smoothed", 1), smoothed, sig_SIGNATURE_BITS);
    jd_set_string(jd_get_ks(hash, "sig", 1), sig);
    jd_printf("%J", hash);
  }
#endif

  return sig;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
