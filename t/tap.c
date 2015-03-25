/* tap.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tap.h"

#define MAX_PREFIX 10

typedef struct test_alert {
  struct test_alert *next;
  int test_no;
  test_cb cb;
  void *ctx;
} test_alert;

int test_no = 0;
double test_nowt = 0.00000000001;

static char *pfx[MAX_PREFIX];
static size_t npfx = 0;

static test_alert *alerts = NULL;

static int (*vfpf)(FILE *f, const char *msg, va_list ap) = vfprintf;

void set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap)) {
  vfpf = nvfpf;
}

static int fpf(FILE *f, const char *msg, ...) {
  va_list ap;
  int rc;
  va_start(ap, msg);
  rc = vfpf(f, msg, ap);
  va_end(ap);
  return rc;
}


void diag(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fpf(stderr, "# ");
  vfpf(stderr, fmt, ap);
  fpf(stderr, "\n");
  va_end(ap);
}

static void die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfpf(stderr, fmt, ap);
  fpf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void *alloc(size_t size) {
  void *m = malloc(size);
  if (!m) die("Out of memory for %lu bytes", (unsigned long) size);
  memset(m, 0, size);
  return m;
}

static char *vssprintf(const char *fmt, va_list ap) {
  char tmp[1];
  char *buf = NULL;
  va_list ap2;

  va_copy(ap2, ap);
  int len = vsnprintf(tmp, 0, fmt, ap);
  buf = alloc(len + 1);
  vsnprintf(buf, len + 1, fmt, ap2);
  return buf;
}

static test_alert *add_alert(test_alert *list, test_alert *ta) {
  if (!list) return ta;
  if (ta->test_no < list->test_no) {
    ta->next = list;
    return ta;
  }
  list->next = add_alert(list->next, ta);
  return list;
}

void at_test(int tn, test_cb cb, void *ctx) {
  test_alert *ta = alloc(sizeof(test_alert));
  ta->test_no = tn;
  ta->cb = cb;
  ta->ctx = ctx;
  alerts = add_alert(alerts, ta);
}

static void run_alerts(int upto) {
  while (alerts && alerts->test_no <= upto) {
    test_alert *ta = alerts;
    alerts = alerts->next;
    ta->cb(ta->test_no, ta->ctx);
    free(ta);
  }
}

void done_testing(void) {
  if (0 == test_no) die("No tests run!");
  fpf(stdout, "1..%d\n", test_no);
}

int nest_in(const char *p, ...) {
  va_list ap;
  if (npfx == MAX_PREFIX) die("Too many prefixes");
  va_start(ap, p);
  pfx[npfx++] = vssprintf(p, ap);
  return 0;
}

int nest_out(void) {
  if (npfx == 0) die("Can't go out a level - we're at the top");
  free(pfx[--npfx]);
  return 0;
}

static void prefix(void) {
  unsigned i;
  for (i = 0; i < npfx; i++) {
    fpf(stdout, "%s: ", pfx[i]);
  }
}

int test(int flag, const char *msg, va_list ap) {
  fpf(stdout, "%sok %d - ", flag ? "" : "not ", ++test_no);
  prefix();
  vfpf(stdout, msg, ap);
  fpf(stdout, "\n");
  run_alerts(test_no);
  return flag;
}

int ok(int flag, const char *msg, ...) {
  TF(flag);
}

int pass(const char *msg, ...) {
  TF(1);
}

int fail(const char *msg, ...) {
  TF(0);
}

int is(long long got, long long want, const char *msg, ...) {
  TF(got == want);
}

int not_null(const void *p, const char *msg, ...) {
  TF(!!p);
}

int null(const void *p, const char *msg, ...) {
  TF(!p);
}

int within(double got, double want, double nowt, const char *msg, ...) {
  TF(fabs(got - want) <= nowt);
}

int close_to(double got, double want, const char *msg, ...) {
  TF(fabs(got - want) <= test_nowt);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
