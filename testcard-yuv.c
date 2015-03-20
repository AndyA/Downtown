/* testcard-yuv.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "log.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define WIDTH  1920
#define HEIGHT 1080
#define FRAMES 256

#define PROG   "testcard-yuv"

#define SCALE(x, ilo, ihi, olo, ohi) \
  (((x) - (ilo)) * ((ohi) - (olo)) / ((ihi) - (ilo)) + (olo))

static void fill_frame(y4m2_frame *frame, int *permute, int cr) {
  y4m2_tell_me_about_stride(frame);
  for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
    int w = frame->i.width / frame->i.plane[pl].xs;
    int h = frame->i.height / frame->i.plane[pl].ys;
    uint8_t *base = frame->plane[pl];

    int ppl = permute[pl];

    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        *base++ = (ppl == Y4M2_Y_PLANE) ? SCALE(x, 0, w - 1, 0, 255) :
                  (ppl == Y4M2_Cb_PLANE) ? SCALE(y, 0, h - 1, 0, 255) : cr;
      }
    }
  }
}

int main(void) {
  log_info("Starting %s", PROG);

  y4m2_output *out = y4m2_output_file(stdout);
  y4m2_parameters *parms = y4m2_adjust_parms(NULL, "C444 W%u H%u A1:1 F25:1 It",
                           WIDTH, HEIGHT);
  y4m2_frame *frame = y4m2_new_frame(parms);

  y4m2_emit_start(out, parms);

  for (int c1 = 0; c1 < Y4M2_N_PLANE; c1++) {
    for (int c2 = 0; c2 < Y4M2_N_PLANE; c2++) {
      if (c2 == c1) continue;
      for (int c3 = 0; c3 < Y4M2_N_PLANE; c3++) {
        if (c3 == c1 || c3 == c2) continue;
        int permute[] = { c1, c2, c3 };
        log_debug("permute: %d, %d, %d", c1, c2, c3);
        for (int fn = 0; fn < FRAMES; fn++) {
          fill_frame(frame, permute, SCALE(fn, 0, FRAMES - 1, 0, 255));
          y4m2_emit_frame(out, parms, frame);
        }
      }
    }
  }

  y4m2_emit_end(out);

  y4m2_free_output(out);
  y4m2_release_frame(frame);
  y4m2_free_parms(parms);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
