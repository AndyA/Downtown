/* bytelist-macro.h */

#ifndef BYTELIST_MACRO_H_
#define BYTELIST_MACRO_H_

#define bytelist__SPLICE(a, b)    a ## b
#define bytelist__PASTE(a, b)     bytelist__SPLICE(a, b)


#define bytelist_DECLARE_F(listtype, itemtype)                                                             \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _append      ) (listtype *bl, const itemtype *d, size_t len);      \
  listtype * bytelist__PASTE( listtype, _join        ) (listtype *bl, listtype *nl2);                      \
  void       bytelist__PASTE( listtype, _split       ) (listtype *bl, unsigned pos,                        \
                                                        listtype **bla, listtype **blb);                   \
  listtype * bytelist__PASTE( listtype, _clone       ) (const listtype *bl);                               \
  listtype * bytelist__PASTE( listtype, _defrag      ) (listtype *bl);                                     \
  void       bytelist__PASTE( listtype, _free        ) (listtype *bl);                                     \
  size_t     bytelist__PASTE( listtype, _size        ) (const listtype *bl);                               \
  size_t     bytelist__PASTE( listtype, _member_size ) (const listtype *bl);                               \
  itemtype * bytelist__PASTE( listtype, _get         ) (const listtype *bl, itemtype *out,                 \
                                                        unsigned start, size_t len);                       \
  itemtype * bytelist__PASTE( listtype, _get_all     ) (const listtype *bl, itemtype *out);                \
  itemtype * bytelist__PASTE( listtype, _fetch       ) (const listtype *bl, size_t *sizep);                \
  itemtype * bytelist__PASTE( listtype, _drain       ) (listtype *bl, size_t *sizep);                      \
  listtype * bytelist__PASTE( listtype, _buffer      ) (listtype *bl, itemtype **bufp, size_t *sizep);     \
  listtype * bytelist__PASTE( listtype, _qsort       ) (listtype *bl,                                      \
                                                        int (*compar)(const void *, const void *));        \
  void     * bytelist__PASTE( listtype, _bsearch     ) (const listtype *bl, const itemtype *key,           \
                                                        int (*compar)(const void *, const void *));


#define bytelist_DECLARE(listtype, itemtype)                                                               \
  typedef struct listtype listtype;                                                                        \
  bytelist_DECLARE_F(listtype, itemtype)



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
  void bytelist__PASTE( listtype, _split )  (listtype *bl, unsigned pos,                                   \
                                             listtype **bla, listtype **blb) {                             \
    bytelist_split((bytelist *) bl, pos, (bytelist **) bla, (bytelist **) blb);                            \
  }                                                                                                        \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _clone ) (const listtype *bl) {                                    \
    return (listtype *) bytelist_clone((bytelist *) bl);                                                   \
  }                                                                                                        \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _defrag ) (listtype *bl) {                                         \
    return (listtype *) bytelist_defrag((bytelist *) bl);                                                  \
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
  }                                                                                                        \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _buffer ) (listtype *bl, itemtype **bufp, size_t *sizep) {         \
    return (listtype *) bytelist_buffer((bytelist *) bl, (unsigned char **) bufp, sizep);                  \
  }                                                                                                        \
                                                                                                           \
  listtype * bytelist__PASTE( listtype, _qsort ) (listtype *bl,                                            \
                                                  int (*compar)(const void *, const void *)) {             \
    return (listtype *) bytelist_qsort((bytelist *) bl, compar);                                           \
  }                                                                                                        \
                                                                                                           \
  void * bytelist__PASTE( listtype, _bsearch ) (const listtype *bl, const itemtype *key,                   \
                                                int (*compar)(const void *, const void *)) {               \
    return bytelist_bsearch((const bytelist *) bl, (unsigned char *) key, compar);                         \
  }



#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
