/* t/tb_convolve.c */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "colour.h"
#include "framework.h"
#include "tap.h"
#include "tb_convolve.h"
#include "util.h"


static void test_convolve(unsigned len, const double *pos_coef, const double *neg_coef,
                          unsigned dlen, const double *in, const double *want) {

  double out[dlen];

  tb_convolve *c = tb_convolve_new_signed(len, pos_coef, neg_coef);
  not_null(c, "tb_convolve_new");

  tb_convolve_apply(c, out, in, dlen);

  for (int i = 0; i < (int) dlen; i++)
    if (!close_to(out[i], want[i], "%d: got %f", i, want[i]))
      diag("wanted %f, got %f", want[i], out[i]);

  tb_convolve_free(c);
}

static void test_blur(int len, int dlen) {
  double coef[len];
  double want[dlen];

  nest_in("blur(%d -> %d)", len, dlen);

  for (int i = 0; i < len; i++) coef[i] = 1 / (double)len;
  for (int i = 0; i < dlen; i++) want[i] = 1;

  test_convolve(len, coef, coef, dlen, want, want);
  nest_out();
}

static void test_unity(int len) {
  double coef[len];

  nest_in("unity(%d)", len);

  for (int i = 0; i < len; i++) coef[i] = 0;
  coef[len / 2] = 1;

  test_convolve(len, coef, coef, len, coef, coef);

  nest_out();
}

static void test_convolver(void) {
  test_blur(1, 2);
  for (int n = 1; n < 10; n += 2)
    test_unity(n);

  for (int n = 1; n < 10; n += 2)
    for (int l = 1; l < 50; l *= 2)
      test_blur(n, l);
}

static void test_elapsed(unsigned len) {
  double data[len];
  double total = 0;
  for (unsigned i = 0; i < len; i++) {
    data[i] = i + 1;
    total += 1.0 / (i + 1);
  }
  double got = tb_convolve_elapsed(data, len);
  if (!close_to(got, total, "total to %d", len)) {
    diag("wanted %f, got %f", total, got);
  }
}

static void test_translation(void) {
  for (unsigned i = 1; i < 1000; i += 13)
    test_elapsed(i);
}

void test_main(void) {
  test_convolver();
  test_translation();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
