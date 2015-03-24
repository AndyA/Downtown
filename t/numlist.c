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

static void test_numlist(void) {
  numlist *nl = NULL;

  ok(numlist_size(nl) == 0, "NULL: size == 0");

  size_t expect = 0;

  for (int n = 1; n < numlist_CHUNK * 3; n += 313) {
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
  }

  numlist_free(nl);

}

void test_main(void) {
  test_numlist();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
