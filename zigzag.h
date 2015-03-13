/* zigzag.h */

#ifndef ZIGZAG_H_
#define ZIGZAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void zigzag_permute(const uint8_t *in, uint8_t *out, int w, int h);
void zigzag_raster(const uint8_t *in, uint8_t *out, int w, int h);
void zigzag_weave(const uint8_t *in, uint8_t *out, int w, int h);

size_t zigzag_unity(int w, int h);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
