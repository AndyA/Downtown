/* t/bytelist.c */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "bytelist.h"
#include "tap.h"
#include "util.h"

#define CHUNK bytelist_CHUNK
/*#define CHUNK 128*/

static bytelist *stuff_list(bytelist *bl, size_t len, int base) {
  unsigned char bytes[len];
  for (unsigned i = 0; i < len; i++)
    bytes[i] = (unsigned char) base + i;
  return bytelist_append(bl, (const unsigned char *) bytes, len);
}

static int check_sequence(const unsigned char *bytes, size_t len, int base) {
  for (unsigned i = 0; i < len; i++)
    if ((unsigned char) bytes[i] != (unsigned char)(base + i)) return (int) i;
  return -1;
}

static bytelist *check_list(bytelist *bl, size_t expect, int n) {
  bl = stuff_list(bl, n, expect + 1);
  expect += n;
  nest_in("added %d (total %llu)", n, (unsigned long long) expect);

  ok(!!bl, "bl != NULL");
  ok(bytelist_size(bl) == expect, "size is %llu", (unsigned long long) expect);

  size_t size;
  unsigned char *d = bytelist_fetch(bl, &size);
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
  return bl;
}

static void test_bytelist(void) {
  nest_in("non-aligned");
  bytelist *bl = NULL;

  ok(bytelist_size(bl) == 0, "NULL: size == 0");

  size_t expect = 0;

  for (int n = 1; n < CHUNK * 3; n += 313) {
    bl = check_list(bl, expect, n);
    expect += n;
  }

  bytelist_free(bl);
  nest_out();
}

static bytelist *check_al(bytelist *bl, size_t *expect, int delta, int count) {
  for (int n = 0; n < count; n++) {
    int past = (int) * expect % CHUNK;
    nest_in("%d past boundary", past);
    bl = check_list(bl, *expect, CHUNK + delta);
    *expect += CHUNK + delta;
    nest_out();
  }
  return bl;
}

static void test_aligned(void) {
  nest_in("aligned");
  bytelist *bl = NULL;

  ok(bytelist_size(bl) == 0, "NULL: size == 0");

  size_t expect = 0;
  bl = check_al(bl, &expect, -1, 5);
  bl = check_al(bl, &expect, 1, 10);
  bl = check_al(bl, &expect, -1, 10);
  bl = check_al(bl, &expect, 1, 5);

  bytelist_free(bl);
  nest_out();
}

static bytelist *put1(bytelist *bl, unsigned char c) {
  return bytelist_append(bl, &c, 1);
}

static void test_get(void) {
  bytelist *bl = NULL;

  const int count = 1000;

  for (int i = 0; i < count; i++)
    bl = put1(bl, i + 1);

  size_t len = 1;
  for (int pos = 0; pos < count;) {
    size_t want = MIN(len, (unsigned) count - pos);
    unsigned char out[want];

    unsigned char *got = bytelist_get(bl, out, pos, want);

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
#if defined(TEST_INIT_SIZE) || defined(TEST_RISE_RATE)
  bytelist_class *me = bytelist__get_class();
#if defined(TEST_INIT_SIZE)
  me->init_size = TEST_INIT_SIZE;
#endif
#if defined(TEST_RISE_RATE)
  me->rise_rate = TEST_RISE_RATE;
#endif
  diag("init_size=%u, rise_rate=%u", (unsigned) me->init_size, me->rise_rate);
#endif
  test_aligned();
  test_bytelist();
  test_get();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
