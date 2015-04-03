/* t/json.c */

#include "framework.h"
#include "jd_pretty.h"
#include "json.h"
#include "tap.h"
#include "util.h"

static void test_get_real(void) {
  scope {
    jd_var *ar = jd_nav(100);
    size_t size;

    for (unsigned i = 0; i < 100; i++)
      jd_set_real(jd_push(ar, 1), (double) i);

    double *d = json_get_real(ar, &size);
    ok(size == 100, "size is OK");
    for (unsigned i = 0; i < 100; i++)
      ok(d[i] == (double) i, "data[%u] = %u", i, i);
  }
}

void test_main(void) {
  test_get_real();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */

