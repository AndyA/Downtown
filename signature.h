/* signature.h */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stdlib.h>

#include "profile.h"

#define signature_WBITS       (sizeof(unsigned) * CHAR_BIT)
#define signature_WCOUNT      (profile_SIGNATURE_BITS / signature_WBITS)
#define signature_LEN_BIN     profile_SIGNATURE_BITS
#define signature_LEN_HEX     (signature_LEN_BIN / 4)

typedef struct {
  unsigned w[signature_WCOUNT];
} signature;

unsigned signature__count_bits(unsigned v);
unsigned signature_count(const signature *sig);
signature *signature_xor(signature *out, const signature *a, const signature *b);
unsigned signature_distance(const signature *a, const signature *b);

signature *signature_parse(signature *out, const char *str);
char *signature_format_bin(const signature *sig, char *buf, size_t len);
char *signature_format_hex(const signature *sig, char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
