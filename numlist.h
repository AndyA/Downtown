/* numlist.h */

#ifndef NUMLIST_H_
#define NUMLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define numlist_CHUNK 1024

typedef struct numlist {
  struct numlist *next;
  size_t used, tail_size;
  double data[numlist_CHUNK];
} numlist;

numlist *numlist_put(numlist *nl, const double *d, size_t len);
numlist *numlist_putn(numlist *nl, double d);

void numlist_free(numlist *nl);
size_t numlist_size(const numlist *nl);
double *numlist_get(const numlist *nl, double *out, unsigned start, size_t len);
double *numlist_get_all(const numlist *nl, double *out);
double *numlist_fetch(const numlist *nl, size_t *sizep);
double *numlist_drain(numlist *nl, size_t *sizep);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
