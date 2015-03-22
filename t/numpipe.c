/* t/numpipe.c */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "numpipe.h"
#include "framework.h"
#include "tap.h"

#define NOWT 0.0000001

static unsigned nested = 0;

static int close_to(double a, double b) {
  return fabs(a - b) < NOWT;
}

static void unwind(void) {
  while (nested > 0) {
    nest_out();
    nested--;
  }
}

#define P(x) \
  do {                                                    \
    numpipe_put(np, (x));                                 \
    nest_in("put %f", (double)(x));                       \
    nested++;                                             \
  } while (0)

#define G(x) \
  do {                                                    \
    double v = numpipe_get(np);                           \
    if (!ok(close_to(v, (x)), "get %f", (double) (x))) {  \
      diag("wanted: %f", (double) (x));                   \
      diag("   got: %f", v);                              \
    }                                                     \
    unwind();                                             \
  } while (0)

#define GG(x) do { G(x); G(x); } while (0)

static void test_numpipe_first(void) {
  numpipe *np = numpipe_new_first(1);
  nest_in("numpipe_new_first");

  GG(1);
  P(2);
  P(3);
  P(4);
  GG(2);
  P(0);
  GG(0);

  nest_out();
  numpipe_free(np);
}

static void test_numpipe_average(void) {
  numpipe *np = numpipe_new_average(2);
  nest_in("numpipe_new_average");

  GG(2);
  P(3);
  GG(3);
  GG(3);
  P(4);
  P(8);
  GG(6);
  P(100);
  GG(100);
  P(2);
  P(4);
  P(6);
  P(8);
  P(10);
  GG(6);

  nest_out();
  numpipe_free(np);
}

static void test_numpipe_last(void) {
  numpipe *np = numpipe_new_last(3);
  nest_in("numpipe_new_last");

  GG(3);
  P(2);
  P(3);
  P(4);
  GG(4);
  P(0);
  GG(0);

  nest_out();
  numpipe_free(np);
}

void test_main(void) {
  test_numpipe_first();
  test_numpipe_average();
  test_numpipe_last();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
