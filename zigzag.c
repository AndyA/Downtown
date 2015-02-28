/* zigzag.c */

#include <stdio.h>
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

    if (x & 1) {
      while (x0 >= x1) {
        *op++ = in[ y0 * w + x0 ];
        x0--;
        y0++;
      }
    }
    else {
      while (x1 <= x0) {
        *op++ = in[ y1 * w + x1];
        x1++;
        y1--;
      }
    }
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
