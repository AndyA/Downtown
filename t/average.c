/* t/average.c */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "average.h"
#include "framework.h"
#include "tap.h"

#define NOWT 0.0000001

static double xn(int n) {
  return (double)((100 - n) * 3.5 * ((n & 1) ? -1.3 : 3.2));
}

static void compute_average(double (*fn)(int n), int from, int to,
                            double *minp, double *avgp, double *maxp) {
  double total = 0;
  double min = NAN, max = NAN;
  for (int i = from; i <= to; i++) {
    double datum = fn(i);
    if (isnan(min) || datum < min) min = datum;
    if (isnan(max) || datum > max) max = datum;
    total += datum;
  }
  *minp = min;
  *avgp = total / (double)(to - from + 1);
  *maxp = max;
}

static void test_average(void) {
  for (unsigned len = 1; len < 5; len++) {
    nest_in("length: %u", len);
    average *avg = average_new(len);

    for (int x = 0; x < (int) len * 2; x++) {
      double datum = xn(x);
      nest_in("datum[%d]=%.4f", x, datum);

      double got_avg = average_push(avg, datum);
      double got_min = average_min(avg);
      double got_max = average_max(avg);

      int used = x + 1;
      if (used > (int) len) used = len;

      double want_min, want_avg, want_max;
      compute_average(xn, x - used + 1, x, &want_min, &want_avg, &want_max);

      if (!close_to(want_avg, got_avg, "average is %f", want_avg)) {
        diag("wanted %8.4f", want_avg);
        diag("   got %8.4f", got_avg);
      }

      if (!close_to(want_min, got_min, "min is %f", want_min)) {
        diag("wanted %8.4f", want_min);
        diag("   got %8.4f", got_min);
      }

      if (!close_to(want_max, got_max, "max is %f", want_max)) {
        diag("wanted %8.4f", want_max);
        diag("   got %8.4f", got_max);
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
