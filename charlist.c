/* charlist.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charlist.h"
#include "util.h"

static bytelist_class me = {
  .init_size = charlist_CHUNK,
  .max_size = charlist_MAX, 
  .terminate = 1
};

charlist *charlist_append_bytes(charlist *cl, const char *str, size_t len) {
  return bytelist_append_internal(cl, (unsigned char *) str, len, &me);
}

charlist *charlist_append(charlist *cl, const char *str) {
  return charlist_append_bytes(cl, str, strlen(str));
}

void charlist_free(charlist *cl) {
  bytelist_free(cl);
}

size_t charlist_size(const charlist *cl) {
  return bytelist_size(cl);
}

char *charlist_get(const charlist *cl, char *out, unsigned start, size_t len) {
  return (char *) bytelist_get(cl, (unsigned char *) out, start, len);
}

char *charlist_get_all(const charlist *cl, char *out) {
  return charlist_get(cl, out, 0, charlist_size(cl));
}

char *charlist_fetch(const charlist *cl) {
  return (char *) bytelist_fetch(cl, NULL);
}

char *charlist_drain(charlist *cl) {
  return (char *) bytelist_drain(cl, NULL);
}

charlist *charlist_vprintf(charlist *cl, const char *fmt, va_list ap) {
  char tmp[1];
  va_list ap2;

  va_copy(ap2, ap);
  int len = vsnprintf(tmp, 0, fmt, ap);
  char buf[len + 1];
  vsnprintf(buf, len + 1, fmt, ap2);
  return charlist_append_bytes(cl, buf, len);
}

charlist *charlist_printf(charlist *cl, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  cl = charlist_vprintf(cl, fmt, ap);
  va_end(ap);
  return cl;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
