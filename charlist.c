/* charlist.c */

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "charlist.h"
#include "util.h"

charlist *charlist_append(charlist *cl, const char *str, size_t len) {
  while (len) {
    if (!cl || cl->used == cl->size) {
      charlist *ncl = alloc(sizeof(charlist));
      ncl->size = cl ? MIN(cl->size * 2, charlist_MAX) : charlist_CHUNK;
      ncl->data = alloc(ncl->size);
      ncl->tail_size = charlist_size(cl);
      ncl->next = cl;
      cl = ncl;
    }
    size_t avail = MIN(len, cl->size - cl->used);
    memcpy(&cl->data[cl->used], str, avail);
    cl->used += avail;
    str += avail;
    len -= avail;
  }

  return cl;
}

void charlist_free(charlist *cl) {
  while (cl) {
    charlist *next = cl->next;
    free(cl->data);
    free(cl);
    cl = next;
  }
}

size_t charlist_size(const charlist *cl) {
  if (!cl) return 0;
  return cl->used + cl->tail_size;
}

static unsigned _get(const charlist *cl, char *out, unsigned end, size_t len) {
  if (!cl || !len) return 0;
  if (cl->used <= end) return _get(cl->next, out, end - cl->used, len);
  unsigned avail = MIN(cl->used - end, len);
  memcpy(out + len - avail, cl->data + cl->used - end - avail, avail);
  return avail + _get(cl->next, out, 0, len - avail);
}

char *charlist_get(const charlist *cl, char *out, unsigned start, size_t len) {
  (void) _get(cl, out, charlist_size(cl) - (start + len), len);
  return out;
}

char *charlist_get_all(const charlist *cl, char *out) {
  return charlist_get(cl, out, 0, charlist_size(cl));
}

char *charlist_fetch(const charlist *cl, size_t *sizep) {
  size_t size = charlist_size(cl);
  char *out = alloc(size);
  charlist_get_all(cl, out);
  if (sizep) *sizep = size;
  return out;
}

char *charlist_drain(charlist *cl, size_t *sizep) {
  char *data = charlist_fetch(cl, sizep);
  charlist_free(cl);
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
