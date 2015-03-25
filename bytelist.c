/* bytelist.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytelist.h"
#include "util.h"

static bytelist_class me = {
  .init_size = bytelist_CHUNK,
  .max_size = bytelist_MAX,
  .terminate = 0
};

bytelist *bytelist_append_internal(bytelist *bl, const unsigned char *bytes, size_t len, const bytelist_class *class) {
  while (len) {
    if (!bl || bl->used == bl->size) {
      bytelist *nbl = alloc(sizeof(bytelist));
      nbl->size = bl ? MIN(bl->size * 2, class->max_size) : class->init_size;
      nbl->data = alloc(nbl->size);
      nbl->tail_size = bytelist_size(bl);
      nbl->class = class;
      nbl->next = bl;
      bl = nbl;
    }
    size_t avail = MIN(len, bl->size - bl->used);
    memcpy(&bl->data[bl->used], bytes, avail);
    bl->used += avail;
    bytes += avail;
    len -= avail;
  }

  return bl;
}

bytelist *bytelist_append(bytelist *bl, const unsigned char *bytes, size_t len) {
  return bytelist_append_internal(bl, bytes, len, &me);
}

void bytelist_free(bytelist *bl) {
  while (bl) {
    bytelist *next = bl->next;
    free(bl->data);
    free(bl);
    bl = next;
  }
}

size_t bytelist_size(const bytelist *bl) {
  if (!bl) return 0;
  return bl->used + bl->tail_size;
}

static unsigned _get(const bytelist *bl, unsigned char *out, unsigned end, size_t len) {
  if (!bl || !len) return 0;
  if (bl->used <= end) return _get(bl->next, out, end - bl->used, len);
  unsigned avail = MIN(bl->used - end, len);
  memcpy(out + len - avail, bl->data + bl->used - end - avail, avail);
  return avail + _get(bl->next, out, 0, len - avail);
}

unsigned char *bytelist_get(const bytelist *bl, unsigned char *out, unsigned start, size_t len) {
  (void) _get(bl, out, bytelist_size(bl) - (start + len), len);
  return out;
}

unsigned char *bytelist_get_all(const bytelist *bl, unsigned char *out) {
  return bytelist_get(bl, out, 0, bytelist_size(bl));
}

unsigned char *bytelist_fetch(const bytelist *bl, size_t *sizep) {
  size_t size = bytelist_size(bl);
  int term = bl->class->terminate ? 1 : 0;
  unsigned char *out = alloc_no_clear(sizeof(unsigned char) * size + term);
  if (term) out[size] = 0;
  bytelist_get_all(bl, out);
  if (sizep) *sizep = size;
  return out;
}

unsigned char *bytelist_drain(bytelist *bl, size_t *sizep) {
  unsigned char *data = bytelist_fetch(bl, sizep);
  bytelist_free(bl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
