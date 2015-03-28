/* tap.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tap.h"

#define MAX_PREFIX 10

typedef struct test_alert {
  struct test_alert *next;
  int tap_test_no;
  tap_test_cb cb;
  void *ctx;
} test_alert;

typedef struct {
  char *desc;
  const char *file;
  int line;
} prefix;

int tap_test_no = 0;
double tap_test_nowt = 0.00000000001;

static prefix pfx[MAX_PREFIX];
static size_t npfx = 0;

static test_alert *alerts = NULL;

static int (*vfpf)(FILE *f, const char *msg, va_list ap) = vfprintf;

void tap_set_vfpf(int (*nvfpf)(FILE *f, const char *msg, va_list ap)) {
  vfpf = nvfpf;
}

static int tap_fpf(FILE *f, const char *msg, ...) {
  va_list ap;
  int rc;
  va_start(ap, msg);
  rc = vfpf(f, msg, ap);
  va_end(ap);
  return rc;
}


void tap_diag(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  tap_fpf(stderr, "# ");
  vfpf(stderr, fmt, ap);
  tap_fpf(stderr, "\n");
  va_end(ap);
}

static void tap_die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfpf(stderr, fmt, ap);
  tap_fpf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void *tap_alloc(size_t size) {
  void *m = malloc(size);
  if (!m) tap_die("Out of memory for %lu bytes", (unsigned long) size);
  memset(m, 0, size);
  return m;
}

static char *tap_vssprintf(const char *fmt, va_list ap) {
  char tmp[1];
  char *buf = NULL;
  va_list ap2;

  va_copy(ap2, ap);
  int len = vsnprintf(tmp, 0, fmt, ap);
  buf = tap_alloc(len + 1);
  vsnprintf(buf, len + 1, fmt, ap2);
  return buf;
}

static test_alert *tap_add_alert(test_alert *list, test_alert *ta) {
  if (!list) return ta;
  if (ta->tap_test_no < list->tap_test_no) {
    ta->next = list;
    return ta;
  }
  list->next = tap_add_alert(list->next, ta);
  return list;
}

void tap_at_test(int tn, tap_test_cb cb, void *ctx) {
  test_alert *ta = tap_alloc(sizeof(test_alert));
  ta->tap_test_no = tn;
  ta->cb = cb;
  ta->ctx = ctx;
  alerts = tap_add_alert(alerts, ta);
}

static void tap_run_alerts(int upto) {
  while (alerts && alerts->tap_test_no <= upto) {
    test_alert *ta = alerts;
    alerts = alerts->next;
    ta->cb(ta->tap_test_no, ta->ctx);
    free(ta);
  }
}

void tap_done_testing(void) {
  if (0 == tap_test_no) tap_die("No tests run!");
  tap_fpf(stdout, "1..%d\n", tap_test_no);
}

int tap__nest_in(const char *file, int line, const char *p, ...) {
  va_list ap;

  if (npfx == MAX_PREFIX) tap_die("Too many prefixes");

  va_start(ap, p);

  pfx[npfx].desc = tap_vssprintf(p, ap);
  pfx[npfx].file = file;
  pfx[npfx].line = line;

  npfx++;

  return 0;
}

int tap__nest_out(const char *file, int line) {
  (void) file;
  (void) line;
  if (npfx == 0) tap_die("Can't go out a level - we're at the top");
  free(pfx[--npfx].desc);
  return 0;
}

static void tap_prefix(void) {
  for (unsigned i = 0; i < npfx; i++) {
    tap_fpf(stdout, "%s: ", pfx[i].desc);
  }
}

int tap_test(TAP2__ARGS, int flag, const char *msg, va_list ap) {
  tap_fpf(stdout, "%sok %d - ", flag ? "" : "not ", ++tap_test_no);
  tap_prefix();
  vfpf(stdout, msg, ap);
  tap_fpf(stdout, "\n");
  tap_run_alerts(tap_test_no);
  if (!flag) {
    tap_diag("%s, line %d: Assertion %s(%s) failed.",
              file, line, name, cond);
    for (unsigned i = npfx; i-- > 0;) 
      tap_diag("  via %s, line %d: %s", pfx[i].file, pfx[i].line, pfx[i].desc);
  }
  return flag;
}

/* Test assertions */

int tap__ok(TAP2__ARGS, int flag, const char *msg, ...) {
  tap_TF(flag);
}

int tap__pass(TAP2__ARGS, const char *msg, ...) {
  tap_TF(1);
}

int tap__fail(TAP2__ARGS, const char *msg, ...) {
  tap_TF(0);
}

int tap__is(TAP2__ARGS, long long got, long long want, const char *msg, ...) {
  tap_TF(got == want);
}

int tap__not_null(TAP2__ARGS, const void *p, const char *msg, ...) {
  tap_TF(!!p);
}

int tap__null(TAP2__ARGS, const void *p, const char *msg, ...) {
  tap_TF(!p);
}

int tap__within(TAP2__ARGS, double got, double want, double nowt, const char *msg, ...) {
  tap_TF(fabs(got - want) <= nowt);
}

int tap__close_to(TAP2__ARGS, double got, double want, const char *msg, ...) {
  tap_TF(fabs(got - want) <= tap_test_nowt);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
