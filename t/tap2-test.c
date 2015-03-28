/* t/tap2-test.c */

#include <stdio.h>

#include "framework.h"
#include "tap.h"
#include "util.h"

#define TAP2_NO_ALIAS

#include "tap2.h"

void test_main(void) {
  tap2_ok(1 == 1, "It's OK!");
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
