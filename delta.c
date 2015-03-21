/* delta.c */

#include <string.h>

#include "delta.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  y4m2_frame *prev;
  y4m2_frame *out;
} context;

static context *ctx_new(y4m2_output *next) {
  context *ctx = alloc(sizeof(context));
  ctx->next = next;
  return ctx;
}

static void set_prev(context *c, y4m2_frame *frame) {
  y4m2_release_frame(c->prev);
  c->prev = frame;
}

static void ctx_free(context *ctx) {
  if (ctx) {
    y4m2_release_frame(ctx->prev);
    y4m2_release_frame(ctx->out);
    free(ctx);
  }
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    y4m2_emit_start(c->next, parms);
    break;

  case Y4M2_FRAME:
    y4m2_tell_me_about_stride(frame);
    if (!c->prev) set_prev(c, frame);
    if (!c->out) c->out = y4m2_like_frame(frame);

    const uint8_t *pp = c->prev->buf;
    const uint8_t *np = frame->buf;
    uint8_t *op = c->out->buf;

    for (unsigned i = 0; i < frame->i.size; i++) {
      int delta = *np++ - *pp++ + 128;
      *op++ = MIN(MAX(0, delta), 255);
    }

    y4m2_copy_notes(c->out, frame);
    y4m2_emit_frame(c->next, parms, y4m2_retain_frame(c->out));
    set_prev(c, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *delta_filter(y4m2_output *next) {
  return y4m2_output_next(callback, ctx_new(next));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
