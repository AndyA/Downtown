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

enum { 
  bytelist_TERMINATED = 1 << 0 
};

typedef struct {
  size_t init_size;
  size_t max_size;
  size_t member_size;
  unsigned options;
} bytelist_class;

typedef struct bytelist {
  struct bytelist *next;
  size_t size, used, tail_size;
  unsigned char *data;
  const bytelist_class *clazz;
} bytelist;

#define bytelist__SPLICE(a, b)    a ## b
#define bytelist__PASTE(a, b)     bytelist__SPLICE(a, b)



#define bytelist_DECLARE_F(listtype, itemtype)                                                         \
                                                                                                       \
  listtype * bytelist__PASTE( listtype, _append  ) (listtype *nl, const itemtype *d, size_t len);      \
  void       bytelist__PASTE( listtype, _free    ) (listtype *nl);                                     \
  size_t     bytelist__PASTE( listtype, _size    ) (const listtype *nl);                               \
  itemtype * bytelist__PASTE( listtype, _get     ) (const listtype *nl, itemtype *out,                 \
                                                    unsigned start, size_t len);                       \
  itemtype * bytelist__PASTE( listtype, _get_all ) (const listtype *nl, itemtype *out);                \
  itemtype * bytelist__PASTE( listtype, _fetch   ) (const listtype *nl, size_t *sizep);                \
  itemtype * bytelist__PASTE( listtype, _drain   ) (listtype *nl, size_t *sizep);


#define bytelist_DECLARE(listtype, itemtype)                                                           \
  typedef struct listtype listtype;                                                                    \
  bytelist_DECLARE_F(listtype, itemtype)


bytelist *bytelist_append_internal(bytelist *bl, 
                                   const unsigned char *bytes, size_t len,
                                   const bytelist_class *cl);

bytelist_DECLARE_F(bytelist, unsigned char)


#define bytelist_DEFINE(listtype, itemtype, chunk, max, opt)                                           \
                                                                                                       \
  static bytelist_class bytelist__PASTE( listtype, _me ) = {                                           \
    .init_size   = chunk,                                                                              \
    .max_size    = max,                                                                                \
    .member_size = sizeof(itemtype),                                                                   \
    .options     = opt                                                                                 \
  };                                                                                                   \
                                                                                                       \
  listtype * bytelist__PASTE( listtype, _append )  (listtype *nl, const itemtype *d, size_t len) {     \
    return (listtype *) bytelist_append_internal((bytelist *) nl, (unsigned char *) d, len,            \
        &bytelist__PASTE( listtype, _me ));                                                            \
  }                                                                                                    \
                                                                                                       \
  void bytelist__PASTE( listtype, _free ) (listtype *nl) {                                             \
    bytelist_free((bytelist *) nl);                                                                    \
  }                                                                                                    \
                                                                                                       \
  size_t  bytelist__PASTE( listtype, _size ) (const listtype *nl) {                                    \
    return bytelist_size((const bytelist *) nl);                                                       \
  }                                                                                                    \
                                                                                                       \
  itemtype * bytelist__PASTE( listtype, _get ) (const listtype *nl, itemtype *out,                     \
                                                unsigned start, size_t len) {                          \
    return (itemtype *) bytelist_get((const bytelist *) nl, (unsigned char *) out, start, len);        \
  }                                                                                                    \
                                                                                                       \
  itemtype * bytelist__PASTE( listtype, _get_all ) (const listtype *nl, itemtype *out) {               \
    return bytelist__PASTE( listtype, _get ) (nl, out, 0,                                              \
                                    bytelist__PASTE( listtype, _size ) (nl));                          \
  }                                                                                                    \
                                                                                                       \
  itemtype * bytelist__PASTE( listtype, _fetch ) (const listtype *nl, size_t *sizep) {                 \
    return (itemtype *) bytelist_fetch((const bytelist *) nl, sizep);                                  \
  }                                                                                                    \
                                                                                                       \
  itemtype * bytelist__PASTE( listtype, _drain ) (listtype *nl, size_t *sizep) {                       \
    return (itemtype *) bytelist_drain((bytelist *) nl, sizep);                                        \
  }

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
