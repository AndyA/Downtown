/* progress.c */

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include "log.h"
#include "progress.h"
#include "util.h"
#include "yuv4mpeg2.h"

typedef struct {
  y4m2_output *next;
  int every;
  double start_time;
  double last_time;
  uint64_t last_sequence;
  uint64_t count;
} context;

static double tv_to_seconds(const struct timeval *tv) {
  return (double) tv->tv_sec + (double) tv->tv_usec / 1000000;
}

static double time_of_day(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv_to_seconds(&tv);
}

static context *ctx_new(y4m2_output *next, int every) {
  context *ctx = alloc(sizeof(context));
  ctx->next = next;
  ctx->every = every;
  ctx->last_time = time_of_day();
  return ctx;
}

static void ctx_free(context *ctx) {
  if (ctx) {
    free(ctx);
  }
}

static char *timecode(double elapsed) {
  return ssprintf("%02d:%02d:%02d.%03d",
                  (int)(elapsed / (60 * 60)),
                  (int)(elapsed / 60) % 60,
                  (int) elapsed % 60,
                  (int)(elapsed * 1000) % 1000);
}

static void show_progress(context *c, const y4m2_frame *frame) {
  double now = time_of_day();
  if (frame->sequence == 0)
    c->start_time = now;

  if (now - c->last_time >= c->every) {
    double since = now - c->last_time;
    char *tc = timecode(frame->elapsed);
    char *etc = timecode(now - c->start_time);
    uint64_t done = frame->sequence - c->last_sequence;
    double rate = (double) done / since;
    log_info("Frame: %14llu, Media time: %s, Real time: %s, Rate: %9.2f FPS",
             (unsigned long long) frame->sequence, tc, etc, rate);
    free(etc);
    free(tc);
    c->last_time = now;
    c->last_sequence = frame->sequence;
  }
  c->count++;
}

static void show_summary(context *c) {
  double now = time_of_day();
  double elapsed = now - c->start_time;
  double rate = (double) c->count / elapsed;
  char *etc = timecode(elapsed);

  log_info("Processed %llu frames in %s (%.2f FPS)",
           (unsigned long long) c->count, etc, rate);
  free(etc);
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
    show_progress(c, frame);
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    c->every = 0;
    show_summary(c);
    ctx_free(c);
    break;

  }
}

y4m2_output *progress_filter(y4m2_output *next, int every) {
  return y4m2_output_next(callback, ctx_new(next, every));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
