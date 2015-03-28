/* tap2.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tap2.h"

#define MAX_PREFIX 10

typedef struct test_alert {
  struct test_alert *next;
  int tap2_test_no;
  tap2_test_cb cb;
  void *ctx;
} test_alert;

int tap2_test_no = 0;
double test_nowt = 0.00000000001;

static char *pfx[MAX_PREFIX];
static size_t npfx = 0;

static test_alert *alerts = NULL;

static int (*vfpf)(FILE *f, const char *msg, va_list ap) = vfprintf;

void tap2_set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap)) {
  vfpf = nvfpf;
}

static int tap2_fpf(FILE *f, const char *msg, ...) {
  va_list ap;
  int rc;
  va_start(ap, msg);
  rc = vfpf(f, msg, ap);
  va_end(ap);
  return rc;
}


void tap2_diag(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  tap2_fpf(stderr, "# ");
  vfpf(stderr, fmt, ap);
  tap2_fpf(stderr, "\n");
  va_end(ap);
}

static void tap2_die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfpf(stderr, fmt, ap);
  tap2_fpf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void *tap2_alloc(size_t size) {
  void *m = malloc(size);
  if (!m) tap2_die("Out of memory for %lu bytes", (unsigned long) size);
  memset(m, 0, size);
  return m;
}

static char *tap2_vssprintf(const char *fmt, va_list ap) {
  char tmp[1];
  char *buf = NULL;
  va_list ap2;

  va_copy(ap2, ap);
  int len = vsnprintf(tmp, 0, fmt, ap);
  buf = tap2_alloc(len + 1);
  vsnprintf(buf, len + 1, fmt, ap2);
  return buf;
}

static test_alert *tap2_add_alert(test_alert *list, test_alert *ta) {
  if (!list) return ta;
  if (ta->tap2_test_no < list->tap2_test_no) {
    ta->next = list;
    return ta;
  }
  list->next = tap2_add_alert(list->next, ta);
  return list;
}

void tap2_at_test(int tn, tap2_test_cb cb, void *ctx) {
  test_alert *ta = tap2_alloc(sizeof(test_alert));
  ta->tap2_test_no = tn;
  ta->cb = cb;
  ta->ctx = ctx;
  alerts = tap2_add_alert(alerts, ta);
}

static void tap2_run_alerts(int upto) {
  while (alerts && alerts->tap2_test_no <= upto) {
    test_alert *ta = alerts;
    alerts = alerts->next;
    ta->cb(ta->tap2_test_no, ta->ctx);
    free(ta);
  }
}

void tap2_done_testing(void) {
  if (0 == tap2_test_no) tap2_die("No tests run!");
  tap2_fpf(stdout, "1..%d\n", tap2_test_no);
}

int tap2_nest_in(const char *p, ...) {
  va_list ap;
  if (npfx == MAX_PREFIX) tap2_die("Too many prefixes");
  va_start(ap, p);
  pfx[npfx++] = tap2_vssprintf(p, ap);
  return 0;
}

int tap2_nest_out(void) {
  if (npfx == 0) tap2_die("Can't go out a level - we're at the top");
  free(pfx[--npfx]);
  return 0;
}

static void tap2_prefix(void) {
  unsigned i;
  for (i = 0; i < npfx; i++) {
    tap2_fpf(stdout, "%s: ", pfx[i]);
  }
}

int tap2_test(int flag, const char *msg, va_list ap) {
  tap2_fpf(stdout, "%sok %d - ", flag ? "" : "not ", ++tap2_test_no);
  tap2_prefix();
  vfpf(stdout, msg, ap);
  tap2_fpf(stdout, "\n");
  tap2_run_alerts(tap2_test_no);
  return flag;
}

int tap2_ok(int flag, const char *msg, ...) {
  tap2_TF(flag);
}

int tap2_pass(const char *msg, ...) {
  tap2_TF(1);
}

int tap2_fail(const char *msg, ...) {
  tap2_TF(0);
}

int tap2_is(long long got, long long want, const char *msg, ...) {
  tap2_TF(got == want);
}

int tap2_not_null(const void *p, const char *msg, ...) {
  tap2_TF(!!p);
}

int tap2_null(const void *p, const char *msg, ...) {
  tap2_TF(!p);
}

int tap2_within(double got, double want, double nowt, const char *msg, ...) {
  tap2_TF(fabs(got - want) <= nowt);
}

int tap2_close_to(double got, double want, const char *msg, ...) {
  tap2_TF(fabs(got - want) <= test_nowt);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
