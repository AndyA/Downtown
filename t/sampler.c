/* t/sampler.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "sampler.h"
#include "tap.h"
#include "util.h"

static void test_param(void) {
  sampler_params *sp = sampler_parse_params("a=1.24,b=-3,c='Hello, World',d=\"Boo\",e=true");

  ok(0 == strcmp(sp->name, "a"), "a: name matches");
  ok(sp->value == 1.24, "a: value matches");

  ok(0 == strcmp(sp->next->name, "b"), "b: name matches");
  ok(sp->next->value == -3, "b: value matches");

  ok(0 == strcmp(sp->next->next->name, "c"), "c: name matches");
  ok(isnan(sp->next->next->value), "c: value matches");
  ok(!strcmp(sp->next->next->text, "Hello, World"), "c: text matches");

  ok(0 == strcmp(sp->next->next->next->name, "d"), "d: name matches");
  ok(isnan(sp->next->next->next->value), "d: value matches");
  ok(!strcmp(sp->next->next->next->text, "Boo"), "d: text matches");

  ok(0 == strcmp(sp->next->next->next->next->name, "e"), "e: name matches");
  ok(isnan(sp->next->next->next->next->value), "e: value matches");
  ok(!strcmp(sp->next->next->next->next->text, "true"), "e: text matches");

  ok(sp->next->next->next->next->next == NULL, "list ends");

  sampler_free_params(sp);
}

static int done_init = 0;
static int done_free = 0;

size_t test_sampler_init(sampler_context *ctx) {
  (void) ctx;
  ok(0 == strcmp(ctx->class->name, "test_sampler"), "name matchs in init");

  ctx->buf = alloc(sizeof(double) * ctx->width * ctx->height);

  done_init++;
  return ctx->width * ctx->height;
}

double *test_sampler_sample(sampler_context *ctx, const uint8_t *in)  {
  (void)  in;
  return ctx->buf;
}

void test_sampler_free(sampler_context *ctx) {
  (void) ctx;
  ok(0 == strcmp(ctx->class->name, "test_sampler"), "name matchs in free");

  /* ctx->buf is freed for us */

  done_free++;
}

static void test_register(void) {
  sampler_info info = {
    .name = "test_sampler",
    .init = test_sampler_init,
    .sample = test_sampler_sample,
    .free = test_sampler_free
  };

  sampler_register(&info);

  sampler_context *ctx = sampler_new("test_sampler:x=99");

  ok(ctx != NULL, "new");
  ok(0 == strcmp(ctx->class->name, "test_sampler"), "name matchs");
  ok(0 == strcmp(ctx->params->name, "x"), "param name matches");
  ok(ctx->params->value = 99, "param value matches");

  size_t size = sampler_init(ctx, 1000, 800);
  ok(1 == done_init, "init called");
  ok(0 == done_free, "free not called");
  is(ctx->width, 1000, "width set");
  is(ctx->height, 800, "height set");
  ok(ctx->buf != NULL, "buf set");
  if (!is(size, 1000 * 800, "size returned"))
    diag("wanted %d, got %lu", 1000 * 800, (unsigned long) size);

  double *buf = sampler_sample(ctx, NULL);
  ok(buf == ctx->buf, "buf returned");

  sampler_free(ctx);
}

void test_main(void) {
  test_param();
  test_register();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
