/* charlist.h */

#ifndef CHARLIST_H_
#define CHARLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include "bytelist.h"

bytelist_DECLARE(charlist, char)

charlist *charlist_puts(charlist *cl, const char *str);
charlist *charlist_vprintf(charlist *cl, const char *fmt, va_list ap);
charlist *charlist_printf(charlist *cl, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
