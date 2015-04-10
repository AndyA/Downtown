/* scale.c */

#include <string.h>
#include <libswscale/swscale.h>

#include "log.h"
#include "scale.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  unsigned width, height;
  y4m2_parameters *oparms;
  y4m2_frame *out;
  struct SwsContext *swc;
} context;

static context *ctx_new(y4m2_output *next, unsigned width, unsigned height) {
  context *c = alloc(sizeof(context));
  c->next = next;
  c->width = width;
  c->height = height;
  return c;
}

static void ctx_free(context *c) {
  if (c) {
    y4m2_free_parms(c->oparms);
    y4m2_release_frame(c->out);
    sws_freeContext(c->swc);
    free(c);
  }
}

static enum AVPixelFormat get_pix_fmt(const y4m2_parameters *parms) {
  const char *cs = y4m2_get_parm(parms, "C");

  if (!cs ||
  !strcmp("420", cs) ||
  !strcmp("420jpeg", cs) ||
  !strcmp("420mpeg2", cs) ||
  !strcmp("420paldv", cs))
    return AV_PIX_FMT_YUV420P;

  if (!strcmp("422", cs))
    return AV_PIX_FMT_YUV422P;

  if (!strcmp("444", cs))
    return AV_PIX_FMT_YUV444P;

  die("Unknown colourspace %s\n", cs);
  return AV_PIX_FMT_NONE;
}

static void setup(context *c, const y4m2_parameters *parms) {
  unsigned sw, sh;
  y4m2_get_parm_size(parms, &sw, &sh);

  if (sw == c->width && sh == c->height) {
    log_info("No scaling needed");
    return;
  }

  enum AVPixelFormat pix_fmt = get_pix_fmt(parms);

  c->oparms = y4m2_adjust_parms(parms, "W%u H%u", c->width, c->height);

  c->out = y4m2_new_frame(c->oparms);
  c->swc = sws_getContext(sw, sh, pix_fmt,
                          c->width, c->height, pix_fmt,
                          SWS_BICUBIC, NULL, NULL, NULL);

  if (!c->swc) die("Failed to create scaling context");

}

static void frame_setup(uint8_t *plane[3], int stride[3], const y4m2_frame *frame) {
  for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++) {
    plane[pl] = frame->plane[pl];
    stride[pl] = frame->i.plane[pl].stride;
  }
}

static void scale(context *c, const y4m2_parameters *parms, y4m2_frame *frame) {
  uint8_t *src[3];
  int src_stride[3];

  uint8_t *dst[3];
  int dst_stride[3];

  frame_setup(src, src_stride, frame);
  frame_setup(dst, dst_stride, c->out);

  sws_scale(c->swc, (const uint8_t *const *) src, src_stride, 0, frame->i.height, dst, dst_stride);
  y4m2_emit_frame(c->next, c->oparms, y4m2_retain_frame(c->out));
  y4m2_release_frame(frame);
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    setup(c, parms);
    y4m2_emit_start(c->next, c->oparms ? c->oparms : parms);
    break;

  case Y4M2_FRAME:
    if (c->oparms)
      scale(c, parms, frame);
    else
      y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    ctx_free(c);
    break;

  }
}

y4m2_output *scale_filter(y4m2_output *next, unsigned width, unsigned height) {
  return y4m2_output_next(callback, ctx_new(next, width, height));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
