/* timebend.c */

#include <string.h>

#include "log.h"
#include "timebend.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  double rate;
  double *buf[Y4M2_N_PLANE];
  double buf_time;
  y4m2_frame *prev;
  y4m2_frame *out;
  y4m2_parameters *parms;
} context;

static context *ctx_new(y4m2_output *next, double rate) {
  context *c = alloc(sizeof(context));
  c->next = next;
  c->rate = rate;
  return c;
}

static void ctx_free(context *c) {
  if (c) {
    for (int i = 0; i < Y4M2_N_PLANE; i++) free(c->buf[i]);
    y4m2_release_frame(c->prev);
    y4m2_release_frame(c->out);
    y4m2_free_parms(c->parms);
    free(c);
  }
}

static void check_note(context *c, y4m2_frame *frame) {
  timebend_rate_note *note = y4m2_find_note(frame, timebend_RATE_NOTE);
  if (note) {
    c->rate = note->rate;
    log_debug("timebend rate set to %d", c->rate);
  }
}

static void flush_frame(context *c) {
  y4m2_frame *frame = c->out;

  for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
    unsigned w = frame->i.width / frame->i.plane[pl].xs;
    unsigned h = frame->i.height / frame->i.plane[pl].ys;
    unsigned stride = frame->i.plane[pl].stride;

    double *in = c->buf[pl];

    for (unsigned y = 0; y < h; y++) {
      uint8_t *out = frame->plane[pl] + y * stride;
      for (unsigned x = 0; x < w; x++) {
        *out++ = (uint8_t) MAX(16, MIN(*in++, 240));
      }
    }

    memset(c->buf[pl], 0, sizeof(double) * w * h);
  }

  c->buf_time = 0;
  y4m2_emit_frame(c->next, c->parms, frame);
}

static void bend_frame(context *c, y4m2_frame *frame) {
  if (frame) {
    check_note(c, frame);
    if (!c->out) c->out = y4m2_like_frame(frame);
  }

  double frame_duration = 1 / c->rate;
  y4m2_frame *prev = c->prev;
  y4m2_frame *cur = frame ? frame : prev;

  while (frame_duration > 0) {
    double p_weight, c_weight;

    if (frame_duration < 1) {
      p_weight = 1;
      c_weight = 0;
    }
    else {
      p_weight = 1 - c->buf_time;
      c_weight = c->buf_time;
    }

    if (!frame) c_weight = 0;

    double need = 1 - c->buf_time;
    if (need > frame_duration) need = frame_duration;

    for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
      unsigned w = cur->i.width / cur->i.plane[pl].xs;
      unsigned h = cur->i.height / cur->i.plane[pl].ys;
      unsigned stride = cur->i.plane[pl].stride;

      if (!c->buf[pl]) c->buf[pl] = alloc(sizeof(double) * w * h);
      double *out = c->buf[pl];

      for (unsigned y = 0; y < h; y++) {
        uint8_t *pin = prev->plane[pl] + y * stride;
        uint8_t *cin = cur->plane[pl] + y * stride;

        for (unsigned x = 0; x < w; x++) {
          *out++ += ((double)(*pin++) * p_weight + (double)(*cin) * c_weight) * need;
        }
      }
    }

    frame_duration -= need;
    c->buf_time += need;
    if (c->buf_time >= 1) flush_frame(c);
  }

  y4m2_retain_frame(frame);
  y4m2_release_frame(c->prev);
  c->prev = frame;
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    c->parms = y4m2_clone_parms(parms);
    y4m2_emit_start(c->next, parms);
    break;

  case Y4M2_FRAME:
    bend_frame(c, frame);
    break;

  case Y4M2_END:
    bend_frame(c, NULL);
    if (c->buf_time > 0) flush_frame(c);
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *timebend_filter(y4m2_output *next, double rate) {
  return y4m2_output_next(callback, ctx_new(next, rate));
}

void timebend_set_rate(y4m2_frame *frame, double rate) {
  timebend_rate_note *note = alloc(sizeof(timebend_rate_note));
  note->rate = rate;
  y4m2_set_note(frame, timebend_RATE_NOTE, note, free);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
