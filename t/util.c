/* t/util.c */

#include <stdio.h>
#include <string.h>

#include "colour.h"
#include "framework.h"
#include "tap.h"
#include "util.h"

static void is_aspect(const char *want, int w, int h) {
  char *ar = aspect_ratio(w, h);
  if (!ok(!strcmp(ar, want), "%ux%u -> %s", w, h, want)) {
    diag("For %dx%d wanted %s, got %s", w, h, want, ar);
  }
  free(ar);
}

static void test_aspect(void) {
  is_aspect("16:9", 1920, 1080);
  is_aspect("4:3", 768, 576);
}

void test_main(void) {
  test_aspect();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
