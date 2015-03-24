/* numlist.c */

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

double *numlist_get(const numlist *nl, size_t *sizep) {
  size_t size = numlist_size(nl);
  double *out = alloc(sizeof(double) * size);
  double *op = out + size;

  while (op != out) {
    op -= nl->used;
    memcpy(op, &nl->data, sizeof(double) * nl->used);
    nl = nl->next;
  }

  if (sizep) *sizep = size;
  return out;
}

double *numlist_drain(numlist *nl, size_t *sizep) {
  double *data = numlist_get(nl, sizep);
  numlist_free(nl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
