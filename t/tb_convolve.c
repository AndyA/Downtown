/* t/tb_convolve.c */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "colour.h"
#include "framework.h"
#include "tap.h"
#include "tb_convolve.h"
#include "util.h"

#define NOWT 0.000000001

#define countof(ar) (sizeof(ar)/sizeof((ar)[0]))

static void check_ramp(double lo, double hi) {
  int size = (int) hi + 2;
  double ramp[size];

  for (int i = 0; i < size; i++) ramp[i] = (double) i;

  nest_in("ramp[%f -> %f]", lo, hi);
  double area = (hi * hi - lo * lo) / 2;
  double sample = tb_convolve__sample(ramp, lo, hi - lo);
  if (!close_to(area, sample, "area: %f", area))
    diag("wanted %f, got %f", area, sample);
  nest_out();
}

static void test_sampler(void) {
  for (double s = 0; s < 5; s += 0.125)
    check_ramp(s, s + 1);

  double lo = 0, hi = 19;
  while (lo < hi) {
    check_ramp(lo, hi);
    lo += 0.1;
    hi -= 0.33;
  }

}

static double _area(const double *coef, size_t len) {
  double area = 0;
  for (int i = 1; i < (int) len; i++)
    area += (coef[i - 1] + coef[i]) / 2;
  return area;
}

static double _scale(double *coef, size_t len, double scale) {
  double area = _area(coef, len);
  double adj = scale / area;
  for (int i = 0; i < (int) len; i++) coef[i] *= adj;
  return adj;
}

static double _random(void) {
  return ((double)rand() / (double)RAND_MAX);
}

static int  _middle_bit(const double *buf, int len,
                        double want, double nowt,
                        int *left, int *right) {
  int l, r, mid = len / 2;

  for (l = mid; l > 0; l--)
    if (fabs(buf[l - 1] - want) > nowt)
      break;

  for (r = mid; r < len; r++)
    if (fabs(buf[r] - want) > nowt)
      break;

  if (left) *left = l;
  if (right) *right = r;
  return r - l;
}

static void check_unity(int len) {
  double coef[len];
  double sample[2048];
  double out[countof(sample)];

  for (int i = 0; i < len; i++) coef[i] = _random();
  _scale(coef, len, 1);

  for (double v = 0.1; v < 20; v *= 1.1) {
    nest_in("value %f", v);

    for (int i = 0; i < (int) countof(sample); i++)
      sample[i] = v;

    tb_convolve *c = tb_convolve_new(len, coef);
    not_null(c, "tb_convolve_new");

    tb_convolve_apply(c, out, sample, countof(sample));
    int left, right;
    _middle_bit(out, countof(sample), v, v / 100,  &left, &right);
    right = countof(sample) - 1 - right;
    int limit = countof(sample) / 15;

    ok(left < limit, "left < %d (it's %d)", limit, left);
    ok(right < limit, "right < %d (it's %d)", limit, right);

    tb_convolve_free(c);

    nest_out();
  }
}

static void test_unity(void) {
  for (int len = 2; len < 15; len += 2) {
    nest_in("kernel length %d", len);
    check_unity(len);
    nest_out();
  }
}

static void check_transform(const double *data, int len, double expect) {

  double elapsed = tb_convolve_elapsed(data, len);
  if (!within(elapsed, expect, 0.001, "total to %d is %f", len, expect))
    diag("wanted %f, got %f", expect, elapsed);

  size_t size = (size_t) elapsed;
  if (elapsed > (double) size) size++;

  double buf[size];
  double frames = tb_convolve_translate(data, len, buf, size);

  if (!within(frames, elapsed, elapsed / 50, "got %f frames", elapsed))
    diag("wanted %f, got %f", elapsed, frames);

}

static void check_elapsed(int len) {
  double data[len];
  double expect = 0;

  nest_in("series(%u)", len);

  for (int i = 0; i < len; i++) {
    data[i] = i + 1;
    expect += 1.0 / (i + 1);
  }

  check_transform(data, len, expect);

  nest_out();
}

static void check_constant_elapsed(int len, double v) {
  double data[len];

  nest_in("constant %f x %d", v, len);
  for (int i = 0; i < len; i++) data[i] = v;
  double expect = 1 / v * (double)len;

  check_transform(data, len, expect);
  nest_out();
}

static void test_translation(void) {
  for (int i = 100; i < 10000; i *= 3)
    for (double v = 0.1; v < 50; v *= 1.1)
      check_constant_elapsed(i, v);

  for (int i = 100; i < 10000; i *= 3)
    check_elapsed(i);
}

void test_main(void) {
  test_unity();
  test_translation();
  test_sampler();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
