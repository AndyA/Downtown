/* ptrlist.h */

#ifndef PTRLIST_H_
#define PTRLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "bytelist.h"

#define ptrlist_CHUNK  (sizeof(void *) * 1024)
#define ptrlist_MAX    (1024*1024)

bytelist_DECLARE(ptrlist, void *)

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
