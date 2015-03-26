/* numlist.h */

#ifndef NUMLIST_H_
#define NUMLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bytelist.h"

bytelist_DECLARE(numlist, double)

numlist *numlist_putn(numlist *nl, double d);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
