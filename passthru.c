/* passthru.c */

#include <string.h>

#include "log.h"
#include "passthru.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
} context;

static context *ctx_new(y4m2_output *next) {
  context *ctx = alloc(sizeof(context));
  ctx->next = next;
  return ctx;
}

static void ctx_free(context *ctx) {
  if (ctx) {
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
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *passthru_filter(y4m2_output *next) {
  return y4m2_output_next(callback, ctx_new(next));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
