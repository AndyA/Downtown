/* charlist.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charlist.h"
#include "util.h"

#define charlist_CHUNK 128
#define charlist_MAX   (1024*1024)

bytelist_DEFINE(charlist, char, charlist_CHUNK, charlist_MAX, bytelist_TERMINATED)

charlist *charlist_puts(charlist *cl, const char *str) {
  return charlist_append(cl, str, strlen(str));
}

charlist *charlist_vprintf(charlist *cl, const char *fmt, va_list ap) {
  char tmp[1];
  va_list ap2;

  va_copy(ap2, ap);
  int len = vsnprintf(tmp, 0, fmt, ap);
  char buf[len + 1];
  vsnprintf(buf, len + 1, fmt, ap2);
  return charlist_append(cl, buf, len);
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
