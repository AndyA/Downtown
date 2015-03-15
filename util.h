/* util.h */

#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void die(const char *msg, ...);
void *alloc(size_t size);
char *sstrdup(const char *s);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
