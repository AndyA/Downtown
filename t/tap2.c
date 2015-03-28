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

typedef struct {
  char *desc;
  const char *file;
  int line;
} prefix;

int tap2_test_no = 0;
double tap2_test_nowt = 0.00000000001;

static prefix pfx[MAX_PREFIX];
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

int tap2__nest_in(const char *file, int line, const char *p, ...) {
  va_list ap;

  if (npfx == MAX_PREFIX) tap2_die("Too many prefixes");

  va_start(ap, p);

  pfx[npfx].desc = tap2_vssprintf(p, ap);
  pfx[npfx].file = file;
  pfx[npfx].line = line;

  npfx++;

  return 0;
}

int tap2__nest_out(const char *file, int line) {
  (void) file;
  (void) line;
  if (npfx == 0) tap2_die("Can't go out a level - we're at the top");
  free(pfx[--npfx].desc);
  return 0;
}

static void tap2_prefix(void) {
  for (unsigned i = 0; i < npfx; i++) {
    tap2_fpf(stdout, "%s: ", pfx[i].desc);
  }
}

int tap2_test(TAP2__ARGS, int flag, const char *msg, va_list ap) {
  tap2_fpf(stdout, "%sok %d - ", flag ? "" : "not ", ++tap2_test_no);
  tap2_prefix();
  vfpf(stdout, msg, ap);
  tap2_fpf(stdout, "\n");
  tap2_run_alerts(tap2_test_no);
  if (!flag) {
    tap2_diag("%s, line %d: Assertion %s(%s) failed.",
              file, line, name, cond);
    for (unsigned i = npfx; i-- > 0;) 
      tap2_diag("  via %s, line %d: %s", pfx[i].file, pfx[i].line, pfx[i].desc);
  }
  return flag;
}

/* Test assertions */

int tap2__ok(TAP2__ARGS, int flag, const char *msg, ...) {
  tap2_TF(flag);
}

int tap2__pass(TAP2__ARGS, const char *msg, ...) {
  tap2_TF(1);
}

int tap2__fail(TAP2__ARGS, const char *msg, ...) {
  tap2_TF(0);
}

int tap2__is(TAP2__ARGS, long long got, long long want, const char *msg, ...) {
  tap2_TF(got == want);
}

int tap2__not_null(TAP2__ARGS, const void *p, const char *msg, ...) {
  tap2_TF(!!p);
}

int tap2__null(TAP2__ARGS, const void *p, const char *msg, ...) {
  tap2_TF(!p);
}

int tap2__within(TAP2__ARGS, double got, double want, double nowt, const char *msg, ...) {
  tap2_TF(fabs(got - want) <= nowt);
}

int tap2__close_to(TAP2__ARGS, double got, double want, const char *msg, ...) {
  tap2_TF(fabs(got - want) <= tap2_test_nowt);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
