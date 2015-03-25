/* csv.c */

#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "numlist.h"
#include "util.h"

csv_column *csv_parse(csv_column *csv, const char *ln) {
  (void) csv;
  (void) ln;
  return NULL;

}

csv_column *csv_parse_fh(csv_column *csv, FILE *fh) {
  (void) csv;
  (void) fh;
  return NULL;

}

void csv_free(csv_column *csv) {
  if (csv) {
    csv_free(csv->next);
    free(csv->name);
    numlist_free(csv->data);
  }
}

csv_column *csv_add_column(csv_column *csv, const char *name) {
  if (csv) {
    csv->next = csv_add_column(csv->next, name);
    return csv;
  }

  csv = alloc(sizeof(csv_column));
  csv->name = sstrdup(name);
  return csv;
}

csv_column *csv_find_column(csv_column *csv, const char *name) {
  if (!csv) return NULL;
  if (!strcmp(name, csv->name)) return csv;
  return csv_find_column(csv->next, name);
}

csv_column *csv_put(csv_column *csv, const double *vals) {
  if (csv) {
    csv->data = numlist_putn(csv->data, *vals);
    csv->next = csv_put(csv->next, vals + 1);
  }
  return csv;
}

double *csv_get(csv_column *csv, unsigned row, double *out) {
  if (row == 0) die("Can't read the header row. Data rows start at 1");
  if (csv) {
    numlist_get(csv->data, out, row - 1, 1);
    csv_get(csv->next, row, out + 1);
  }
  return out;
}

size_t csv_cols(const csv_column *csv) {
  if (!csv) return 0;
  return 1 + csv_cols(csv->next); /* +1 for me */
}

size_t csv_rows(const csv_column *csv) {
  if (!csv) return 0;
  return 1 + numlist_size(csv->data); /* +1 for header */
}

void csv_emit_row(const csv_column *csv, unsigned row, csv_emit_cb cb, void *ctx) {
  (void) csv;
  (void) row;
  (void) cb;
  (void) ctx;

}

void csv_emit(const csv_column *csv, csv_emit_cb cb, void *ctx) {
  (void) csv;
  (void) cb;
  (void) ctx;

}

void csv_emit_fh(const csv_column *csv, FILE *fh) {
  (void) csv;
  (void) fh;

}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
