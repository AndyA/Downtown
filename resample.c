/* resample.c */

#include <string.h>

#include "resample.h"

double *resample_double(double *out, size_t osize, const double *in, size_t isize) {

  if (osize == 0)
    return out;

  if (isize == 0) {
    memset(out, 0, sizeof(double) * osize);
    return out;
  }

  if (isize == osize) {
    memcpy(out, in, sizeof(double) * osize);
    return out;
  }

  double scale = (double) isize / (double) osize;
  for (int i = 0; i < (int) osize; i++) {

    // when scale == 0.7
    //
    // i      0    1    2    3    4    5    6    7
    // is     0.0  0.7  1.4  2.8  3.5  4.2  4.9  5.6
    // ie     0.7  1.4  2.8  3.5  4.2  4.9  5.6  6.3
    //
    // when scale == 1.1
    //
    // i      0    1    2    3    4    5    6    7
    // is     0.0  1.1  2.2  3.3  4.4  5.5  6.6  7.7
    // ie     1.1  2.2  3.3  4.4  5.5  6.6  7.7  8.8
    //
    // when scale == 2.3
    //
    // i      0    1    2    3    4    5    6    7
    // is     0.0  2.3  4.6  6.9  9.2 11.5 13.8 16.1
    // ie     2.3  4.6  6.9  9.2 11.5 13.8 16.1 18.4

    double is = (double) i * scale;
    double ie = is + scale;

    int iis = (int) is;
    int iie = (int) ie;

    double sum;
    if (iis == iie) {
      sum = in[iis] * scale;
    }
    else {
      sum = in[iis] * (1 - (is - iis));
      iis++;
      while (iis != iie) sum += in[iis++];
      sum += in[iis] * (ie - iie);
    }
    out[i] = sum / scale;
  }
  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
