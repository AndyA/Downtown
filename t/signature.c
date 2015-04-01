/* t/signature.c */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "signature.h"
#include "tap.h"
#include "util.h"

#define NOWT 0.0001
#define NREP 10
#define MAX_DIFFS 8

static char *diff_str(char *out, const char *a, const char *b) {
  size_t alen = strlen(a);
  size_t blen = strlen(b);
  char *op = out;
  unsigned i;

  for (i = 0; i < alen && i < blen; i++)
    *op++ = a[i] == b[i] ? ' ' : '^';

  for (; i < alen || i < blen; i++)
    *op++ = '^';

  *op++ = '\0';

  return out;
}

static int ndiff(const char *a, const char *b) {
  int diffs = 0;
  while (*a && *b) if (*a++ != *b++) diffs++;
  while (*a++) diffs++;
  while (*b++) diffs++;
  return diffs;
}

static void test_signature(void) {

  nest_in("signature");

  double data[700];
  char sig[sig_SIGNATURE_BITS + 1];

  const char *want = "01111110000000111111100000001111"
                     "11100110001100110001100111111100"
                     "00000111111100000001111111000000"
                     "01111111000000011111110010001110"
                     "11100010001110111000100111111100"
                     "00000111111110000001111111100000"
                     "01111111000100011101110001000111"
                     "01110011001111111000000011111110";


  for (unsigned i = 0; i < sizeof(data) / sizeof(data[0]); i++)
    data[i] = 15
              + sin((double) i / 2) * 3
              + sin((double) i / 6) * 3.2
              + sin((double) i / 7.1) * 1.9;

  char *got = sig_signature(sig, data, sizeof(data) / sizeof(data[0]));
  ok(got == sig, "returned pointer to buf");
  int diffs = ndiff(want, got);
  if (!ok(diffs < MAX_DIFFS, "signature matches with %d bits", MAX_DIFFS)) {
    char diff[sig_SIGNATURE_BITS + 1];

    diag("wanted: %s", want);
    diag("got:    %s", got);
    diag("        %s", diff_str(diff, want, got));
  }

  nest_out();
}

void test_main(void) {
  test_signature();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
