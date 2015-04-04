/* t/profile.c */

#include <stdlib.h>

#include "profile.h"
#include "framework.h"
#include "tap.h"
#include "util.h"

#define countof(x) (sizeof(x)/sizeof((x)[0]))

static double _random(void) {
  return ((double)rand() / (double)RAND_MAX);
}

static int ar_close_to(const double *got, const double *want, size_t len) {
  int bad = 0;
  for (unsigned i = 0; i < len; i++) {
    if (!close_to(got[i], want[i], "datum %u mateches", i)) {
      diag("%4u: wanted %f, got %f", i, want[i], got[i]);
      bad++;
    }
  }

  return !bad;
}

static void test_lin_log(void) {
  nest_in("lin/log");

  double data[200];

  for (unsigned i = 0; i < countof(data); i++)
    data[i] = _random() + 1;

  double dlog[countof(data)];
  double dlin[countof(data)];

  profile__lin2log(dlog, data, countof(data));
  profile__log2lin(dlin, dlog, countof(dlog));

  ar_close_to(dlin, data, countof(data));

  nest_out();
}

static void test_profile(void) {
  nest_in("profile");
  char *prof = tf_resource("data/profile.json");
  profile *p = profile_load(prof);
  free(prof);

  ok(p->len == 7, "profile length");

  profile_free(p);
  nest_out();
}

void test_main(void) {
  test_lin_log();
  test_profile();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
