/* voronoi.c */

#include <stdlib.h>

#include "quadtree.h"
#include "sampler.h"
#include "util.h"
#include "voronoi.h"

typedef struct {
  unsigned *xlate;

} voronoi_context;

static voronoi_context *_init(sampler_context *ctx) {
  voronoi_context *vc = alloc(sizeof(voronoi_context));

  ctx->buf = alloc(sizeof(double) * ctx->width * ctx->height);
  vc->xlate = alloc(sizeof(unsigned) * ctx->width * ctx->height);

  ctx->user = ctx;
  return vc;
}

static void _setup(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;
}

static size_t _voronoi_init(sampler_context *ctx) {
  voronoi_context *vc = _init(ctx);
  /* compute points */
  _setup(ctx);
  return ctx->width * ctx->height;
}

static double *_sample(sampler_context *ctx, const uint8_t *in)  {
  voronoi_context *vc = ctx->user;
  size_t limit = ctx->width * ctx->height;
  for (unsigned i = 0; i < limit; i++) {
    ctx->buf[vc->xlate[i]] = sampler_byte2double(in[i]);
  }
  return ctx->buf;
}


static void _free(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;
  (void) ctx;
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
