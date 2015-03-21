/* splitter.c */

#include <string.h>

#include "log.h"
#include "splitter.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output **nexts;
  size_t n_next;
} context;

static context *ctx_new(y4m2_output **nexts, size_t n_next) {
  context *c = alloc(sizeof(context));
  c->n_next = n_next;
  c->nexts = alloc(sizeof(y4m2_output) * n_next);
  memcpy(c->nexts, nexts, sizeof(y4m2_output) * n_next);
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
  unsigned i;

  switch (reason) {

  case Y4M2_START:
    for (i = 0; i < c->n_next; i++)
      y4m2_emit_start(c->nexts[i], parms);
    break;

  case Y4M2_FRAME:
    for (i = 0; i < c->n_next; i++)
      y4m2_emit_frame(c->nexts[i], parms, frame);
    break;

  case Y4M2_END:
    for (i = 0; i < c->n_next; i++)
      y4m2_emit_end(c->nexts[i]);
    ctx_free(c);
    break;

  }
}

y4m2_output *splitter_filter(y4m2_output **nexts, size_t n_next) {
  return y4m2_output_next(callback, ctx_new(nexts, n_next));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
