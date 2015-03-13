/* downtown.h */

#ifndef DOWNTOWN_H_
#define DOWNTOWN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void (*permute_func)(const uint8_t *in, uint8_t *out, int w, int h);
typedef size_t (*permute_size_func)(int w, int h);

typedef struct {
  const char *name;
  permute_func f;
  permute_size_func sf;
} permute_method;

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
