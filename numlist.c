
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "numlist.h"
#include "util.h"

static bytelist_class me = {
  .init_size = numlist_CHUNK,
  .max_size = numlist_MAX,
  .terminate = 0
};

#define SC(x)  (sizeof(double) * (x))

numlist *numlist_put(numlist *nl, const double *d, size_t len) {
  return bytelist_append_internal(nl, (unsigned char *) d, SC(len), &me);
}

numlist *numlist_putn(numlist *nl, double d) {
  return numlist_put(nl, &d, 1);
}

void numlist_free(numlist *nl) {
  bytelist_free(nl);
}

size_t numlist_size(const numlist *nl) {
  return bytelist_size(nl) / sizeof(double);
}

double *numlist_get(const numlist *nl, double *out, unsigned start, size_t len) {
  return (double *) bytelist_get(nl, (unsigned char *) out, SC(start), SC(len));
}

double *numlist_get_all(const numlist *nl, double *out) {
  return numlist_get(nl, out, 0, numlist_size(nl));
}

double *numlist_fetch(const numlist *nl, size_t *sizep) {
  if (*sizep) *sizep = numlist_size(nl);
  return (double *) bytelist_fetch(nl, NULL);
}

double *numlist_drain(numlist *nl, size_t *sizep) {
  if (*sizep) *sizep = numlist_size(nl);
  return (double *) bytelist_drain(nl, NULL);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
