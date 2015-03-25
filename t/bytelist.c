/* t/bytelist.c */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "bytelist.h"
#include "tap.h"
#include "util.h"

#define CHUNK bytelist_CHUNK
/*#define CHUNK 128*/

static bytelist *stuff_list(bytelist *cl, size_t len, int base) {
  unsigned char bytes[len];
  for (unsigned i = 0; i < len; i++)
    bytes[i] = (unsigned char) base + i;
  return bytelist_append(cl, (const unsigned char *) bytes, len);
}

static int check_sequence(const unsigned char *bytes, size_t len, int base) {
  for (unsigned i = 0; i < len; i++)
    if ((unsigned char) bytes[i] != (unsigned char)(base + i)) return (int) i;
  return -1;
}

static bytelist *check_list(bytelist *cl, size_t expect, int n) {
  cl = stuff_list(cl, n, expect + 1);
  expect += n;
  nest_in("added %d (total %llu)", n, (unsigned long long) expect);

  ok(!!cl, "cl != NULL");
  ok(bytelist_size(cl) == expect, "size is %llu", (unsigned long long) expect);

  size_t size;
  unsigned char *d = bytelist_fetch(cl, &size);
  ok(size == expect, "returned size is correct");
  int seq = check_sequence(d, expect, 1);
  if (!ok(seq == -1, "data is correct")) {
    diag("Data starts to differ at offset %d", seq);
    unsigned char want = (unsigned char)(seq + 1);
    for (int i = 0; i < MIN((int) expect, 10); i++) {
      diag("wanted %d, got %d", want, (unsigned char) d[i]);
      want++;
    }
  }
  free(d);

  nest_out();
  return cl;
}

static void test_bytelist(void) {
  nest_in("non-aligned");
  bytelist *cl = NULL;

  ok(bytelist_size(cl) == 0, "NULL: size == 0");

  size_t expect = 0;

  for (int n = 1; n < CHUNK * 3; n += 313) {
    cl = check_list(cl, expect, n);
    expect += n;
  }

  bytelist_free(cl);
  nest_out();
}

static bytelist *check_al(bytelist *cl, size_t *expect, int delta, int count) {
  for (int n = 0; n < count; n++) {
    int past = (int) * expect % CHUNK;
    nest_in("%d past boundary", past);
    cl = check_list(cl, *expect, CHUNK + delta);
    *expect += CHUNK + delta;
    nest_out();
  }
  return cl;
}

static void test_aligned(void) {
  nest_in("aligned");
  bytelist *cl = NULL;

  ok(bytelist_size(cl) == 0, "NULL: size == 0");

  size_t expect = 0;
  cl = check_al(cl, &expect, -1, 5);
  cl = check_al(cl, &expect, 1, 10);
  cl = check_al(cl, &expect, -1, 10);
  cl = check_al(cl, &expect, 1, 5);

  bytelist_free(cl);
  nest_out();
}

static bytelist *put1(bytelist *cl, unsigned char c) {
  return bytelist_append(cl, &c, 1);
}

static void test_get(void) {
  bytelist *cl = NULL;

  const int count = 1000;

  for (int i = 0; i < count; i++)
    cl = put1(cl, i + 1);

  size_t len = 1;
  for (int pos = 0; pos < count;) {
    size_t want = MIN(len, (unsigned) count - pos);
    unsigned char out[want];

    unsigned char *got = bytelist_get(cl, out, pos, want);

    nest_in("get, want=%u", (unsigned) want);

    ok(out == got, "returned correct pointer");

    int seq = check_sequence(out, want, pos + 1);
    if (!ok(seq == -1, "data is correct")) {
      diag("Data starts to differ at offset %d", seq);
      unsigned char w = (unsigned char)(seq + 1);
      for (int i = 0; i < MIN((int) want, 10); i++) {
        diag("wanted %d, got %d", w, (unsigned char) out[i]);
        w++;
      }
    }

    nest_out();

    pos += len;
    len++;
  }

}

void test_main(void) {
  test_aligned();
  test_bytelist();
  test_get();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
