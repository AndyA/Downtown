/* splitter.c */

#include <stdarg.h>
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

y4m2_output *splitter_filter_ar(y4m2_output **nexts, size_t n_next) {
  return y4m2_output_next(callback, ctx_new(nexts, n_next));
}

static unsigned count_args(y4m2_output *next, va_list ap) {
  va_list ap2;

  va_copy(ap2, ap);
  unsigned count = 0;
  while (next) {
    count++;
    next = va_arg(ap2, y4m2_output *);
  }
  va_end(ap2);

  return count;
}

y4m2_output *splitter_filter(y4m2_output *next, ...) {
  va_list ap;

  va_start(ap, next);
  unsigned count = count_args(next, ap);
  y4m2_output **nexts = alloc(sizeof(y4m2_output) * count);
  y4m2_output **np = nexts;

  while (next) {
    *np++ = next;
    next = va_arg(ap, y4m2_output *);
  }

  va_end(ap);

  y4m2_output *rv = splitter_filter_ar(nexts, count);
  free(nexts);

  return rv;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
