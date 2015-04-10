/* t/profile.c */

#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "json.h"
#include "profile.h"
#include "tap.h"
#include "util.h"

#define MAX_DIFFS 8

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

static char *diff_str(char *out, const char *a, const char *b) {
  size_t alen = strlen(a);
  size_t blen = strlen(b);
  char *op = out;
  unsigned i;

  for (i = 0; i < alen && i < blen; i++)
    *op++ = a[i] == b[i] ? ' ' : '^';

  for (; i < alen || i < blen; i++)
    *op++ = '^';

  *op++ = '\0';

  return out;
}

static int ndiff(const char *a, const char *b) {
  int diffs = 0;
  while (*a && *b) if (*a++ != *b++) diffs++;
  while (*a++) diffs++;
  while (*b++) diffs++;
  return diffs;
}

static void test_sig(void) {
  nest_in("sig");

  const char *save_as = getenv("PROFILE_SAVE");

  jd_var sig = JD_INIT;

  char *prof = tf_resource("data/default.profile");
  profile *p = profile_load(prof);

  char *ref = tf_resource("data/sig.json");
  json_load_file(&sig, ref);

  size_t frames = jd_count(&sig);
  for (unsigned f = 0; f < frames; f++) {
    char got[profile_SIGNATURE_BITS + 1];
    size_t dlen;
    jd_var *slot = jd_get_idx(&sig, f);
    double *data = json_get_real(jd_get_idx(jd_get_ks(slot, "planes", 0), 0), &dlen);

    profile_signature(p, got, data, dlen);

    if (save_as)
      jd_set_string(jd_get_ks(slot, "signature", 1), got);

    const char *want = jd_bytes(jd_get_ks(slot, "signature", 0), NULL);
    int diffs = ndiff(want, got);
    if (!ok(diffs < MAX_DIFFS, "signature matches with %d bits", MAX_DIFFS)) {
      char diff[profile_SIGNATURE_BITS + 1];

      diag("wanted: %s", want);
      diag("got:    %s", got);
      diag("        %s", diff_str(diff, want, got));
    }
    ok(!strcmp(want, got), "signature: %s", want);

    free(data);
  }

  if (save_as) {
    json_save_file(&sig, save_as);
    diag("Updated test data saved as %s", save_as);
  }

  profile_free(p);

  free(prof);
  free(ref);
  jd_release(&sig);

  nest_out();
}

void test_main(void) {
  test_lin_log();
  test_profile();
  test_sig();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
