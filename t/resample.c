/* t/resample.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "resample.h"
#include "tap.h"
#include "util.h"

#define SIGMA 0.00001

#define countof(ar) (sizeof(ar) / sizeof((ar)[0]))

static int cmp_double(const double *a, const double *b, size_t len) {
  for (unsigned i = 0; i < len; i++)
    if (fabs(a[i] - b[i]) >= SIGMA)
      return 0;
  return 1;
}

static void diag_double(const char *capn, const double *a, size_t len) {
  fprintf(stderr, "# %-10s { ", capn);
  for (unsigned i = 0; i < len; i++)
    fprintf(stderr, "%s%7.3f", i ? ", " : "", a[i]);
  fprintf(stderr, " }\n");
}

static void fmt_double(char *buf, size_t bsz, const double *a, size_t len) {
  char *bp = buf;
  char *bl = buf + bsz;

  for (unsigned i = 0; i < len; i++) {
    if (bp + 5 > bl) break;
    if (i) {
      strcpy(bp ,  ", ");
      bp += strlen(bp);
    }
    size_t nl = snprintf(bp, bl - bp, "%.3f", a[i]);
    if ((int) nl >= bl - bp) break;
    bp += strlen(bp);
  }
}

static void is_resample(const double *in, size_t isize, const double *want, size_t wsize) {
  double out[wsize];
  char dibuf[1024];
  char dobuf[1024];

  /*  diag("is_resample(%p, %llu, %p, %llu)", in, isize, want, wsize);*/

  resample_double(out, wsize, in, isize);
  fmt_double(dibuf, sizeof(dibuf), in, isize);
  fmt_double(dobuf, sizeof(dobuf), want, wsize);
  if (!ok(cmp_double(out, want, wsize), "{ %s } -> { %s }", dibuf, dobuf)) {
    diag_double("in", in, isize);
    diag_double("want", want, wsize);
    diag_double("out", out, wsize);
  }
}

static void test_resample(void) {
  {
    const double in[] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    const double want[] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    is_resample(in, countof(in), want, countof(want));
  }

  {
    const double in[] = { 1.0, 2.0 };
    const double want[] = {1.0, 1.5, 2.0};
    is_resample(in, countof(in), want, countof(want));
  }

  {
    const double in[] = { 1.0, 2.0, 3.0 };
    const double want[] = {1.0, (4.0 / 3.0), 2.0, (8.0 / 3.0), 3.0};
    is_resample(in, countof(in), want, countof(want));
  }

  {
    const double in[] = { 1.0, 2.0, 3.0, 4.0 };
    const double want[] = { 1.5, 3.5 };
    is_resample(in, countof(in), want, countof(want));
  }

}

void test_main(void) {
  test_resample();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
