/* util.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

void die(const char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  fprintf(stderr, "Fatal :");
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

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
