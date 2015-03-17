/* t/zigzag.c */

#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "sampler.h"
#include "tap.h"
#include "zigzag.h"

typedef struct {
  int w, h;
  int x, y;
  int delta;
} test_zz;

static void init_zz(test_zz *zz, int w, int h) {
  zz->w = w;
  zz->h = h;
  zz->x = 0;
  zz->y = 0;
  zz->delta = 1;
}

static int next_zz(test_zz *zz, int *x, int *y) {
  if (zz->x == zz->w || zz->y == zz->h) return 0;

  *x = zz->x;
  *y = zz->y;

  int xx = zz->x + zz->delta;
  int yy = zz->y - zz->delta;

  if (xx >= 0 && xx < zz->w && yy >= 0 && yy < zz->h) {
    zz->x = xx;
    zz->y = yy;
    return 1;
  }

  zz->delta = -zz->delta;
  if (zz->delta < 0) {
    if (zz->x < zz->w - 1) zz->x++;
    else zz->y++;
  }
  else {
    if (zz->y < zz->h - 1) zz->y++;
    else zz->x++;
  }

  return 1;
}

static void permute(const uint8_t *in, double *out, int w, int h) {
  test_zz zz;
  int x, y;
  init_zz(&zz, w, h);
  while (next_zz(&zz, &x, &y)) {
    *out++ = ((double)in[x + y * w] - 128) / 128;
  }
}

static void dump_grid(uint8_t *grid, int w, int h) {
  for (int y = 0; y < h; y++) {
    fprintf(stderr, "#     %3d |", y);
    for (int x = 0; x < w; x++) {
      fprintf(stderr, " %3d", grid[x + y * w]);
    }
    fprintf(stderr, " |\n");
  }
}

static void dump_dgrid(double *grid, int w, int h) {
  for (int y = 0; y < h; y++) {
    fprintf(stderr, "#     %3d |", y);
    for (int x = 0; x < w; x++) {
      fprintf(stderr, " %7.3f", grid[x + y * w]);
    }
    fprintf(stderr, " |\n");
  }
}

static void test_grid(int w, int h) {
  uint8_t in[w * h];
  double ref[w * h];

  for (int i = 0; i < w * h; i++) in[i] = i + 1;

  permute(in, ref, w, h);

  sampler_context *ctx = sampler_new("zigzag", "zigzag");
  size_t size = sampler_init(ctx, w, h);
  double *out = sampler_sample(ctx, in);

  ok(size == (size_t)(w * h), "size OK");

  if (!ok(0 == memcmp(ref, out, w * h), "%d x %d", w, h)) {
    diag("In:");
    dump_grid(in, w, h);
    diag("Wanted:");
    dump_dgrid(ref, w, h);
    diag("Got:");
    dump_dgrid(out, w, h);
  }
  sampler_free(ctx);
}

static void test_zigzag(void) {
  zigzag_register();

  test_grid(1, 1);
  test_grid(1, 2);
  test_grid(2, 1);
  test_grid(2, 2);
  test_grid(10, 10);
  test_grid(10, 11);
  test_grid(11, 10);
  test_grid(256, 256);
}

void test_main(void) {
  test_zigzag();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
