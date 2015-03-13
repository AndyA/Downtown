/* t/sampler.c */

#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "tap.h"
#include "sampler.h"

static void test_param(void) {
  sampler_params *sp = sampler_parse_params("a=1.24,b=-3,c=99999");

  ok(0 == strcmp(sp->name, "a"), "a: name matches");
  ok(sp->value == 1.24, "a: value matches");

  sampler_free_params(sp);
}

void test_main(void) {
  test_param();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
