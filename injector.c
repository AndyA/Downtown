/* injector.c */

#include <stdint.h>
#include <string.h>

#include "log.h"
#include "injector.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  uint64_t sequence;
  injector_callback cb;
  void *ctx;
} context;

static context *ctx_new(y4m2_output *next, injector_callback cb, void *ctx) {
  context *c = alloc(sizeof(context));
  c->next = next;
  c->cb = cb;
  c->ctx = ctx;
  return c;
}

static void ctx_free(context *c) {
  if (c) {
    free(c);
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
    /* slight hack - reconstitute sequence */
    frame->sequence = c->sequence++;
    frame = c->cb(frame, c->ctx);
    if (frame) y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *injector_filter(y4m2_output *next, injector_callback cb, void *ctx) {
  return y4m2_output_next(callback, ctx_new(next, cb, ctx));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
