/* centre.c */

#include <string.h>

#include "centre.h"
#include "log.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  y4m2_parameters *parms;
  y4m2_frame *frame;
} context;

static context *ctx_new(y4m2_output *next) {
  context *ctx = alloc(sizeof(context));
  ctx->next = next;
  return ctx;
}

static void ctx_free(context *ctx) {
  if (ctx) {
    y4m2_free_parms(ctx->parms);
    y4m2_release_frame(ctx->frame);
    free(ctx);
  }
}

static y4m2_parameters *_cook_parms(const y4m2_parameters *parms) {
  y4m2_frame_info info;
  y4m2_parse_frame_info(&info, parms);
  return y4m2_adjust_parms(parms, "W%d H%d", info.width * 2, info.height * 2);
}

static void centre_frame(const y4m2_frame *frame, int plane, int *cx, int *cy) {
  unsigned total = 0;
  uint8_t *buf = frame->plane[plane];
  int x, y;

  int width = frame->i.width / frame->i.plane[plane].xs;
  int height = frame->i.height / frame->i.plane[plane].ys;
  size_t size = frame->i.plane[plane].size;

  for (x = 0; x < (int) size; x++) total += buf[x];

  unsigned accum = 0;
  for (x = 0; x < width; x++) {
    for (y = 0; y < height; y++)
      accum += buf[x + y * width];
    if (accum >= total / 2) break;
  }
  *cx = x;

  accum = 0;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++)
      accum += buf[x + y * width];
    if (accum >= total / 2) break;
  }
  *cy = y;
}

/* assume matching colourspace */

static void blit_plane(y4m2_frame *dst, const y4m2_frame *src, int x, int y, int pl) {
  int sx0 = 0;
  int sy0 = 0;
  int sx1 = sx0 + src->i.width / src->i.plane[pl].xs;
  int sy1 = sy0 + src->i.height / src->i.plane[pl].ys;

  int dx0 = x / dst->i.plane[pl].xs;
  int dy0 = y / dst->i.plane[pl].ys;
  int dx1 = dx0 + sx1;
  int dy1 = dy0 + sy1;

  int sxm = sx1;
  int dxm = dst->i.width / dst->i.plane[pl].xs;
  int dym = dst->i.height / dst->i.plane[pl].ys;

  if (dx0 < 0) {
    sx0 -= dx0;
    dx0 -= dx0;
  }
  if (dy0 < 0) {
    sy0 -= dy0;
    dy0 -= dy0;
  }
  if (dx1 >= dxm) {
    sx1 -= dx1 - dxm;
    dx1 -= dx1 - dxm;
  }
  if (dy1 >= dym) {
    sy1 -= dy1 - dym;
    dy1 -= dy1 - dym;
  }

  if (sx0 < sx1)
    for (int y = sy0; y < sy1; y++)
      memcpy(dst->plane[pl] + dx0 + (dy0 + y) * dxm, src->plane[pl] + sx0 + (sy0 + y) * sxm, sx1 - sx0);

}

static void blit(y4m2_frame *dst, const y4m2_frame *src, int x, int y) {
  for (int pl = 0; pl < Y4M2_N_PLANE; pl++)
    blit_plane(dst, src, x, y, pl);
}

static void align_frame(y4m2_frame *dst, const y4m2_frame *src) {
  int cx, cy;
  centre_frame(src, Y4M2_Y_PLANE, &cx, &cy);

  /*  log_debug("centre=%4d, %4d", cx, cy);*/

  int px = dst->i.width / 2 - cx;
  int py = dst->i.height / 2 - cy;

  blit(dst, src, px, py);
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  (void) parms;
  (void) frame;
  context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    c->parms = _cook_parms(parms);
    c->frame = y4m2_new_frame(c->parms);
    y4m2_emit_start(c->next, c->parms);
    break;

  case Y4M2_FRAME:
    y4m2_clear_frame(c->frame);
    align_frame(c->frame, frame);
    y4m2_emit_frame(c->next, c->parms, c->frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *filter_centre(y4m2_output *next) {
  return y4m2_output_next(callback, ctx_new(next));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
