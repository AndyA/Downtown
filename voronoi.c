/* voronoi.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "quadtree.h"
#include "sampler.h"
#include "util.h"
#include "voronoi.h"

typedef struct {
  unsigned *xlate;
  double *area;
  quadtree *qt;
  unsigned n_points;
} voronoi_context;

static voronoi_context *_init(sampler_context *ctx) {
  voronoi_context *vc = alloc(sizeof(voronoi_context));

  vc->qt = quadtree_new(ctx->width, ctx->height);

  ctx->user = vc;
  return vc;
}

static void _setup(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;

  vc->xlate = alloc(sizeof(unsigned) * ctx->width * ctx->height);

  vc->n_points = quadtree_used(vc->qt);
  vc->area = alloc(sizeof(double) * vc->n_points);
  ctx->buf = alloc(sizeof(double) * vc->n_points);

  for (unsigned y = 0; y < ctx->height; y++) {
    for (unsigned x = 0; x < ctx->width; x++) {
      const quadtree_point *pt = quadtree_nearest(vc->qt, x, y);
      vc->area[pt->tag]++;
      vc->xlate[x + y * ctx->width] = pt->tag;
    }
  }
}

static size_t _spiral_init(sampler_context *ctx) {
  voronoi_context *vc = _init(ctx);

  int cx = ctx->width / 2;
  int cy = ctx->height / 2;

  /* compute points */
  double a = 0;
  double r = 1;
  int lx = 0, ly = 0;
  unsigned tag = 0;
  for (;;) {
    int px = cx + (int)(sin(a) * r);
    int py = cy + (int)(cos(a) * r);

    /*    log_debug("a=%f, r=%f, px=%d, py=%d", a, r, px, py);*/

    a += 2 / r;
    r += 1 / r;

    if (px == lx && py == ly) continue;
    lx = px;
    ly = py;

    if (!quadtree_add(vc->qt, px, py, tag++)) break;
  }

  _setup(ctx);
  return vc->n_points;
}

static double *_sample(sampler_context *ctx, const uint8_t *in)  {
  voronoi_context *vc = ctx->user;
  size_t limit = ctx->width * ctx->height;
  unsigned i;

  memset(ctx->buf, 0, sizeof(double) * vc->n_points);
  for (i = 0; i < limit; i++)
    ctx->buf[vc->xlate[i]] += sampler_byte2double(in[i]);
  for (i = 0; i < vc->n_points; i++)
    if (vc->area[i]) ctx->buf[i] /= vc->area[i];

  return ctx->buf;
}


static void _free(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;
  free(vc->xlate);
  free(vc->area);
  quadtree_free(vc->qt);
}

void voronoi_register(void) {
  sampler_info spiral = {
    .name = "spiral",
    .init = _spiral_init,
    .sample = _sample,
    .free = _free
  };

  sampler_register(&spiral);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
