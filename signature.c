/* signature.c */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "profile.h"
#include "signature.h"
#include "util.h"

/*
 * https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 */

unsigned signature__count_bits(unsigned v) {
  v = v - ((v >> 1) & (unsigned)~(unsigned)0 / 3);
  v = (v & (unsigned)~(unsigned)0 / 15 * 3) + ((v >> 2) & (unsigned)~(unsigned)0 / 15 * 3);
  v = (v + (v >> 4)) & (unsigned)~(unsigned)0 / 255 * 15;
  return (unsigned)(v * ((unsigned)~(unsigned)0 / 255)) >> (sizeof(unsigned) - 1) * CHAR_BIT;
}

unsigned signature_count(const signature *sig) {
  unsigned bits = 0;
  for (unsigned i = 0; i < signature_WCOUNT; i++)
    bits += signature__count_bits(sig->w[i]);
  return bits;
}

signature *signature_xor(signature *out, const signature *a, const signature *b) {
  for (unsigned i = 0; i < signature_WCOUNT; i++)
    out->w[i] = a->w[i] ^ b->w[i];
  return out;
}

unsigned signature_distance(const signature *a, const signature *b) {
  signature tmp;
  signature_xor(&tmp, a, b);
  return signature_count(&tmp);
}

static signature *parse(signature *out, const char *str, unsigned base) {
  unsigned dmax = 1u << base;
  memset(out, 0, sizeof(signature));
  for (unsigned i = signature_WCOUNT; i--;) {
    for (unsigned shift = signature_WBITS; shift;) {
      shift -= base;
      char digit = *str++;
      unsigned dv = (digit >= '0' && digit <= '9') ? (digit - '0')
                    : (digit >= 'A' && digit <= 'F') ? (digit - 'A' + 10)
                    : (digit >= 'a' && digit <= 'f') ? (digit - 'a' + 10) : dmax;

      if (dv >= dmax) die("Bad digit in signature");
      out->w[i] |= (dv << shift);
    }
  }
  return out;
}

signature *signature_parse(signature *out, const char *str) {
  size_t sl = strlen(str);
  switch (sl) {
  case signature_LEN_BIN:
    return parse(out, str, 1);
  case signature_LEN_HEX:
    return parse(out, str, 4);
  default:
    die("Invalid signature");
    return NULL;
  }
}

static void format(const signature *sig, char *buf, size_t len, unsigned base) {
  if (len <= (profile_SIGNATURE_BITS >> base))
    die("Buffer too small");

  unsigned mask = (1u << base) - 1 ;
  for (unsigned i = signature_WCOUNT; i--;) {
    for (unsigned shift = signature_WBITS; shift;) {
      shift -= base;
      unsigned bits = (sig->w[i] >> shift) & mask;
      *buf++ = bits < 10 ? ('0' + bits) : ('a' + bits - 10);
    }
  }
  *buf++ = '\0';
}

char *signature_format_bin(const signature *sig, char *buf, size_t len) {
  format(sig, buf, len, 1);
  return buf;
}

char *signature_format_hex(const signature *sig, char *buf, size_t len) {
  format(sig, buf, len, 4);
  return buf;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
