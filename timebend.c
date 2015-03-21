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
    log_debug("timebend rate set to %f", c->rate);
  }
}

static void flush_frame(context *c) {
  y4m2_frame *frame = c->out;

  for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
    unsigned w = frame->i.width / frame->i.plane[pl].xs;
    unsigned h = frame->i.height / frame->i.plane[pl].ys;
    unsigned stride = frame->i.plane[pl].stride;
    double fill = (1 - c->buf_time) * (double) frame->i.plane[pl].fill;

    double *in = c->buf[pl];

    for (unsigned y = 0; y < h; y++) {
      uint8_t *out = frame->plane[pl] + y * stride;
      for (unsigned x = 0; x < w; x++) {
        double sample = *in++ + fill;
        *out++ = (uint8_t) MAX(16, MIN(sample, 240));
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

  y4m2_frame *prev = c->prev;

  if (prev) {
    y4m2_frame *cur = frame ? frame : prev;
    double frame_duration = 1 / c->rate;

    for (double frame_left = frame_duration; frame_left > 0;) {

      double need = 1 - c->buf_time;
      if (need > frame_left) need = frame_left;

      double p_weight = frame_duration < 1 ? 1 : frame_left / frame_duration;
      double c_weight = 1 - p_weight;

      for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
        double cc_weight = c_weight;
        double fill = 0;
        if (!frame) {
          fill = c_weight * (double) cur->i.plane[pl].fill;
          cc_weight = 0;
        }
        unsigned w = cur->i.width / cur->i.plane[pl].xs;
        unsigned h = cur->i.height / cur->i.plane[pl].ys;
        unsigned stride = cur->i.plane[pl].stride;

        if (!c->buf[pl]) c->buf[pl] = alloc(sizeof(double) * w * h);
        double *out = c->buf[pl];

        for (unsigned y = 0; y < h; y++) {
          uint8_t *pin = prev->plane[pl] + y * stride;
          uint8_t *cin = cur->plane[pl] + y * stride;

          for (unsigned x = 0; x < w; x++) {
            *out++ += ((double)(*pin++) * p_weight + (double)(*cin++) * cc_weight + fill) * need;
          }
        }
      }

      frame_left -= need;
      c->buf_time += need;
      if (c->buf_time >= 1) flush_frame(c);
    }
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
