/* t/numlist.c */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "numlist.h"
#include "tap.h"
#include "util.h"

static numlist *stuff_list(numlist *nl, size_t len, double base) {
  double d[len];
  for (unsigned i = 0; i < len; i++)
    d[i] = base + (double) i;
  return numlist_put(nl, d, len);
}

static int check_sequence(double *d, size_t len, double base) {
  for (unsigned i = 0; i < len; i++)
    if (d[i] != base + (double) i) return (int) i;
  return -1;
}

static numlist *test_nl(numlist *nl, size_t expect, int n) {
  nl = stuff_list(nl, n, (double) expect + 1);
  expect += n;
  nest_in("added %d (total %llu)", n, (unsigned long long) expect);

  ok(!!nl, "nl != NULL");
  ok(numlist_size(nl) == expect, "size is %llu", (unsigned long long) expect);

  size_t size;
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

  for (int n = 1; n < numlist_CHUNK * 3; n += 313) {
    nl = test_nl(nl, expect, n);
    expect += n;
  }

  numlist_free(nl);
  nest_out();
}

static numlist *test_al(numlist *nl, size_t *expect, int delta, int count) {
  for (int n = 0; n < count; n++) {
    int past = (int) * expect % numlist_CHUNK;
    nest_in("%d past boundary", past);
    nl = test_nl(nl, *expect, numlist_CHUNK + delta);
    *expect += numlist_CHUNK + delta;
    nest_out();
  }
  return nl;
}

static void test_aligned(void) {
  nest_in("aligned");
  numlist *nl = NULL;

  ok(numlist_size(nl) == 0, "NULL: size == 0");

  size_t expect = 0;
  nl = test_al(nl, &expect, -1, 5);
  nl = test_al(nl, &expect, 1, 10);
  nl = test_al(nl, &expect, -1, 10);
  nl = test_al(nl, &expect, 1, 5);

  numlist_free(nl);
  nest_out();
}

void test_main(void) {
  test_aligned();
  test_numlist();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
