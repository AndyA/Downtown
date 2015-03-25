/* csv.h */

#ifndef CSV_H_
#define CSV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "numlist.h"

typedef struct csv_column {
  struct csv_column *next;
  char *name;
  numlist *data;
} csv_column;

typedef void (*csv_emit_cb)(unsigned row, const char *text, void *ctx);

csv_column *csv_parse(csv_column *csv, const char *ln);
csv_column *csv_parse_fh(csv_column *csv, FILE *fh);
void csv_free(csv_column *csv);
csv_column *csv_add_column(csv_column *csv, const char *name);
csv_column *csv_find_column(csv_column *csv, const char *name);

csv_column *csv_put(csv_column *csv, const double *vals);
double *csv_get(csv_column *csv, unsigned row, double *out);

size_t csv_cols(const csv_column *csv);
size_t csv_rows(const csv_column *csv);

void csv_emit_row(const csv_column *csv, unsigned row, csv_emit_cb cb, void *ctx);
void csv_emit(const csv_column *csv, csv_emit_cb cb, void *ctx);
void csv_emit_fh(const csv_column *csv, FILE *fh);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
