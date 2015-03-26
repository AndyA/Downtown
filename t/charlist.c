/* t/charlist.c */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "charlist.h"
#include "tap.h"
#include "util.h"

#define CHUNK 128

static charlist *stuff_list(charlist *cl, size_t len, int base) {
  unsigned char str[len];
  for (unsigned i = 0; i < len; i++)
    str[i] = (unsigned char) base + i;
  return charlist_append(cl, (const char *) str, len);
}

static int check_sequence(const char *str, size_t len, int base) {
  for (unsigned i = 0; i < len; i++)
    if ((unsigned char) str[i] != (unsigned char)(base + i)) return (int) i;
  return -1;
}

static charlist *check_list(charlist *cl, size_t expect, int n) {
  cl = stuff_list(cl, n, expect + 1);
  expect += n;
  nest_in("added %d (total %llu)", n, (unsigned long long) expect);

  ok(!!cl, "cl != NULL");
  ok(charlist_size(cl) == expect, "size is %llu", (unsigned long long) expect);

  char *d = charlist_fetch(cl, NULL);
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

static void test_charlist(void) {
  nest_in("non-aligned");
  charlist *cl = NULL;

  ok(charlist_size(cl) == 0, "NULL: size == 0");

  size_t expect = 0;

  for (int n = 1; n < CHUNK * 3; n += 313) {
    cl = check_list(cl, expect, n);
    expect += n;
  }

  charlist_free(cl);
  nest_out();
}

static charlist *check_al(charlist *cl, size_t *expect, int delta, int count) {
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
  charlist *cl = NULL;

  ok(charlist_size(cl) == 0, "NULL: size == 0");

  size_t expect = 0;
  cl = check_al(cl, &expect, -1, 5);
  cl = check_al(cl, &expect, 1, 10);
  cl = check_al(cl, &expect, -1, 10);
  cl = check_al(cl, &expect, 1, 5);

  charlist_free(cl);
  nest_out();
}

static charlist *put1(charlist *cl, char c) {
  return charlist_append(cl, &c, 1);
}

static void test_get(void) {
  charlist *cl = NULL;

  const int count = 1000;

  for (int i = 0; i < count; i++)
    cl = put1(cl, i + 1);

  size_t len = 1;
  for (int pos = 0; pos < count;) {
    size_t want = MIN(len, (unsigned) count - pos);
    char out[want];

    char *got = charlist_get(cl, out, pos, want);

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

static void test_long_string(void) {
  char buf[313];
  charlist *cl = NULL;
  for (int i = 0; i < (int) sizeof(buf); i++)
    buf[i] = (rand() & 0x3f) + 0x30;

  size_t len = 0;
  unsigned span = sizeof(buf);
  while (--span) {
    cl = charlist_append(cl, buf, span);
    len += span;
  }

  nest_in("long string");

  ok(charlist_size(cl) == len, "charlist_size=%u", (unsigned) len);

  /*  ok(cl->size > charlist_CHUNK, "buffer chunk (%u)", (unsigned) cl->size);*/

  char *str = charlist_drain(cl, NULL);
  ok(strlen(str) == len, "strlen=%u", (unsigned) len);

  span = sizeof(buf);
  char *sp = str;
  while (--span) {
    ok(!memcmp(buf, sp, span), "chunk at %u for %u matches", sp - str, span);
    sp += span;
    len += span;
  }

  free(str);

  nest_out();
}

static void test_string(void) {
  charlist *cl = NULL;
  cl = charlist_puts(cl, "Hello, World");
  cl = charlist_printf(cl, ", how are you %s?", "today");
  cl = charlist_puts(cl, " I'm full of string.");
  cl = charlist_printf(cl, " It couldn't happen to a %s.", "nicer person");
  cl = charlist_printf(cl, " Bye!");

  char *s1 = charlist_fetch(cl, NULL);
  char *s2 = charlist_drain(cl, NULL);

  const char *want = "Hello, World, how are you today? I'm full of string. "
                     "It couldn't happen to a nicer person. Bye!";

  if (!ok(!strcmp(s1, want), "got expected string"))
    diag("wanted \"%s\", got \"%s\"", want, s1);
  ok(!strcmp(s1, s2), "got it again");
  ok(s1 != s2, "and it's different");

  free(s1);
  free(s2);
}

void test_main(void) {
  test_aligned();
  test_charlist();
  test_get();
  test_long_string();
  test_string();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
