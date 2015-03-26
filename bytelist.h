/* bytelist.h */

#ifndef BYTELIST_H_
#define BYTELIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>

#define bytelist_CHUNK 128
#define bytelist_MAX   (256*1024)

#define bytelist_TO_BL(p, type, field) ((bytelist *)((p) ? (((char *)(p)) + offsetof(type, field)) : NULL))
#define bytelist_BL_TO(p, type, field) ((type *)    ((p) ? (((char *)(p)) - offsetof(type, field)) : NULL))

typedef struct {
  size_t init_size;
  size_t max_size;
  size_t member_size;
  int    terminate;
} bytelist_class;

typedef struct bytelist {
  struct bytelist *next;
  size_t size, used, tail_size;
  unsigned char *data;
  const bytelist_class *clazz;
} bytelist;

bytelist *bytelist_append_internal(bytelist *bl, const unsigned char *bytes, size_t len, const bytelist_class *cl);
bytelist *bytelist_append(bytelist *bl, const unsigned char *bytes, size_t len);

void bytelist_free(bytelist *bl);
size_t bytelist_size(const bytelist *bl);
unsigned char *bytelist_get(const bytelist *bl, unsigned char *out, unsigned start, size_t len);
unsigned char *bytelist_get_all(const bytelist *bl, unsigned char *out);
unsigned char *bytelist_fetch(const bytelist *nl, size_t *sizep);
unsigned char *bytelist_drain(bytelist *nl, size_t *sizep);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
