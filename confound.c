/* confound.c */

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "numlist.h"
#include "tb_convolve.h"
#include "util.h"

#define PROG      "confound"

#define NOWT        0.000000001

#define countof(ar) (sizeof(ar)/sizeof((ar)[0]))

static void confound(int n) {
  double sample[1000];
  double out[countof(sample)];
  double coef[] = {1, 1};

  for (unsigned i = 0; i < countof(sample); i++)
    sample[i] = n;

  tb_convolve *c = tb_convolve_new(countof(coef), coef);
  tb_convolve_set_linear(c, 1);

  tb_convolve_apply(c, out, sample, countof(sample));

  for (unsigned i = 0; i < countof(out); i++)
    printf("%f ", out[i]);
  printf("\n");

  tb_convolve_free(c);
}

int main(void) {
  confound(2);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
