/* tap2.h */

#ifndef __TAP2_H
#define __TAP2_H

#include <stdio.h>
#include <stdarg.h>

extern int tap2_test_no;
extern double tap2_test_nowt;

void tap2_set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap));
int tap2_test(const char *file, int line, int flag, const char *msg, va_list ap);
void tap2_diag(const char *fmt, ...);
void tap2_done_testing(void);
int tap2_nest_in(const char *p, ...);
int tap2_nest_out(void);

/* Test assertions */

int tap2__ok(const char *file, int line, int flag, const char *msg, ...);
int tap2__pass(const char *file, int line, const char *msg, ...);
int tap2__fail(const char *file, int line, const char *msg, ...);
int tap2__is(const char *file, int line, long long got, long long want, const char *msg, ...);
int tap2__not_null(const char *file, int line, const void *p, const char *msg, ...);
int tap2__null(const char *file, int line, const void *p, const char *msg, ...);
int tap2__within(const char *file, int line, double got, double want, double nowt, const char *msg, ...);
int tap2__close_to(const char *file, int line, double got, double want, const char *msg, ...);

/* Test callback */

typedef void (*tap2_test_cb)(int tn, void *ctx);

void tap2_at_test(int tn, tap2_test_cb cb, void *ctx);

#define tap2_TF(flag)                    \
  va_list ap;                            \
  int _c = (flag);                       \
  va_start( ap, msg );                   \
  tap2_test( file, line, _c, msg, ap );  \
  va_end( ap );                          \
  return _c;

#endif

#ifdef TAP2_NO_ALIAS

#define tap2_ok(...)         tap2__ok(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_pass(...)       tap2__pass(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_fail(...)       tap2__fail(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_is(...)         tap2__is(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_not_null(...)   tap2__not_null(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_null(...)       tap2__null(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_within(...)     tap2__within(__FILE__, __LINE__, __VA_ARGS__)
#define tap2_close_to(...)   tap2__close_to(__FILE__, __LINE__, __VA_ARGS__)

#else

#define ok(...)              tap2__ok(__FILE__, __LINE__, __VA_ARGS__)
#define pass(...)            tap2__pass(__FILE__, __LINE__, __VA_ARGS__)
#define fail(...)            tap2__fail(__FILE__, __LINE__, __VA_ARGS__)
#define is(...)              tap2__is(__FILE__, __LINE__, __VA_ARGS__)
#define not_null(...)        tap2__not_null(__FILE__, __LINE__, __VA_ARGS__)
#define null(...)            tap2__null(__FILE__, __LINE__, __VA_ARGS__)
#define within(...)          tap2__within(__FILE__, __LINE__, __VA_ARGS__)
#define close_to(...)        tap2__close_to(__FILE__, __LINE__, __VA_ARGS__)

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
