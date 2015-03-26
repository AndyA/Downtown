/* t/numlist.c */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "numlist.h"
#include "tap.h"
#include "util.h"

#define CHUNK 1024

static numlist *stuff_list(numlist *nl, size_t len, double base) {
  double d[len];
  for (unsigned i = 0; i < len; i++)
    d[i] = base + (double) i;
  return numlist_append(nl, d, len);
}

static int check_sequence(const double *d, size_t len, double base) {
  for (unsigned i = 0; i < len; i++)
    if (d[i] != base + (double) i) return (int) i;
  return -1;
}

static numlist *check_nl(numlist *nl, size_t expect, int n) {
  nl = stuff_list(nl, n, (double) expect + 1);
  expect += n;
  nest_in("added %d (total %llu)", n, (unsigned long long) expect);

  ok(!!nl, "nl != NULL");
  size_t size = numlist_size(nl);
  if (!ok(size == expect, "size is %u", (unsigned) expect))
    diag("Wanted %u, got %u", (unsigned) expect, (unsigned) size);

  double *d = numlist_fetch(nl, &size);
  ok(size == expect, "returned size is correct");
  int seq = check_sequence(d, size, 1);
  if (! ok(seq == -1, "data is correct")) {
    diag("Data starts to differ at offset %d", seq);
    double want = (double) seq + 1;
    for (int i = 0; i < MIN((int) expect, 10); i++) {
      diag("wanted %8.2f, got %8.2f", want, d[i]);
      want++;
    }
  }
  free(d);

  nest_out();
  return nl;
}

static void test_numlist(void) {
  nest_in("non-aligned");
  numlist *nl = NULL;

  ok(numlist_size(nl) == 0, "NULL: size == 0");

  size_t expect = 0;

  for (int n = 1; n < (int) CHUNK * 3; n += 313) {
    nl = check_nl(nl, expect, n);
    expect += n;
  }

  numlist_free(nl);
  nest_out();
}

static numlist *check_al(numlist *nl, size_t *expect, int delta, int count) {
  for (int n = 0; n < count; n++) {
    int past = (int) * expect % CHUNK;
    nest_in("%d past boundary", past);
    nl = check_nl(nl, *expect, CHUNK + delta);
    *expect += CHUNK + delta;
    nest_out();
  }
  return nl;
}

static void test_aligned(void) {
  nest_in("aligned");
  numlist *nl = NULL;

  ok(numlist_size(nl) == 0, "NULL: size == 0");

  size_t expect = 0;
  nl = check_al(nl, &expect, -1, 5);
  nl = check_al(nl, &expect, 1, 10);
  nl = check_al(nl, &expect, -1, 10);
  nl = check_al(nl, &expect, 1, 5);

  numlist_free(nl);
  nest_out();
}

static void test_get(void) {
  numlist *nl = NULL;

  const int count = 10000;

  for (int i = 0; i < count; i++)
    nl = numlist_putn(nl, (double) i + 1);

  size_t len = 1;
  for (int pos = 0; pos < count;) {
    size_t want = MIN(len, (unsigned) count - pos);
    double out[want];

    double *got = numlist_get(nl, out, pos, want);

    nest_in("get, want=%u", (unsigned) want);

    ok(out == got, "returned correct pointer");

    int seq = check_sequence(out, want, pos + 1);
    if (!ok(seq == -1, "data is correct")) {
      diag("Data starts to differ at offset %d", seq);
      double w = (double) seq + 1;
      for (int i = 0; i < MIN((int) want, 10); i++) {
        diag("wanted %8.2f, got %8.2f", w, out[i]);
        w++;
      }
    }

    nest_out();

    pos += len;
    len++;
  }

}

void test_main(void) {
  test_aligned();
  test_numlist();
  test_get();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
