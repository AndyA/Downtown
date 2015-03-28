/* tap.h */

#ifndef __TAP2_H
#define __TAP2_H

#include <stdio.h>
#include <stdarg.h>

extern int tap_test_no;
extern double tap_test_nowt;

void tap_set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap));
void tap_diag(const char *fmt, ...);
void tap_done_testing(void);
int tap__nest_in(const char *file, int line, const char *p, ...);
int tap__nest_out(const char *file, int line);

#define TAP2__ARGS  const char *name, const char *cond, const char *file, int line
#define TAP2__ARGV  name, cond, file, line

int tap__test(TAP2__ARGS, int flag, const char *msg, va_list ap);

/* Test assertions */

int tap__ok(TAP2__ARGS, int flag, const char *msg, ...);
int tap__pass(TAP2__ARGS, const char *msg, ...);
int tap__fail(TAP2__ARGS, const char *msg, ...);
int tap__is(TAP2__ARGS, long long got, long long want, const char *msg, ...);
int tap__not_null(TAP2__ARGS, const void *p, const char *msg, ...);
int tap__null(TAP2__ARGS, const void *p, const char *msg, ...);
int tap__within(TAP2__ARGS, double got, double want, double nowt, const char *msg, ...);
int tap__close_to(TAP2__ARGS, double got, double want, const char *msg, ...);

/* Test callback */

typedef void (*tap_test_cb)(int tn, void *ctx);

void tap_at_test(int tn, tap_test_cb cb, void *ctx);

#define tap_TF(flag)                     \
  va_list ap;                            \
  int _c = (flag);                       \
  va_start( ap, msg );                   \
  tap__test( TAP2__ARGV, _c, msg, ap );  \
  va_end( ap );                          \
  return _c;

#endif

#define tap__SPLICE(a, b)    a ## b
#define tap__PASTE(a, b)     tap__SPLICE(a, b)

#define tap_ok(...)         tap__ok(       "ok",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_pass(...)       tap__pass(     "pass",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_fail(...)       tap__fail(     "fail",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_is(...)         tap__is(       "is",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_not_null(...)   tap__not_null( "not_null",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_null(...)       tap__null(     "null",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_within(...)     tap__within(   "within",    # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_close_to(...)   tap__close_to( "close_to",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap_nest_in(...)    tap__nest_in(  __FILE__, __LINE__, __VA_ARGS__ )
#define tap_nest_out(...)   tap__nest_out( __FILE__, __LINE__ )

#if !defined(TAP2_NO_ALIAS)

#define ok(...)              tap__ok(       "ok",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define pass(...)            tap__pass(     "pass",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define fail(...)            tap__fail(     "fail",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define is(...)              tap__is(       "is",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define not_null(...)        tap__not_null( "not_null",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define null(...)            tap__null(     "null",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define within(...)          tap__within(   "within",    # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define close_to(...)        tap__close_to( "close_to",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define nest_in(...)         tap__nest_in(  __FILE__, __LINE__, __VA_ARGS__ )
#define nest_out(...)        tap__nest_out( __FILE__, __LINE__ )

#define set_vfpf(...)        tap_set_vfpf(     __VA_ARGS__ )
#define diag(...)            tap_diag(         __VA_ARGS__ )
#define done_testing(...)    tap_done_testing( __VA_ARGS__ )

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
