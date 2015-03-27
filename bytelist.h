/* bytelist.h */

#ifndef BYTELIST_H_
#define BYTELIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>

#include "bytelist-macro.h"

#define bytelist_CHUNK 128
#define bytelist_MAX   (256*1024)

enum {
  bytelist_TERMINATED = 1 << 0
};

typedef struct {
  size_t init_size;
  size_t max_size;
  size_t member_size;
  unsigned rise_rate;
  unsigned options;
} bytelist_class;

typedef struct bytelist {
  struct bytelist *next;
  size_t size, used, tail_size, next_size;
  unsigned char *data;
  const bytelist_class *clazz;
} bytelist;

bytelist *bytelist_append_internal(bytelist *bl,
                                   const unsigned char *bytes, size_t len,
                                   const bytelist_class *cl);

bytelist_DECLARE_F(bytelist, unsigned char)

// For tests mainly

bytelist_class *bytelist__get_class(void);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
