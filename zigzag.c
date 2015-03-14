/* zigzag.c */

#include <stdio.h>
#include <string.h>

#include "downtown.h"
#include "sampler.h"
#include "util.h"
#include "zigzag.h"

void zigzag_permute(const uint8_t *in, uint8_t *out, int w, int h) {
  unsigned limit = w + h - 1;
  uint8_t *op = out;

  for (unsigned x = 0; x < limit; x++) {
    int x0 = x;
    if (x0 >= w) x0 = w - 1;
    int y0 = x - x0;
    int y1 = x;
    if (y1 >= h) y1 = h - 1;
    int x1 = x - y1;

    const uint8_t *inp = (x & 1) ? (in + y0 * w + x0) : (in + y1 * w + x1);
    int stride = (x & 1) ? (w - 1) : (1 - w);
    unsigned count = x0 - x1 + 1;

    for (unsigned i = 0; i < count; i++) {
      *op++ = *inp;
      inp += stride;
    }
  }
}

void zigzag_raster(const uint8_t *in, uint8_t *out, int w, int h) {
  memcpy(out, in, w * h);
}

static void _reverse(uint8_t *buf, size_t size) {
  uint8_t *left = buf;
  uint8_t *right = buf + size - 1;
  while (left < right) {
    uint8_t t = *left;
    *left++ = *right;
    *right-- = t;
  }
}

void zigzag_weave(const uint8_t *in, uint8_t *out, int w, int h) {
  zigzag_raster(in, out, w, h);
  for (int y = 0; y < h; y += 2)
    _reverse(out + y * w, w);
}

size_t zigzag_unity(int w, int h) {
  return (size_t)(w * h);
}

static size_t _init(sampler_context *ctx) {
  ctx->buf = alloc(sizeof(double) * ctx->width * ctx->height);
  return ctx->width * ctx->height;
}

static void _zigzag_permute(double *out, const uint8_t *in, int w, int h) {
  unsigned limit = w + h - 1;
  double *op = out;

  for (unsigned x = 0; x < limit; x++) {
    int x0 = x;
    if (x0 >= w) x0 = w - 1;
    int y0 = x - x0;
    int y1 = x;
    if (y1 >= h) y1 = h - 1;
    int x1 = x - y1;

    const uint8_t *inp = (x & 1) ? (in + y0 * w + x0) : (in + y1 * w + x1);
    int stride = (x & 1) ? (w - 1) : (1 - w);
    unsigned count = x0 - x1 + 1;

    for (unsigned i = 0; i < count; i++) {
      *op++ = sampler_byte2double(*inp);
      inp += stride;
    }
  }
}

static double *_zigzag_sample(sampler_context *ctx, const uint8_t *in)  {
  _zigzag_permute(ctx->buf, in, ctx->width, ctx->height);
  return ctx->buf;
}

static void _b2d(double *out, const uint8_t *in, size_t size) {
  for (unsigned i = 0; i < size; i++) *out++ = sampler_byte2double(*in++);
}

static void _b2dr(double *out, const uint8_t *in, size_t size) {
  double *op = out + size;
  for (unsigned i = 0; i < size; i++) *--op = sampler_byte2double(*in++);
}

static double *_raster_sample(sampler_context *ctx, const uint8_t *in)  {
  _b2d(ctx->buf, in, ctx->width * ctx->height);
  return ctx->buf;
}

static double *_weave_sample(sampler_context *ctx, const uint8_t *in)  {
  for (unsigned y = 0; y < ctx->height; y++) {
    if (y & 1) _b2dr(ctx->buf + ctx->width * y, in + ctx->width * y, ctx->width);
    else _b2d(ctx->buf + ctx->width * y, in + ctx->width * y, ctx->width);
  }
  return ctx->buf;
}

static void _free(sampler_context *ctx) {
  (void) ctx;
}

void zigzag_register(void) {
  sampler_info zigzag = {
    .name = "zigzag",
    .init = _init,
    .sample = _zigzag_sample,
    .free = _free
  };

  sampler_info raster = {
    .name = "raster",
    .init = _init,
    .sample = _raster_sample,
    .free = _free
  };

  sampler_info weave = {
    .name = "weave",
    .init = _init,
    .sample = _weave_sample,
    .free = _free
  };

  sampler_register(&zigzag);
  sampler_register(&raster);
  sampler_register(&weave);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
