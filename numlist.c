
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "numlist.h"
#include "util.h"

numlist *numlist_put(numlist *nl, const double *d, size_t len) {
  while (len) {
    if (!nl || nl->used == numlist_CHUNK) {
      numlist *nnl = alloc(sizeof(numlist));
      nnl->tail_size = numlist_size(nl);
      nnl->next = nl;
      nl = nnl;
    }
    size_t avail = MIN(len, numlist_CHUNK - nl->used);
    memcpy(&nl->data[nl->used], d, sizeof(double) * avail);
    nl->used += avail;
    d += avail;
    len -= avail;
  }

  return nl;
}

numlist *numlist_putn(numlist *nl, double d) {
  return numlist_put(nl, &d, 1);
}

void numlist_free(numlist *nl) {
  while (nl) {
    numlist *next = nl->next;
    free(nl);
    nl = next;
  }
}

size_t numlist_size(const numlist *nl) {
  if (!nl) return 0;
  return nl->used + nl->tail_size;
}

static unsigned _get(const numlist *nl, double *out, unsigned end, size_t len) {

  if (!nl || !len) return 0;
  if (nl->used <= end) return _get(nl->next, out, end - nl->used, len);

  unsigned avail = MIN(nl->used - end, len);

  memcpy(out + len - avail,
         nl->data + nl->used - end - avail,
         sizeof(double) * avail);

  return avail + _get(nl->next, out, 0, len - avail);
}

double *numlist_get(const numlist *nl, double *out, unsigned start, size_t len) {
  (void) _get(nl, out, numlist_size(nl) - (start + len), len);
  return out;
}

double *numlist_get_all(const numlist *nl, double *out) {
  return numlist_get(nl, out, 0, numlist_size(nl));
}

double *numlist_fetch(const numlist *nl, size_t *sizep) {
  size_t size = numlist_size(nl);
  double *out = alloc(sizeof(double) * size);
  numlist_get_all(nl, out);
  if (sizep) *sizep = size;
  return out;
}

double *numlist_drain(numlist *nl, size_t *sizep) {
  double *data = numlist_fetch(nl, sizep);
  numlist_free(nl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
