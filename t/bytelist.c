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

static unsigned char char_for(unsigned pos) {
  return (unsigned char)(
           ((pos >>  0) *  1) ^
           ((pos >>  7) * 11) ^
           ((pos >>  9) *  2) ^
           ((pos >> 11) * 19) ^
           ((pos >> 15) *  3) ^
           ((pos >> 19) * 27) ^
           ((pos >> 25) *  5) ^
           ((pos >> 30) * 22) ^
           ((pos >> 31) *  9)
         );
}

static bytelist *append_some(bytelist *bl, size_t some, size_t adjust) {
  unsigned char buf[some];
  size_t got = adjust + bytelist_size(bl);
  for (unsigned i = 0; i < some; i++) buf[i] = char_for(got + i);
  return bytelist_append(bl, buf, some);
}

static int randum(int limit) {
  int mask = 1;
  while (mask < limit) mask <<= 1;
  for (;;) {
    int rr = rand() & (mask - 1);
    if (rr < limit) return rr;
  }
}

static bytelist *append_about(bytelist *bl, size_t about, size_t adjust) {
  while (bytelist_size(bl) < about)
    bl = append_some(bl, randum(10) * randum(10) + 1, adjust);
  return bl;
}

static int check_sanity(const bytelist *bl, size_t adjust, size_t chunk) {
  unsigned char buf[chunk];

  size_t size = bytelist_size(bl);
  for (unsigned pos = 0; pos < size; pos += chunk) {
    size_t avail = MIN(chunk, size - pos);
    bytelist_get(bl, buf, pos, avail);
    for (unsigned i = 0; i < avail; i++) {
      unsigned char want = char_for(adjust + pos + i);
      if (buf[i] != want) return (int) pos + i;
    }
  }
  return -1;
}

static int sane_list_c(const bytelist *bl, size_t adjust, size_t chunk) {
  size_t size = bytelist_size(bl);
  int bad_pos = check_sanity(bl, adjust, chunk);

  if (ok(bad_pos == -1,
         "all %u bytes (%u at a time) have the expected value",
         (unsigned) size, (unsigned) chunk))
    return 1;

  size_t avail = MIN(10, size - (unsigned) bad_pos);
  diag("Difference starts at %u:", (unsigned) bad_pos);
  unsigned char buf[avail];
  bytelist_get(bl, buf, (unsigned) bad_pos, avail);
  for (unsigned i = 0; i < avail; i++) {
    diag("wanted: %02x, got: %02x", char_for(adjust + bad_pos + i), buf[i]);
  }

  return 0;
}

static int sane_list(const bytelist *bl, size_t adjust, const char *msg) {
  nest_in(msg);
  int t1 = sane_list_c(bl, adjust, 1024);
  nest_out();
  return t1;
}

static void test_join(void) {
  bytelist *bl = NULL;
  sane_list(bl, 0, "bl - empty");

  nest_in("join");

  for (int i = 0; i < 10; i++) {
    size_t bls = bytelist_size(bl);
    bytelist *bl2 = append_about(NULL, 313, bls);
    sane_list(bl2, bls, "bl2");
    bl = bytelist_join(bl, bl2);
    sane_list(bl, 0, "bl - joined");
  }

  nest_out();
  nest_in("split");

  size_t bls = bytelist_size(bl);
  const unsigned nsplit = 100;

  unsigned split[nsplit];
  split[0] = (unsigned) bls - 1;
  split[1] = (unsigned) bls;
  split[2] = 1;
  split[3] = 0;

  for (unsigned i = 4; i < nsplit; i++)
    split[i] = randum((int) bls);

  for (unsigned i = 0; i < nsplit; i++) {
    bytelist *bla, *blb;
    unsigned pos = split[i];

    nest_in("at %u", pos);

    bytelist_split(bl, pos, &bla, &blb);
    size_t blas = bytelist_size(bla);
    size_t blbs = bytelist_size(blb);

    if (!ok(blas == pos, "bla size == %u", pos))
      diag("wanted %u, got %u", pos, (unsigned) blas);
    if (!ok(blbs == bls - pos, "blb size = %u", bls - pos))
      diag("wanted %u, got %u", bls - pos, (unsigned) blbs);

    sane_list(bla, 0, "bla");
    sane_list(blb, pos, "blb");

    bl = bytelist_join(bla, blb);
    sane_list(bl, 0, "rejoined");

    nest_out();
  }

  nest_out();

  bytelist_free(bl);
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
  /*  test_aligned();*/
  /*  test_bytelist();*/
  /*  test_get();*/
  test_join();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
