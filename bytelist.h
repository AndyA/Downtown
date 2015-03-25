/* bytelist.h */

#ifndef BYTELIST_H_
#define BYTELIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define bytelist_CHUNK 128
#define bytelist_MAX   (256*1024)

typedef struct bytelist {
  struct bytelist *next;
  size_t size, used, tail_size;
  unsigned char *data;
} bytelist;

bytelist *bytelist_append_internal(bytelist *cl, const unsigned char *bytes, size_t len, size_t min_chunk);
bytelist *bytelist_append(bytelist *cl, const unsigned char *bytes, size_t len);

void bytelist_free(bytelist *cl);
size_t bytelist_size(const bytelist *cl);
unsigned char *bytelist_get(const bytelist *cl, unsigned char *out, unsigned start, size_t len);
unsigned char *bytelist_get_all(const bytelist *cl, unsigned char *out);
unsigned char *bytelist_fetch(const bytelist *nl, size_t *sizep);
unsigned char *bytelist_drain(bytelist *nl, size_t *sizep);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
