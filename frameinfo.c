/* frameinfo.c */

#include <math.h>
#include <string.h>

#include "frameinfo.h"
#include "log.h"
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

static const char *pl_name[] = { "Y", "Cb", "Cr" };

static void info_for_plane(y4m2_frame *frame, int pl) {
  uint8_t *base = frame->plane[pl];
  size_t size = frame->i.plane[pl].size;
  unsigned i;

  frameinfo *info = alloc(sizeof(frameinfo));
  char name[30];
  sprintf(name, "frameinfo.%s", pl_name[pl]);

  double total = 0;
  for (i = 0; i < (unsigned) size; i++) total += base[i];
  info->average = total / (double) size;

  double var = 0;
  for (i = 0; i < (unsigned) size; i++) {
    double delta = info->average - base[i];
    var += delta * delta;
  }
  info->variance = sqrt(var);

  y4m2_set_note(frame, name, info, free);

  /*  log_debug("Info %s: average=%f, variance=%f", name, info->average, info->variance);*/
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
    for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++)
      info_for_plane(frame, pl);
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *filter_frameinfo(y4m2_output *next) {
  return y4m2_output_next(callback, ctx_new(next));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
