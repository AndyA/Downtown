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

#define MIN_RATE  0.1
#define MAX_RATE  100

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
  if (fi) {
    double rms = MAX(fi->rms, 0.00000001);
    w->raw_rate = MAX(MIN_RATE, MIN(0.1 / rms, MAX_RATE));
  }
  return frame;
}

int main(void) {
  rate_work work;

  init_work(&work);
  work.decay = 0.9;

  log_info("Starting " PROG);

  y4m2_output *out = y4m2_output_file(stdout);
  y4m2_output *analyse = y4m2_output_null();

  /* process chain */
  out = timebend_filter_cb(out, calc_rate, &work);
  /*  out = delay_filter(out, 25);*/

  /* analyse chain */
  analyse = injector_filter(analyse, catch_analysis, &work);
  analyse = frameinfo_filter(analyse);
  analyse = delta_filter(analyse);

  y4m2_output *head = splitter_filter(analyse, out, NULL);

  head = progress_filter(head, PROGRESS_RATE);
  y4m2_parse(stdin, head);


  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
