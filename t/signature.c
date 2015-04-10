/* t/signature.c */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "signature.h"
#include "tap.h"
#include "util.h"

static void test_count(void) {
  nest_in("signature__count_bits");

  ok(0 == signature__count_bits(0), "0 has 0 set bits");
  ok(signature_WBITS == signature__count_bits(UINT_MAX), "%u has %u set bits",
     UINT_MAX, signature_WBITS);

  for (unsigned b = 0; b < signature_WBITS; b++)
    ok(1 == signature__count_bits(1u << b), "%u has 1 set bit", 1u << b);

  nest_out();
}

static unsigned randbit(void) {
  return rand() & (profile_SIGNATURE_BITS - 1);
}

static unsigned count_ones(const char *buf) {
  unsigned count = 0;
  for (unsigned i = 0; buf[i]; i++)
    if (buf[i] == '1') count++;
  return count;
}

static void test_format(void) {
  nest_in("formatting");

  char bin_buf[signature_LEN_BIN + 1];
  char bin_got[signature_LEN_BIN + 1];
  char hex_buf[signature_LEN_HEX + 1];

  signature sig, sig2;

  for (unsigned i = 1; i < profile_SIGNATURE_BITS; i++) {
    memset(bin_buf, '0', signature_LEN_BIN);
    bin_buf[signature_LEN_BIN] = '\0';
    for (unsigned b = 0; b < i; b++) bin_buf[randbit()] = '1';
    unsigned want = count_ones(bin_buf);

    signature_parse(&sig, bin_buf);
    unsigned got = signature_count(&sig);
    ok(got == want, "%u bits in %s", want, bin_buf);

    signature_format_hex(&sig, hex_buf, sizeof(hex_buf));
    signature_parse(&sig2, hex_buf);

    got = signature_count(&sig2);
    ok(got == want, "%u bits in %s", want, hex_buf);

    signature_format_bin(&sig2, bin_got, sizeof(bin_got));
    ok(!strcmp(bin_buf, bin_got), "%s -> %s", hex_buf, bin_buf);
  }

  nest_out();
}

void test_main(void) {
  test_count();
  test_format();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
