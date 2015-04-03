/* signature.c */

#include <jd_pretty.h>
#include <math.h>
#include <stdlib.h>

#include "average.h"
#include "resample.h"
#include "signature.h"

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

char *sig_signature(char *sig, const double *src, size_t len) {
  double scaled[sig_SIGNATURE_BITS];
  double smoothed[sig_SIGNATURE_BITS];

  sig_scale_length(scaled, src, len);
  sig_smooth(smoothed, scaled, sig_SIGNATURE_BITS);

  for (unsigned i = 0; i < sig_SIGNATURE_BITS; i++)
    sig[i] = scaled[i] > smoothed[i] ? '1' : '0';
  sig[sig_SIGNATURE_BITS] = '\0';

  return sig;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
