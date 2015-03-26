#include "ptrlist.h"

#define ptrlist_CHUNK  (sizeof(void *) * 1024)
#define ptrlist_MAX    (1024*1024)

bytelist_DEFINE(ptrlist, void *, ptrlist_CHUNK, ptrlist_MAX, 0)

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
