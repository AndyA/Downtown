/* util.h */

#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

  void die(const char *msg, ...);
  void *alloc(size_t size);
  char *sstrdup(const char *s);
  char *vssprintf(const char *fmt, va_list ap);
  char *ssprintf(const char *fmt, ...);
  int gcd(int a, int b);
  char *aspect_ratio(int w, int h);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
