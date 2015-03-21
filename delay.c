/* delay.c */

#include <string.h>

#include "log.h"
#include "delay.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_parameters *parms;
  y4m2_frame *frame;
} framestuff;

typedef struct {
  y4m2_output *next;
  unsigned length;

  framestuff *queue;
  unsigned used;
} context;

static context *ctx_new(y4m2_output *next, unsigned length) {
  context *c = alloc(sizeof(context));
  c->next = next;
  c->length = length;
  c->queue = alloc(sizeof(y4m2_frame) * length);
  return c;
}

static void put_stuff(framestuff *fs, const y4m2_parameters *parms, y4m2_frame *frame) {
  fs->parms = y4m2_clone_parms(parms);
  fs->frame = y4m2_retain_frame(frame);
}

static void free_stuff(framestuff *fs) {
  y4m2_free_parms(fs->parms);
  y4m2_release_frame(fs->frame);
}

static void ctx_free(context *c) {
  if (c) {
    for (unsigned i = 0; i < c->used; i++)
      free_stuff(&c->queue[i]);
    free(c->queue);
    free(c);
  }
}

static void emit_frame(context *c) {
  if (c->used == 0) die("Queue empty");
  framestuff *fs = &c->queue[0];
  y4m2_emit_frame(c->next, fs->parms, fs->frame);
  free_stuff(fs);
  c->used--;
  memmove(&c->queue[0], &c->queue[1], sizeof(framestuff) * c->used);
}

static void put_frame(context *c, const y4m2_parameters *parms, y4m2_frame *frame) {
  if (c->used == c->length) emit_frame(c);
  put_stuff(&c->queue[c->used++], parms, frame);
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
    put_frame(c, parms, frame);
    break;

  case Y4M2_END:
    while (c->used) emit_frame(c);
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *delay_filter(y4m2_output *next, unsigned length) {
  return y4m2_output_next(callback, ctx_new(next, length));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
