/* downtown.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yuv4mpeg2.h"
#include "zigzag.h"

typedef struct {
  y4m2_frame *buf;
  y4m2_output *next;
} context;

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  (void) parms;
  (void) frame;
  context *c = ctx;

  switch (reason) {
  case Y4M2_START:
    y4m2_emit_start(c->next, parms);
    break;
  case Y4M2_FRAME:
    if (!c->buf) c->buf = y4m2_like_frame(frame);
    for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
      int w = frame->i.width / frame->i.plane[pl].xs;
      int h = frame->i.height / frame->i.plane[pl].ys;
      zigzag_permute(frame->plane[pl], c->buf->plane[pl], w, h);
    }
    y4m2_emit_frame(c->next, parms, c->buf);
    break;
  case Y4M2_END:
    y4m2_release_frame(c->buf);
    y4m2_emit_end(c->next);
    break;
  }
}

int main(void) {
  context ctx;

  ctx.next = y4m2_output_file(stdout);

  y4m2_output *out = y4m2_output_next(callback, &ctx);
  y4m2_parse(stdin, out);
  y4m2_free_output(ctx.next);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
