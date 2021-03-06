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

bytelist *bytelist_reverse(bytelist *bl) {
  bytelist *nbl = NULL;

  while (bl) {
    _reverse_buf(bl);
    bytelist *next = bl->next;
    bl->next = nbl;
    nbl = bl;
    bl = next;
  }

  return nbl;
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

bytelist *bytelist_join(bytelist *bl, bytelist *bl2) {
  if (!bl2) return bl;
  if (!bl) return bl2;

  /* actually add bl onto bl2 */
  size_t adjust = _size(bl);
  bytelist *blp = bl2;
  for (;;) {
    blp->tail_size += adjust;
    if (!blp->next) break;
    blp = blp->next;
  }

  blp->next = bl;
  return bl2;
}

static bytelist *_split(bytelist *bl, unsigned pos, size_t shrink, bytelist **blb) {
  bytelist *blp = bl;

  while (blp && pos >= blp->used) {
    blp->tail_size -= shrink;
    pos -= blp->used;
    blp = blp->next;
  }

  if (!blp) return bl;

  *blb = _append(blp->next, blp->data, blp->used - pos, blp->clazz);
  memmove(blp->data, blp->data + blp->used - pos, pos);

  blp->used = pos;
  blp->tail_size = 0;
  blp->next = NULL;

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
  if (!len) return 0;

  while (bl && end >= bl->used) {
    end -= bl->used;
    bl = bl->next;
  }

  size_t got = 0;

  while (bl && len > 0) {
    unsigned avail = MIN(bl->used - end, len);
    memcpy(out + len - avail, bl->data + bl->used - end - avail, avail);
    got += avail;
    end = 0;
    len -= avail;
    bl = bl->next;
  }

  return got;
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
  unsigned term = (bl->clazz->options & bytelist_TERMINATED) ? bl->clazz->member_size : 0;
  unsigned char *out = alloc_no_clear(size + term);
  memset(out + size, 0, term);
  bytelist_get_all(bl, out);
  if (sizep) *sizep = size / bl->clazz->member_size;;
  return out;
}

unsigned char *bytelist_drain(bytelist *bl, size_t *sizep) {
  if (sizep) *sizep = bytelist_size(bl);
  unsigned term = (bl->clazz->options & bytelist_TERMINATED) ? bl->clazz->member_size : 0;
  if (term) {
    /* since we're going to throw it away... */
    unsigned char zero[term];
    memset(zero, 0, term);
    bl = bytelist_append(bl, zero, 1);
  }
  bl = bytelist_defrag(bl);
  unsigned char *data = bl->data;
  bl->data = NULL;
  bytelist_free(bl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
