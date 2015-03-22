/* test-timebend.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "delay.h"
#include "delta.h"
#include "downtown.h"
#include "frameinfo.h"
#include "injector.h"
#include "log.h"
#include "progress.h"
#include "splitter.h"
#include "timebend.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define PROG      "test-timebend"
#define FRAMEINFO "frameinfo.Y"

#define MIN_RATE   1
#define MAX_RATE   100
#define SCALE_RATE 0.02
#define DELAY      50
#define DECAY      0.75

typedef struct {
  double rate;
  double scale;
  double decay;
  double raw_rate;
} rate_work;

static void init_work(rate_work *work) {
  work->rate = 1;
  work->scale = 1;
  work->decay = 0;
}

static double next_rate(rate_work *w, double next) {
  w->scale = w->scale * w->decay + 1;
  w->rate = w->rate * w->decay + next;
  return w->rate / w->scale;
}

static double calc_rate(void *ctx) {
  rate_work *w = (rate_work *) ctx;

  double rate = next_rate(w, w->raw_rate);

  log_debug("raw_rate: %8.3f, rate: %8.3f", w->raw_rate, rate);

  return rate;
}

static y4m2_frame *catch_analysis(y4m2_frame *frame, void *ctx) {
  rate_work *w = (rate_work *) ctx;

  frameinfo *fi = (frameinfo *) y4m2_find_note(frame, FRAMEINFO);
  double rms = MAX(fi->rms, 0.00000001);
  double rms2 = rms * rms;
  w->raw_rate = MAX(MIN_RATE, MIN(SCALE_RATE / rms2, MAX_RATE));
  return frame;
}

typedef struct {
  y4m2_frame *src;
} dup_notes;

static y4m2_frame *get_notes(y4m2_frame *frame, void *ctx) {
  dup_notes *dn = (dup_notes *) ctx;

  y4m2_retain_frame(frame);
  y4m2_release_frame(dn->src);
  dn->src = frame;

  return frame;
}

static y4m2_frame *put_notes(y4m2_frame *frame, void *ctx) {
  dup_notes *dn = (dup_notes *) ctx;
  if (dn->src) y4m2_copy_notes(frame, dn->src);
  return frame;
}

int main(void) {
  rate_work work;
  dup_notes dup;

  dup.src = NULL;

  init_work(&work);
  work.decay = DECAY;

  log_info("Starting " PROG);

  /* process chain */
  y4m2_output *process = y4m2_output_file(stdout);
  process = timebend_filter_cb(process, calc_rate, &work);
  process = frameinfo_grapher(process, FRAMEINFO, "rms", "#f00");

  process = delay_filter(process, DELAY);
  process = injector_filter(process, put_notes, &dup);

  /* analyse chain */
  y4m2_output *analyse = y4m2_output_null();
  analyse = injector_filter(analyse, get_notes, &dup);
  analyse = injector_filter(analyse, catch_analysis, &work);
  analyse = frameinfo_filter(analyse);
  analyse = delta_filter(analyse);

  /* split */
  y4m2_output *head = splitter_filter(analyse, process, NULL);
  head = progress_filter(head, PROGRESS_RATE);
  y4m2_parse(stdin, head);

  y4m2_release_frame(dup.src);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
