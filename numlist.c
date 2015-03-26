
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "numlist.h"
#include "util.h"

bytelist_DEFINE(numlist, double, numlist_CHUNK, numlist_MAX, 0)

numlist *numlist_putn(numlist *nl, double d) {
  return numlist_append(nl, &d, 1);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
