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
  unsigned rise_rate;
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



#define bytelist_DECLARE_F(listtype, itemtype)                                                             \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _append      ) (listtype *bl, const itemtype *d, size_t len);      \
  listtype * bytelist__PASTE( listtype, _join        ) (listtype *bl, listtype *nl2);                      \
  void       bytelist__PASTE( listtype, _split       ) (listtype *bl, unsigned pos,                        \
                                                        listtype **bla, listtype **blb);                   \
  void       bytelist__PASTE( listtype, _free        ) (listtype *bl);                                     \
  size_t     bytelist__PASTE( listtype, _size        ) (const listtype *bl);                               \
  size_t     bytelist__PASTE( listtype, _member_size ) (const listtype *bl);                               \
  itemtype * bytelist__PASTE( listtype, _get         ) (const listtype *bl, itemtype *out,                 \
                                                        unsigned start, size_t len);                       \
  itemtype * bytelist__PASTE( listtype, _get_all     ) (const listtype *bl, itemtype *out);                \
  itemtype * bytelist__PASTE( listtype, _fetch       ) (const listtype *bl, size_t *sizep);                \
  itemtype * bytelist__PASTE( listtype, _drain       ) (listtype *bl, size_t *sizep);


#define bytelist_DECLARE(listtype, itemtype)                                                               \
  typedef struct listtype listtype;                                                                        \
  bytelist_DECLARE_F(listtype, itemtype)


bytelist *bytelist_append_internal(bytelist *bl, 
                                   const unsigned char *bytes, size_t len,
                                   const bytelist_class *cl);

bytelist_DECLARE_F(bytelist, unsigned char)


#define bytelist_DEFINE(listtype, itemtype, chunk, max, opt)                                               \
                                                                                                           \
  static bytelist_class bytelist__PASTE( listtype, _me ) = {                                               \
    .init_size   = chunk,                                                                                  \
    .max_size    = max,                                                                                    \
    .rise_rate   = 2,                                                                                      \
    .member_size = sizeof(itemtype),                                                                       \
    .options     = opt                                                                                     \
  };                                                                                                       \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _append )  (listtype *bl, const itemtype *d, size_t len) {         \
    return (listtype *) bytelist_append_internal((bytelist *) bl, (unsigned char *) d, len,                \
        &bytelist__PASTE( listtype, _me ));                                                                \
  }                                                                                                        \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _join )  (listtype *bl, listtype *bl2) {                           \
    return (listtype *) bytelist_join((bytelist *) bl, (bytelist *) bl2);                                  \
  }                                                                                                        \
                                                                                                           \
  void bytelist__PASTE( listtype, _free ) (listtype *bl) {                                                 \
    bytelist_free((bytelist *) bl);                                                                        \
  }                                                                                                        \
                                                                                                           \
  size_t  bytelist__PASTE( listtype, _size ) (const listtype *bl) {                                        \
    return bytelist_size((const bytelist *) bl);                                                           \
  }                                                                                                        \
                                                                                                           \
  size_t  bytelist__PASTE( listtype, _member_size ) (const listtype *bl) {                                 \
    return bytelist_member_size((const bytelist *) bl);                                                    \
  }                                                                                                        \
                                                                                                           \
  itemtype * bytelist__PASTE( listtype, _get ) (const listtype *bl, itemtype *out,                         \
                                                unsigned start, size_t len) {                              \
    return (itemtype *) bytelist_get((const bytelist *) bl, (unsigned char *) out, start, len);            \
  }                                                                                                        \
                                                                                                           \
  itemtype * bytelist__PASTE( listtype, _get_all ) (const listtype *bl, itemtype *out) {                   \
    return bytelist__PASTE( listtype, _get ) (bl, out, 0,                                                  \
                                    bytelist__PASTE( listtype, _size ) (bl));                              \
  }                                                                                                        \
                                                                                                           \
  itemtype * bytelist__PASTE( listtype, _fetch ) (const listtype *bl, size_t *sizep) {                     \
    return (itemtype *) bytelist_fetch((const bytelist *) bl, sizep);                                      \
  }                                                                                                        \
                                                                                                           \
  itemtype * bytelist__PASTE( listtype, _drain ) (listtype *bl, size_t *sizep) {                           \
    return (itemtype *) bytelist_drain((bytelist *) bl, sizep);                                            \
  }

// For tests mainly

bytelist_class *bytelist__get_class(void);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
