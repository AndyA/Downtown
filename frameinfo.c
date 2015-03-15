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

static double variance_around(const uint8_t *base, size_t size, double zero) {
  double var = 0;
  for (unsigned i = 0; i < (unsigned) size; i++) {
    double delta = zero - base[i];
    var += delta * delta;
  }
  return sqrt(var);
}

static void info_for_plane(y4m2_frame *frame, int pl) {
  uint8_t *base = frame->plane[pl];
  size_t size = frame->i.plane[pl].size;
  unsigned i;

  frameinfo *info = alloc(sizeof(frameinfo));
  char name[30];
  sprintf(name, "frameinfo.%s", pl_name[pl]);

  double total = 0;
  double min = 0;
  double max = 0;

  double scale = sqrt(255 * 255 * size);

  for (i = 0; i < (unsigned) size; i++) {
    total += base[i];
    if (i == 0 || base[i] < min) min = base[i];
    if (i == 0 || base[i] > max) max = base[i];
  }

  info->average = total / (double) size;
  info->min = min;
  info->max = max;

  info->rms = variance_around(base, size, info->average) / scale;
  info->energy = variance_around(base, size, info->min) / scale;

  y4m2_set_note(frame, name, info, free);

#if 0
#define FMT "%10.3f"
  log_debug("%-15s: min=" FMT ", average=" FMT ", max=" FMT ", rms=" FMT ", energy=" FMT,
            name, info->min, info->average, info->max, info->rms, info->energy);
#undef FMT
#endif
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
