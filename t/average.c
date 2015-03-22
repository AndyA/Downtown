/* t/average.c */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "average.h"
#include "framework.h"
#include "tap.h"

#define NOWT 0.0000001

static double xn(int n) {
  return (double)((n + 1) * 3.5 * ((n & 1) ? -1.3 : 3.2));
}

static double compute_average(double (*fn)(int n), int from, int to) {
  double total = 0;
  for (int i = from; i <= to; i++) total += fn(i);
  return total / (double)(to - from + 1);
}

static int close_to(double a, double b) {
  return fabs(a - b) < NOWT;
}

static void test_average(void) {
  for (unsigned len = 1; len < 5; len++) {
    nest_in("length: %u", len);
    average *avg = average_new(len);

    for (int x = 0; x < (int) len * 2; x++) {
      double datum = xn(x);
      nest_in("datum[%d]=%.4f", x, datum);

      double got = average_push(avg, datum);

      int used = x + 1;
      if (used > (int) len) used = len;

      double want = compute_average(xn, x - used + 1, x);

      if (!ok(close_to(want, got), "average is %f", want)) {
        diag("wanted %8.4f", want);
        diag("   got %8.4f", got);
      }

      ok(average_used(avg) == (unsigned) used, "used is %d", used);

      nest_out();
    }
    nest_out();
  }
}

void test_main(void) {
  test_average();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
