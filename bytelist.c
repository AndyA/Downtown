/* bytelist.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytelist.h"
#include "util.h"

static bytelist_class me = {
  .init_size   = bytelist_CHUNK,
  .max_size    = bytelist_MAX,
  .rise_rate   = 2,
  .member_size = sizeof(unsigned char),
  .options = 0
};

size_t _size(const bytelist *bl) {
  if (!bl) return 0;
  return bl->used + bl->tail_size;
}

// For tests

bytelist_class *bytelist__get_class(void) {
  return &me;
}

size_t bytelist_member_size(const bytelist *bl) {
  if (!bl) return 0;
  return bl->clazz->member_size;
}

static bytelist *_new(const bytelist_class *clazz) {
  bytelist *nbl = alloc(sizeof(bytelist));
  nbl->clazz = clazz;
  nbl->next_size = clazz->init_size * clazz->member_size;
  return nbl;
}

static bytelist *_append(bytelist *bl, const unsigned char *bytes, size_t len, const bytelist_class *clazz) {
  while (len) {
    if (!bl || bl->used == bl->size) {
      bytelist *nbl = _new(clazz);

      size_t init_size = clazz->init_size * clazz->member_size;
      size_t next_size = (bl ? MIN(bl->next_size, clazz->max_size) : init_size);

      if (len > next_size) {
        nbl->size = len;
        nbl->next_size = next_size;
      }
      else {
        nbl->size = next_size;
        nbl->next_size = nbl->size * clazz->rise_rate;
      }

      nbl->clazz = clazz;
      nbl->data = alloc(nbl->size);
      nbl->tail_size = _size(bl);
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

bytelist *bytelist_extract(const bytelist *bl, unsigned pos, size_t len) {
  if (!bl) return NULL;

  size_t size = bytelist_size(bl);
  if (pos >= size) return NULL;
  if (pos + len > size) len = size - pos;

  bytelist *nbl = _new(bl->clazz);
  nbl->size = nbl->used = len * bl->clazz->member_size;;
  nbl->data = alloc_no_clear(nbl->size);

  bytelist_get(bl, nbl->data, pos, len);

  return nbl;
}

bytelist *bytelist_clone(const bytelist *bl) {
  return bytelist_extract(bl, 0, bytelist_size(bl));
}

bytelist *bytelist_defrag(bytelist *bl) {
  if (!bl || !bl->next) return bl;
  bytelist *nbl = bytelist_clone(bl);
  bytelist_free(bl);
  return nbl;
}

static void _reverse_buf(bytelist *bl) {
  if (!bl) return;

  size_t msize = bl->clazz->member_size;
  unsigned char tmp[bl->used];

  for (unsigned char *pa = bl->data, *pb = bl->data + bl->used;
       pa < (pb -= msize);
       pa += msize) {
    memcpy(tmp, pa, msize);
    memcpy(pa, pb, msize);
    memcpy(pb, tmp, msize);
  }
}

static bytelist *_reverse(bytelist *bl, bytelist *next) {
  if (!bl) return NULL;
  _reverse_buf(bl);
  bytelist *old_next = bl->next;
  bl->next = next;
  if (old_next) return _reverse(old_next, bl);
  return bl;
}

bytelist *bytelist_reverse(bytelist *bl) {
  return _reverse(bl, NULL);
}

bytelist *bytelist_buffer(bytelist *bl, unsigned char **bufp, size_t *sizep) {
  bl = bytelist_defrag(bl);
  if (bufp)  *bufp  = bl->data;
  if (sizep) *sizep = bytelist_size(bl);
  return bl;
}

bytelist *bytelist_qsort(bytelist *bl, int (*compar)(const void *, const void *)) {
  unsigned char *buf;
  size_t size;

  bl = bytelist_buffer(bl, &buf, &size);
  qsort(buf, size, bl->clazz->member_size, compar);

  return bl;
}

void *bytelist_bsearch(const bytelist *bl, const unsigned char *key, int (*compar)(const void *, const void *)) {
  while (bl && (bl->used == 0 || compar(bl->data, key) > 0)) bl = bl->next;
  if (!bl) return NULL;
  return bsearch(key, bl->data, bl->used / bl->clazz->member_size, bl->clazz->member_size, compar);
}

bytelist *_join(bytelist *bl, bytelist *bl2, size_t grow) {
  if (!bl) return bl2;
  bl->tail_size += grow;
  bl->next = _join(bl->next, bl2, grow);
  return bl;
}

bytelist *bytelist_join(bytelist *bl, bytelist *bl2) {
  return _join(bl2, bl, _size(bl));
}

static bytelist *_split(bytelist *bl, unsigned pos, size_t shrink, bytelist **blb) {
  if (!bl) return NULL;

  if (pos >= bl->used) {
    bl->tail_size -= shrink;
    bl->next = _split(bl->next, pos - bl->used, shrink, blb);
    return bl;
  }

  *blb = _append(bl->next, bl->data, bl->used - pos, bl->clazz);
  memmove(bl->data, bl->data + bl->used - pos, pos);

  bl->used = pos;
  bl->tail_size = 0;
  bl->next = NULL;

  return bl;
}

static void _native_split(bytelist *bl, unsigned pos, size_t shrink, bytelist **bla, bytelist **blb) {
  if (!bl || pos == 0) {
    *bla = bl;
    *blb = NULL;
    return;
  }

  size_t size = _size(bl);
  if (pos >= size) {
    *bla = NULL;
    *blb = bl;
    return;
  }

  *blb = _split(bl, pos, shrink, bla);
}

void bytelist_split(bytelist *bl, unsigned pos, bytelist **bla, bytelist **blb) {
  pos *= bl->clazz->member_size;
  _native_split(bl, _size(bl) - pos, pos, bla, blb);
}

bytelist *bytelist_append_internal(bytelist *bl, const unsigned char *bytes, size_t len, const bytelist_class *clazz) {
  return _append(bl, bytes, len * clazz->member_size, clazz);
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
  return _size(bl) / bl->clazz->member_size;
}

static unsigned _get(const bytelist *bl, unsigned char *out, unsigned end, size_t len) {
  if (!bl || !len) return 0;
  if (bl->used <= end) return _get(bl->next, out, end - bl->used, len);
  unsigned avail = MIN(bl->used - end, len);
  memcpy(out + len - avail, bl->data + bl->used - end - avail, avail);
  return avail + _get(bl->next, out, 0, len - avail);
}

unsigned char *bytelist_get(const bytelist *bl, unsigned char *out, unsigned start, size_t len) {
  start *= bl->clazz->member_size;
  len   *= bl->clazz->member_size;
  (void) _get(bl, out, _size(bl) - (start + len), len);
  return out;
}

unsigned char *bytelist_get_all(const bytelist *bl, unsigned char *out) {
  return bytelist_get(bl, out, 0, bytelist_size(bl));
}

unsigned char *bytelist_fetch(const bytelist *bl, size_t *sizep) {
  size_t size = _size(bl);
  int term = (bl->clazz->options & bytelist_TERMINATED) ? 1 : 0;
  unsigned char *out = alloc_no_clear(size + term);
  if (term) out[size] = 0;
  bytelist_get_all(bl, out);
  if (sizep) *sizep = size / bl->clazz->member_size;;
  return out;
}

unsigned char *bytelist_drain(bytelist *bl, size_t *sizep) {
  unsigned char *data = bytelist_fetch(bl, sizep);
  bytelist_free(bl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
