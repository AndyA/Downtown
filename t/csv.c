/* t/csv.c */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "framework.h"
#include "tap.h"
#include "util.h"

static int randint(int limit) {
  if (limit == 0) return 0;
  int rn, mask = 1;
  while (mask < limit) mask <<= 1;
  mask--;
  do {
    rn = rand() & mask;
  }
  while (rn >= limit);
  return rn;
}

static void shuffle(int *ar, size_t len) {
  while (len > 1) {
    int pick = randint(len--);
    int t = ar[len];
    ar[len] = ar[pick];
    ar[pick] = t ;
  }
}

static void check_find(csv_column *csv, const char *name) {
  csv_column *hit = csv_find_column(csv, name);
  not_null(hit, "found %s", name);
  ok(!strcmp(name, hit->name), "name matches");
}

static void test_basic(void) {
  csv_column *csv = NULL;

  csv = csv_add_column(csv, "A");
  csv = csv_add_column(csv, "B");
  csv = csv_add_column(csv, "C");

  check_find(csv, "A");
  check_find(csv, "B");
  check_find(csv, "C");
  null(csv_find_column(csv, "D"), "D not found");

  size_t ncols = csv_cols(csv);
  const size_t nrows = 2000;

  is(ncols, 3, "3 columns");
  is(csv_rows(csv), 1, "1 row (header)");

  double row[ncols];
  int order[nrows];

  for (int r = 1; r <= (int) nrows; r++) {
    for (int c = 0; c < (int) ncols; c++)
      row[c] = r * ncols + c;
    csv = csv_put(csv, row);
    order[r - 1] = r;
  }

  is(csv_rows(csv), nrows + 1, "%d rows", (int) nrows + 1);

  shuffle(order, nrows);
  for (int i = 0; i < (int) nrows; i++) {
    int r = order[i];
    csv_get(csv, r, row);
    for (int c = 0; c < (int) ncols; c++) {
      double want = r * ncols + c;
      double got  = row[c];
      if (!ok(want == got, "[%d, %d] = %f", c, r, want))
        diag("wanted %f, got %f", want, got);
    }
  }

  csv_free(csv);
}

void test_main(void) {
  test_basic();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
