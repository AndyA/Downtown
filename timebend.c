/* timebend.c */

#include <math.h>
#include <string.h>

#include "log.h"
#include "timebend.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define NOWT 0.00001

typedef struct {
  y4m2_output *next;

  double rate;
  timebend_rate_cb cb;
  void *ctx;

  double *buf[Y4M2_N_PLANE];
  double buf_time;
  y4m2_frame *prev;
  y4m2_frame *out;
  y4m2_parameters *parms;
} context;

static double _fixed_rate(void *ctx) {
  context *c = (context *) ctx;
  return c->rate;
}

static context *ctx_new(y4m2_output *next, double rate, timebend_rate_cb cb, void *ctx) {
  context *c = alloc(sizeof(context));

  c->next = next;
  c->rate = rate;

  c->cb = cb ? cb : _fixed_rate;;
  c->ctx = cb ? ctx : c;;

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

static void update_rate(context *c) {
  double rate = c->cb(c->ctx);
  if (isnan(rate)) {
    log_warning("Attempt to set rate to NAN");
    return;
  }
  c->rate = MAX(rate, NOWT);
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
  y4m2_emit_frame(c->next, c->parms, y4m2_retain_frame(frame));
  update_rate(c);
}

static void bend_frame(context *c, y4m2_frame *frame) {
  if (frame && !c->out) c->out = y4m2_like_frame(frame);

  if (c->prev) {
    double frame_duration = 1;
    while (frame_duration > NOWT) {

      double got      = frame_duration / c->rate;
      double need     = 1 - c->buf_time;

      if (need > got) need = got;

      double p_weight = frame_duration * need;
      double c_weight = (1 - frame_duration) * need;

      for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
        unsigned w = c->prev->i.width / c->prev->i.plane[pl].xs;
        unsigned h = c->prev->i.height / c->prev->i.plane[pl].ys;
        unsigned stride = c->prev->i.plane[pl].stride;

        if (!c->buf[pl]) c->buf[pl] = alloc(sizeof(double) * w * h);
        double *out = c->buf[pl];

        if (c_weight > NOWT) {
          if (frame) {
            for (unsigned y = 0; y < h; y++) {
              uint8_t *pin = c->prev->plane[pl] + y * stride;
              uint8_t *cin = frame->plane[pl] + y * stride;
              for (unsigned x = 0; x < w; x++) {
                *out++ += ((double)(*pin++) * p_weight) + ((double)(*cin++) * c_weight);
              }
            }
          }
          else {
            double fill = (double) c->prev->i.plane[pl].fill * c_weight;
            for (unsigned y = 0; y < h; y++) {
              uint8_t *pin = c->prev->plane[pl] + y * stride;
              for (unsigned x = 0; x < w; x++) {
                *out++ += ((double)(*pin++) * p_weight) + fill;
              }
            }
          }
        }
        else {
          for (unsigned y = 0; y < h; y++) {
            uint8_t *pin = c->prev->plane[pl] + y * stride;
            for (unsigned x = 0; x < w; x++) {
              *out++ += (double)(*pin++) * p_weight;
            }
          }
        }
      }

      frame_duration -= need * c->rate;
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
    /* get the initial rate */
    update_rate(c);
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
  return y4m2_output_next(callback, ctx_new(next, rate, NULL, NULL));
}

y4m2_output *timebend_filter_cb(y4m2_output *next, timebend_rate_cb cb, void *ctx) {
  return y4m2_output_next(callback, ctx_new(next, 1, cb, ctx));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
