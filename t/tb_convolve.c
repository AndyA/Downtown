/* t/tb_convolve.c */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "colour.h"
#include "framework.h"
#include "tap.h"
#include "tb_convolve.h"
#include "util.h"


static void test_convolver(void) {
  const double coef[] = {1};

  tb_convolve *c = tb_convolve_new(sizeof(coef) / sizeof(coef[0]), coef);
  not_null(c, "tb_convolve_new");
  tb_convolve_free(c);
}

void test_main(void) {
  test_convolver();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
