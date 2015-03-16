/* frameinfo.c */

#include <math.h>
#include <string.h>

#include "colour.h"
#include "frameinfo.h"
#include "log.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
} info_context;

static info_context *info_ctx_new(y4m2_output *next) {
  info_context *ctx = alloc(sizeof(info_context));
  ctx->next = next;
  return ctx;
}

static void info_ctx_free(info_context *ctx) {
  free(ctx);
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

static void info_callback(y4m2_reason reason,
                          const y4m2_parameters *parms,
                          y4m2_frame *frame,
                          void *ctx) {
  info_context *c = ctx;

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
    info_ctx_free(c);
    break;

  }
}

y4m2_output *frameinfo_filter(y4m2_output *next) {
  return y4m2_output_next(info_callback, info_ctx_new(next));
}

frameinfo *frameinfo_get(const y4m2_frame *frame, const char *name) {
  void *note = y4m2_need_note(frame, name);
  return (frameinfo *) note;
}

typedef struct {
  y4m2_output *next;
  char *note;
  size_t offset;
  double prev;
  int warned;
  colour_bytes col;
} grapher_context;

#define X(x) { #x, offsetof(frameinfo, x) },
static struct info_field {
  const char *name;
  size_t offset;
} _fields[] = {
  FRAMEINFO_FIELDS
  { NULL, 0 }
};
#undef X

static struct info_field *_find_info(const char *name) {
  for (struct info_field *f = _fields; f->name; f++)
    if (0 == strcmp(f->name, name)) return f;
  die("No info field called %s", name);
  return NULL;
}

static grapher_context *grapher_ctx_new(y4m2_output *next, const char *note, const char *field) {
  struct info_field *f = _find_info(field);
  grapher_context *ctx = alloc(sizeof(grapher_context));
  ctx->next = next;
  ctx->note = sstrdup(note);
  ctx->offset = f->offset;
  ctx->prev = NAN;

  colour_bytes red;
  red.c[cR] = 255;
  red.c[cG] = 0;
  red.c[cB] = 0;

  colour_b_rgb2yuv(&red, &ctx->col);

  return ctx;
}

static void grapher_ctx_free(grapher_context *ctx) {
  free(ctx->note);
  free(ctx);
}

static void _plot_info(grapher_context *c, y4m2_frame *frame) {
  frameinfo *info = y4m2_find_note(frame, c->note);

  if (!info) {
    if (!c->warned++) log_warning("No note %s", c->note);
    return;
  }

  double v = *((double *)((char *) info + c->offset));
  int x = frame->i.width - 1;
  int y1 = (1 - v) * frame->i.height;

  /*  log_debug("plotting %5d (%3d,%3d,%3d)", y1, c->col.c[cY], c->col.c[cCb], c->col.c[cCr]);*/

  if (!isnan(c->prev)) {
    int y0 = (1 - c->prev) * frame->i.height;
    y4m2_draw_line(frame, x - 1, y0, x, y1, c->col.c[cY], c->col.c[cCb], c->col.c[cCr]);
  }
  else {
    y4m2_draw_point(frame, x, y1, c->col.c[cY], c->col.c[cCb], c->col.c[cCr]);
  }

  c->prev = v;
}

static void grapher_callback(y4m2_reason reason,
                             const y4m2_parameters *parms,
                             y4m2_frame *frame,
                             void *ctx) {
  grapher_context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    y4m2_emit_start(c->next, parms);
    break;

  case Y4M2_FRAME:
    _plot_info(c, frame);
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    grapher_ctx_free(c);
    break;

  }
}

y4m2_output *frameinfo_grapher(y4m2_output *next, const char *note, const char *field) {
  return y4m2_output_next(grapher_callback, grapher_ctx_new(next, note, field));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
