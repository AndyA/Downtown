/* voronoi.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "quadtree.h"
#include "sampler.h"
#include "util.h"
#include "voronoi.h"
#include "y4m2png.h"
#include "yuv4mpeg2.h"

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

static int _rand(int lo, int hi) {
  unsigned range = hi - lo;
  unsigned shift = 0;
  while ((1u << shift) < range) shift++;
  long rv, mask = (1 << shift) - 1;
  do {
    rv = random() & mask;
  }
  while (rv >= range);
  return rv + lo;
}

static void _random_colour(colour_bytes *col) {
  col->c[cY] = _rand(16, 235);
  col->c[cCb] = _rand(16, 240);
  col->c[cCr] = _rand(16, 240);
}

static void _debug_dump_png(sampler_context *ctx, const char *name) {
  voronoi_context *vc = ctx->user;

  y4m2_parameters *parms = y4m2_adjust_parms(NULL, "C444 W%u H%u", ctx->width, ctx->height);
  y4m2_frame *frame = y4m2_new_frame(parms);

  colour_bytes *col_map = alloc(sizeof(colour_bytes) * vc->n_points);
  for (unsigned i = 0; i < vc->n_points; i++)
    _random_colour(&col_map[i]);

  uint8_t *plane[Y4M2_N_PLANE];
  unsigned xs[Y4M2_N_PLANE], ys[Y4M2_N_PLANE];

  y4m2__plane_map(frame, plane, xs, ys);

  for (unsigned y = 0; y < ctx->width; y++) {
    for (unsigned x = 0; x < ctx->width; x++) {
      unsigned xl = vc->xlate[x + y * ctx->width];
      if (xl != vc->n_points) {
        colour_bytes *cp = &col_map[xl];
        for (unsigned p = 0; p < Y4M2_N_PLANE; p++)
          plane[p][x + y * ctx->width] = cp->c[p];
      }
    }
  }

  y4m2_png_write_file(frame, name);

  free(col_map);
  y4m2_release_frame(frame);
  y4m2_free_parms(parms);
}

static void _debug_dump(sampler_context *ctx) {
  const char *dump_png = sampler_optional_text(ctx->params, "dump");
  if (dump_png) {
    char *name = ssprintf(dump_png, ctx->name);
    log_debug("Writing voronoi map to %s", name);
    _debug_dump_png(ctx, name);
    free(name);
  }
}

static void _area_limit(sampler_context *ctx, double limit) {
  voronoi_context *vc = ctx->user;

  double avg = ctx->width * ctx->height / vc->n_points;
  double max = avg * limit;

  for (unsigned y = 0; y < ctx->height; y++) {
    for (unsigned x = 0; x < ctx->width; x++) {
      unsigned xl = vc->xlate[x + y * ctx->width];
      if (xl == vc->n_points) continue;
      if (vc->area[xl] > max) vc->xlate[x + y * ctx->width] = vc->n_points;
    }
  }

  for (unsigned i = 0; i < vc->n_points; i++)
    if (vc->area[i] > max) vc->area[i] = 0;
}

static void _edge_trim(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;
  uint8_t *edge = alloc(vc->n_points);

  for (unsigned y = 0; y < ctx->height; y++) {
    unsigned dx = (y == 0 || y == ctx->height - 1) ? 1 : ctx->width - 1;
    for (unsigned x = 0; x < ctx->width; x += dx) {
      unsigned xl = vc->xlate[x + y * ctx->width];
      if (xl == vc->n_points) continue;
      edge[xl] = 1;
    }
  }

  for (unsigned y = 0; y < ctx->height; y++) {
    for (unsigned x = 0; x < ctx->width; x++) {
      unsigned xl = vc->xlate[x + y * ctx->width];
      if (xl == vc->n_points) continue;
      if (edge[xl]) vc->xlate[x + y * ctx->width] = vc->n_points;
    }
  }

  for (unsigned i = 0; i < vc->n_points; i++)
    if (edge[i]) vc->area[i] = 0;

  free(edge);
}

static size_t _count_points(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;
  for (unsigned np = vc->n_points; np > 0; np--)
    if (vc->area[np - 1] > 0) return np;
  return 0;
}

static size_t _setup(sampler_context *ctx) {
  voronoi_context *vc = ctx->user;

  vc->xlate = alloc(sizeof(unsigned) * ctx->width * ctx->height);

  vc->n_points = quadtree_used(vc->qt);
  vc->area = alloc(sizeof(double) * vc->n_points);
  ctx->buf = alloc(sizeof(double) * vc->n_points);

  /* build voronoi map */
  for (unsigned y = 0; y < ctx->height; y++) {
    for (unsigned x = 0; x < ctx->width; x++) {
      const quadtree_point *pt = quadtree_nearest(vc->qt, x, y);
      vc->area[pt->tag]++;
      vc->xlate[x + y * ctx->width] = pt->tag;
    }
  }

  _area_limit(ctx, sampler_require_double(ctx->params, "area_limit"));
  if (sampler_require_double(ctx->params, "edge_trim")) _edge_trim(ctx);

  _debug_dump(ctx);
  return _count_points(ctx);
}

static size_t _spiral_init(sampler_context *ctx) {
  voronoi_context *vc = _init(ctx);

  int cx = ctx->width / 2;
  int cy = ctx->height / 2;

  /* compute points */
  double a = 0;
  double r = 1;

  double a_rate = sampler_require_double(ctx->params, "a_rate"); /* default 2.0 */
  double r_rate = sampler_require_double(ctx->params, "r_rate"); /* default 1.0 */

  int lx = 0, ly = 0;
  unsigned tag = 0;
  for (;;) {
    int px = cx + (int)(sin(a) * r);
    int py = cy + (int)(cos(a) * r);

    a += a_rate / r;
    r += r_rate / r;

    if (px == lx && py == ly) continue;
    lx = px;
    ly = py;

    if (!quadtree_add(vc->qt, px, py, tag++)) break;
  }

  return _setup(ctx);
}

static double *_sample(sampler_context *ctx, const uint8_t *in)  {
  voronoi_context *vc = ctx->user;
  size_t limit = ctx->width * ctx->height;
  unsigned i;

  memset(ctx->buf, 0, sizeof(double) * vc->n_points);
  for (i = 0; i < limit; i++) {
    unsigned xl = vc->xlate[i];
    if (xl != vc->n_points)
      ctx->buf[xl] += sampler_byte2double(in[i]);
  }

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
    .free = _free,
    .default_config = "r_rate=5,a_rate=5,area_limit=1.2,edge_trim=1"
  };

  sampler_register(&spiral);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
*/
