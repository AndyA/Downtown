/* charlist.h */

#ifndef CHARLIST_H_
#define CHARLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define charlist_CHUNK 128
#define charlist_MAX   (256*1024)

typedef struct charlist {
  struct charlist *next;
  size_t size, used, tail_size;
  char *data;
} charlist;

charlist *charlist_append_bytes(charlist *cl, const char *str, size_t len);
charlist *charlist_append(charlist *cl, const char *str);
charlist *charlist_vprintf(charlist *cl, const char *fmt, va_list ap);
charlist *charlist_printf(charlist *cl, const char *fmt, ...);

void charlist_free(charlist *cl);
size_t charlist_size(const charlist *cl);
char *charlist_get(const charlist *cl, char *out, unsigned start, size_t len);
char *charlist_get_all(const charlist *cl, char *out);
char *charlist_fetch(const charlist *cl);
char *charlist_drain(charlist *cl);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
