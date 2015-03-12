/* zigzag.c */

#include <stdio.h>
#include <string.h>

#include "downtown.h"
#include "zigzag.h"

void zigzag_permute(const uint8_t *in, uint8_t *out, int w, int h) {
  unsigned limit = w + h - 1;
  uint8_t *op = out;

  for (unsigned x = 0; x < limit; x++) {
    int x0 = x;
    if (x0 >= w) x0 = w - 1;
    int y0 = x - x0;
    int y1 = x;
    if (y1 >= h) y1 = h - 1;
    int x1 = x - y1;

    const uint8_t *inp = (x & 1) ? (in + y0 * w + x0) : (in + y1 * w + x1);
    int stride = (x & 1) ? (w - 1) : (1 - w);
    unsigned count = x0 - x1 + 1;

    for (unsigned i = 0; i < count; i++) {
      *op++ = *inp;
      inp += stride;
    }
  }
}

void zigzag_raster(const uint8_t *in, uint8_t *out, int w, int h) {
  memcpy(out, in, w * h);
}

static void _reverse(uint8_t *buf, size_t size) {
  uint8_t *left = buf;
  uint8_t *right = buf + size - 1;
  while (left < right) {
    uint8_t t = *left;
    *left++ = *right;
    *right-- = t;
  }
}

void zigzag_weave(const uint8_t *in, uint8_t *out, int w, int h) {
  zigzag_raster(in, out, w, h);
  for (int y = 0; y < h; y += 2)
    _reverse(out + y * w, w);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
