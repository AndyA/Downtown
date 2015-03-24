/* util.c */

#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "log.h"
#include "util.h"

void die(const char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  log_out(FATAL, msg, ap);
  va_end(ap);
  exit(1);
}

void *alloc_no_clear(size_t size) {
  void *m = malloc(size);
  if (!m) die("Out of memory for %lu bytes", (unsigned long) size);
  return m;
}

void *alloc(size_t size) {
  void *m = alloc_no_clear(size);
  memset(m, 0, size);
  return m;
}

void *memdup(const void *in, size_t size) {
  void *m = alloc_no_clear(size);
  memcpy(m, in, size);
  return m;
}

char *sstrdup(const char *s) {
  if (!s) return NULL;
  size_t sz = strlen(s) + 1;
  char *ss = alloc(sz);
  memcpy(ss, s, sz);
  return ss;
}

char *vssprintf(const char *fmt, va_list ap) {
  char tmp[1];
  char *buf = NULL;
  va_list ap2;

  va_copy(ap2, ap);
  int len = vsnprintf(tmp, 0, fmt, ap);
  buf = alloc(len + 1);
  vsnprintf(buf, len + 1, fmt, ap2);
  return buf;
}

char *ssprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char *buf = vssprintf(fmt, ap);
  va_end(ap);
  return buf;
}

int gcd(int a, int b) {
  int c;
  while (a != 0) {
    c = a;
    a = b % a;
    b = c;
  }
  return b;
}

char *aspect_ratio(int w, int h) {
  int g = gcd(w, h);
  return ssprintf("%d:%d", w / g, h / g);
}

void mkpath(const char *path, mode_t mode) {
  if (mkdir(path, mode) == 0 || errno == EEXIST) return;
  char *pcopy = sstrdup(path);
  char *parent = sstrdup(dirname(pcopy));
  mkpath(parent, mode);
  free(parent);
  free(pcopy);
  if (mkdir(path, mode) && errno != EEXIST)
    die("Can't create %s: %s", strerror(errno));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
