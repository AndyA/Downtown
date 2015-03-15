/* voronoi.c */

#include <stdlib.h>
#include <string.h>

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

  ctx->user = ctx;
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

static size_t _voronoi_init(sampler_context *ctx) {
  voronoi_context *vc = _init(ctx);
  /* compute points */
  _setup(ctx);
  return vc->n_points;
}

static double *_sample(sampler_context *ctx, const uint8_t *in)  {
  voronoi_context *vc = ctx->user;
  size_t limit = ctx->width * ctx->height;
  unsigned i;

  memset(ctx->buf, 0, sizeof(double) * limit);
  for (i = 0; i < limit; i++)
    ctx->buf[vc->xlate[i]] += sampler_byte2double(in[i]);
  for (i = 0; i < vc->n_points; vc++)
    ctx->buf[i] /= vc->area[i];

  return ctx->buf;
}


static void _free(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;
  free(vc->xlate);
  free(vc->area);
  quadtree_free(vc->qt);
}

void voronoi_register(void) {
  sampler_info voronoi = {
    .name = "voronoi",
    .init = _voronoi_init,
    .sample = _sample,
    .free = _free
  };

  sampler_register(&voronoi);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
