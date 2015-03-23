/* test-timebend.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "average.h"
#include "delay.h"
#include "delta.h"
#include "downtown.h"
#include "frameinfo.h"
#include "injector.h"
#include "log.h"
#include "numpipe.h"
#include "progress.h"
#include "splitter.h"
#include "timebend.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define PROG      "test-timebend"
#define FRAMEINFO "frameinfo.Y"

#define MIN_RATE     1
#define MAX_RATE     100

#define SMOOTH_RMS     5
#define PEAK_RMS       25
#define SMOOTH_RATE    25

#define RMS_BASE      0.5
#define RMS_POWER     1.5

typedef struct {
  double rate;
  double rate_total;
  unsigned rate_count;

  numpipe *np;

  average *sm_rms;
  average *pk_rms;
  average *sm_rate;

} rate_work;

static double rate_func(double rms) {
  return pow(RMS_BASE / rms, RMS_POWER);
}

static y4m2_frame *catch_analysis(y4m2_frame *frame, void *ctx) {
  rate_work *w = (rate_work *) ctx;

  frameinfo *fi = (frameinfo *) y4m2_find_note(frame, FRAMEINFO);

  average_push(w->pk_rms, average_push(w->sm_rms, fi->rms));

  double rms = average_max(w->pk_rms);
  double raw_rate = rate_func(rms);
  double rate = average_push(w->sm_rate, MAX(MIN_RATE, MIN(raw_rate, MAX_RATE)));

  log_debug("rms %8.3f, raw_rate: %8.3f, rate: %8.3f", rms, raw_rate, rate);
  numpipe_put(w->np, rate);

  return frame;
}

static void work_init(rate_work *w) {
  w->np = numpipe_new_average(1);
  w->sm_rms = average_new(SMOOTH_RMS);
  w->pk_rms = average_new(PEAK_RMS);
  w->sm_rate = average_new(SMOOTH_RATE);
}

static void work_free(rate_work *w) {
  numpipe_free(w->np);
  average_free(w->sm_rms);
  average_free(w->pk_rms);
  average_free(w->sm_rate);
}

static double calc_rate(void *ctx) {
  rate_work *w = (rate_work *) ctx;
  double rate = numpipe_get(w->np);
  log_debug("current rate: %8.3f", rate);
  return rate;
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

static void show_rate(void) {
  for (double rms = 0.0; rms <= 1.0; rms += 0.05) {
    double rate = rate_func(rms);
    log_debug("rms: %5.2f -> rate: %8.3f", rms, rate);
  }
}

int main(void) {
  rate_work w;
  dup_notes dup;

  dup.src = NULL;

  work_init(&w);

  log_info("Starting " PROG);

  show_rate();

  /* process chain */
  y4m2_output *process = y4m2_output_file(stdout);
  process = timebend_filter_cb(process, calc_rate, &w);
  process = frameinfo_grapher(process, FRAMEINFO, "rms", "#f00");

  process = delay_filter(process, (SMOOTH_RATE + SMOOTH_RMS + PEAK_RMS) / 2);
  process = injector_filter(process, put_notes, &dup);

  /* analyse chain */
  y4m2_output *analyse = y4m2_output_null();
  analyse = injector_filter(analyse, get_notes, &dup);
  analyse = injector_filter(analyse, catch_analysis, &w);
  analyse = frameinfo_filter(analyse);
  analyse = delta_filter(analyse);

  /* split */
  y4m2_output *head = splitter_filter(analyse, process, NULL);
  head = progress_filter(head, PROGRESS_RATE);
  y4m2_parse(stdin, head);

  y4m2_release_frame(dup.src);

  work_free(&w);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
