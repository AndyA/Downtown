/* tap2.h */

#ifndef __TAP2_H
#define __TAP2_H

#include <stdio.h>
#include <stdarg.h>

extern int tap2_test_no;
extern double tap2_test_nowt;

void tap2_set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap));
int tap2_test(int flag, const char *msg, va_list ap);
void tap2_diag(const char *fmt, ...);
void tap2_done_testing(void);
int tap2_nest_in(const char *p, ...);
int tap2_nest_out(void);
int tap2_ok(int flag, const char *msg, ...);
int tap2_pass(const char *msg, ...);
int tap2_fail(const char *msg, ...);
int tap2_is(long long got, long long want, const char *msg, ...);
int tap2_not_null(const void *p, const char *msg, ...);
int tap2_null(const void *p, const char *msg, ...);
int tap2_within(double got, double want, double nowt, const char *msg, ...);
int tap2_close_to(double got, double want, const char *msg, ...);

typedef void (*tap2_test_cb)(int tn, void *ctx);

void tap2_at_test(int tn, tap2_test_cb cb, void *ctx);

#define tap2_TF(flag)        \
  va_list ap;                \
  int _c = (flag);           \
  va_start( ap, msg );       \
  tap2_test( _c, msg, ap );  \
  va_end( ap );              \
  return _c;

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
