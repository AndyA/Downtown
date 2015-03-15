/* util.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void die(const char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  fprintf(stderr, "Fatal: ");
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void *alloc(size_t size) {
  void *m = malloc(size);
  if (!m) die("Out of memory for %lu bytes", (unsigned long) size);
  memset(m, 0, size);
  return m;
}

char *sstrdup(const char *s) {
  if (!s) return NULL;
  size_t sz = strlen(s) + 1;
  char *ss = alloc(sz);
  memcpy(ss, s, sz);
  return ss;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
