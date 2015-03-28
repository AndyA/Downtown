/* tap2.h */

#ifndef __TAP2_H
#define __TAP2_H

#include <stdio.h>
#include <stdarg.h>

extern int tap2_test_no;
extern double tap2_test_nowt;

void tap2_set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap));
void tap2_diag(const char *fmt, ...);
void tap2_done_testing(void);
int tap2__nest_in(const char *file, int line, const char *p, ...);
int tap2__nest_out(const char *file, int line);

#define TAP2__ARGS  const char *name, const char *cond, const char *file, int line
#define TAP2__ARGV  name, cond, file, line

int tap2_test(TAP2__ARGS, int flag, const char *msg, va_list ap);

/* Test assertions */

int tap2__ok(TAP2__ARGS, int flag, const char *msg, ...);
int tap2__pass(TAP2__ARGS, const char *msg, ...);
int tap2__fail(TAP2__ARGS, const char *msg, ...);
int tap2__is(TAP2__ARGS, long long got, long long want, const char *msg, ...);
int tap2__not_null(TAP2__ARGS, const void *p, const char *msg, ...);
int tap2__null(TAP2__ARGS, const void *p, const char *msg, ...);
int tap2__within(TAP2__ARGS, double got, double want, double nowt, const char *msg, ...);
int tap2__close_to(TAP2__ARGS, double got, double want, const char *msg, ...);

/* Test callback */

typedef void (*tap2_test_cb)(int tn, void *ctx);

void tap2_at_test(int tn, tap2_test_cb cb, void *ctx);

#define tap2_TF(flag)                    \
  va_list ap;                            \
  int _c = (flag);                       \
  va_start( ap, msg );                   \
  tap2_test( TAP2__ARGV, _c, msg, ap );  \
  va_end( ap );                          \
  return _c;

#endif

#define tap2__SPLICE(a, b)    a ## b
#define tap2__PASTE(a, b)     tap2__SPLICE(a, b)

/*#define tap2__DEF_ASSERT(*/

#ifdef TAP2_NO_ALIAS

#define tap2_ok(...)         tap2__ok(       "ok",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_pass(...)       tap2__pass(     "pass",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_fail(...)       tap2__fail(     "fail",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_is(...)         tap2__is(       "is",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_not_null(...)   tap2__not_null( "not_null",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_null(...)       tap2__null(     "null",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_within(...)     tap2__within(   "within",    # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_close_to(...)   tap2__close_to( "close_to",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_nest_in(...)    tap2__nest_in(  __FILE__, __LINE__, __VA_ARGS__ )
#define tap2_nest_out(...)   tap2__nest_out( __FILE__, __LINE__ )
                             
#else                        
                             
#define ok(...)              tap2__ok(       "ok",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define pass(...)            tap2__pass(     "pass",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define fail(...)            tap2__fail(     "fail",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define is(...)              tap2__is(       "is",        # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define not_null(...)        tap2__not_null( "not_null",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define null(...)            tap2__null(     "null",      # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define within(...)          tap2__within(   "within",    # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define close_to(...)        tap2__close_to( "close_to",  # __VA_ARGS__, __FILE__, __LINE__, __VA_ARGS__ )
#define nest_in(...)         tap2__nest_in(  __FILE__, __LINE__, __VA_ARGS__ )
#define nest_out(...)        tap2__nest_out( __FILE__, __LINE__ )

#define set_vfpf(...)        tap2_set_vfpf(     __VA_ARGS__ )
#define diag(...)            tap2_diag(         __VA_ARGS__ )
#define done_testing(...)    tap2_done_testing( __VA_ARGS__ )

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
