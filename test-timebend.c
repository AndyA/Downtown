/* test-timebend.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "downtown.h"
#include "frameinfo.h"
#include "injector.h"
#include "log.h"
#include "progress.h"
#include "timebend.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define PROG      "test-timebend"

static double rate = 0.1;

static y4m2_frame *inject_rate(y4m2_frame *frame, void *ctx) {
  (void) ctx;

  if (frame->sequence >= 10 && rate < 10) {
    rate *= 1.01;
    timebend_set_rate(frame, rate);
  }

  return frame;
}

int main(void) {

  log_info("Starting " PROG);

  y4m2_output *out = y4m2_output_file(stdout);

  out = timebend_filter(out, rate);
  out = injector_filter(out, inject_rate, NULL);

  out = frameinfo_filter(out);
  out = progress_filter(out, PROGRESS_RATE);

  y4m2_parse(stdin, out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
