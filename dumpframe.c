/* dumpframe.c */

#include <string.h>

#include "log.h"
#include "dumpframe.h"
#include "util.h"
#include "y4m2png.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  char *name_pattern;
  unsigned every;
  unsigned count;
} context;

static context *ctx_new(y4m2_output *next, const char *name_pattern, unsigned every) {
  context *ctx = alloc(sizeof(context));
  ctx->next = next;
  ctx->name_pattern = sstrdup(name_pattern);
  ctx->every = every;
  return ctx;
}

static void ctx_free(context *ctx) {
  if (ctx) {
    free(ctx->name_pattern);
    free(ctx);
  }
}

static void _dump_frame(context *c, const y4m2_frame *frame) {
  if (c->count % c->every == 0) {
    char *name = ssprintf(c->name_pattern, c->count);
    y4m2_png_write_file(frame, name);
    free(name);
  }
  c->count++;
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
    _dump_frame(c, frame);
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *dumpframe_filter(y4m2_output *next, const char *name_pattern, unsigned every) {
  return y4m2_output_next(callback, ctx_new(next, name_pattern, every));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
