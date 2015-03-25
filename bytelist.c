/* bytelist.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytelist.h"
#include "util.h"

bytelist *bytelist_append_internal(bytelist *cl, const unsigned char *bytes, size_t len, size_t min_chunk) {
  while (len) {
    if (!cl || cl->used == cl->size) {
      bytelist *ncl = alloc(sizeof(bytelist));
      ncl->size = cl ? MIN(cl->size * 2, bytelist_MAX) : min_chunk;
      ncl->data = alloc(ncl->size);
      ncl->tail_size = bytelist_size(cl);
      ncl->next = cl;
      cl = ncl;
    }
    size_t avail = MIN(len, cl->size - cl->used);
    memcpy(&cl->data[cl->used], bytes, avail);
    cl->used += avail;
    bytes += avail;
    len -= avail;
  }

  return cl;
}

bytelist *bytelist_append(bytelist *cl, const unsigned char *bytes, size_t len) {
  return bytelist_append_internal(cl, bytes, len, bytelist_CHUNK);
}

void bytelist_free(bytelist *cl) {
  while (cl) {
    bytelist *next = cl->next;
    free(cl->data);
    free(cl);
    cl = next;
  }
}

size_t bytelist_size(const bytelist *cl) {
  if (!cl) return 0;
  return cl->used + cl->tail_size;
}

static unsigned _get(const bytelist *cl, unsigned char *out, unsigned end, size_t len) {
  if (!cl || !len) return 0;
  if (cl->used <= end) return _get(cl->next, out, end - cl->used, len);
  unsigned avail = MIN(cl->used - end, len);
  memcpy(out + len - avail, cl->data + cl->used - end - avail, avail);
  return avail + _get(cl->next, out, 0, len - avail);
}

unsigned char *bytelist_get(const bytelist *cl, unsigned char *out, unsigned start, size_t len) {
  (void) _get(cl, out, bytelist_size(cl) - (start + len), len);
  return out;
}

unsigned char *bytelist_get_all(const bytelist *cl, unsigned char *out) {
  return bytelist_get(cl, out, 0, bytelist_size(cl));
}

unsigned char *bytelist_fetch(const bytelist *cl, size_t *sizep) {
  size_t size = bytelist_size(cl);
  unsigned char *out = alloc(sizeof(unsigned char) * size);
  bytelist_get_all(cl, out);
  if (sizep) *sizep = size;
  return out;
}

unsigned char *bytelist_drain(bytelist *cl, size_t *sizep) {
  unsigned char *data = bytelist_fetch(cl, sizep);
  bytelist_free(cl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
