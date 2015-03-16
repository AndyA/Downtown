/* merge.c */

#include <string.h>

#include "log.h"
#include "merge.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  int frames;
  int phase;
  double *buf;
  y4m2_frame *out_frame;
  const y4m2_parameters *last_parms;
  int warned;
} context;

static context *ctx_new(y4m2_output *next, int frames) {
  context *ctx = alloc(sizeof(context));
  ctx->next = next;
  ctx->frames = frames;
  return ctx;
}

static void ctx_free(context *ctx) {
  if (ctx) {
    free(ctx->buf);
    y4m2_release_frame(ctx->out_frame);
    free(ctx);
  }
}

static void add_frame(context *c, const y4m2_frame *frame) {
  size_t sz = frame->i.size;
  for (unsigned i = 0; i < sz; i++)
    c->buf[i] += frame->buf[i];
}

static void fill_frame(context *c, y4m2_frame *frame) {
  size_t sz = frame->i.size;
  for (unsigned i = 0; i < sz; i++)
    frame->buf[i] = c->buf[i] / c->frames;
  memset(c->buf, 0, sizeof(double) * sz);
}

static void flush_frame(context *c, const y4m2_parameters *parms) {
  fill_frame(c, c->out_frame);
  y4m2_emit_frame(c->next, parms, c->out_frame);
  c->phase = 0;
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
    if (y4m2_has_notes(frame) && !c->warned++)
      log_warning("Frame notes will be lost by merge filter");

    if (!c->buf) {
      c->buf = alloc(sizeof(double) * frame->i.size);
      c->out_frame = y4m2_like_frame(frame);
    }
    c->last_parms = parms;
    add_frame(c, frame);
    if (++c->phase == c->frames) flush_frame(c, parms);
    break;

  case Y4M2_END:
    if (c->phase) flush_frame(c, c->last_parms);
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *merge_filter(y4m2_output *next, int frames) {
  return y4m2_output_next(callback, ctx_new(next, frames));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
