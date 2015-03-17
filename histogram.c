/* histogram.c */

#include <string.h>

#include "log.h"
#include "histogram.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
} context;

static context *ctx_new(y4m2_output *next) {
  context *c = alloc(sizeof(context));
  c->next = next;
  return c;
}

static void ctx_free(context *c) {
  if (c) {
    free(c);
  }
}

static void _histogram(const uint8_t *in, size_t size, double *hist) {
  unsigned i;

  memset(hist, 0, sizeof(double) * 256);

  for (i = 0; i < size; i++)
    hist[in[i]]++;

  /* Make cummulative */
  for (i = 1; i < 256; i++)
    hist[i] += hist[i - 1];

  for (i = 0; i < 256; i++)
    hist[i] = hist[i] * 255 / hist[255];
}

static void _scale_hist(double *hist, double min, double max) {
  double imin = hist[0];
  double imax = hist[255];
  for (unsigned i = 0; i < 256; i++)
    hist[i] = (hist[i] - imin) / (imax - imin) * (max - min) + min;
}

static void _remap(uint8_t *buf, size_t size, double *lut) {
  for (unsigned i = 0; i < size; i++) {
    double sample = lut[buf[i]];
    buf[i] = (uint8_t) MAX(16, MIN(sample, 235));
  }
}

static void _show_hist(y4m2_frame *frame, const double *hist, colour_bytes *col) {
  double max_hist = hist[255];
  int lx, ly;
  int ox = (frame->i.width - 256) / 2;
  int oy = (frame->i.width - 256) / 2;
  for (int x = 0; x < 256; x++) {
    int y = 255 - hist[x] * 255 / max_hist;
    if (x == 0) y4m2_draw_point(frame, ox + x, oy + y, col->c[cY], col->c[cCb], col->c[cCr]);
    else y4m2_draw_line(frame, ox + lx, oy + ly, ox + x, oy + y, col->c[cY], col->c[cCb], col->c[cCr]);
    lx = x;
    ly = y;
  }
}

static void _eq_frame(y4m2_frame *frame) {
  double hist[256];

  for (unsigned pl = 0; pl < 1; pl++) {
    _histogram(frame->plane[pl], frame->i.plane[pl].size, hist);
    _scale_hist(hist, 16, 235);
    _remap(frame->plane[pl], frame->i.plane[pl].size, hist);

    if (1) {
      colour_bytes before, after, rgb;
      double hist2[256];

      colour_parse_rgb(&rgb, "#0f0");
      colour_b_rgb2yuv(&rgb, &before);

      colour_parse_rgb(&rgb, "#f00");
      colour_b_rgb2yuv(&rgb, &after);

      _histogram(frame->plane[pl], frame->i.plane[pl].size, hist2);

      _show_hist(frame, hist, &before);
      _show_hist(frame, hist2, &after);
    }
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
    _eq_frame(frame);
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *histogram_filter(y4m2_output *next) {
  return y4m2_output_next(callback, ctx_new(next));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
