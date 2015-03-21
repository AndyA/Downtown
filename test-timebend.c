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

static double angle = 0;

static double calc_rate(void *ctx) {
  (void) ctx;

  double rate = pow(25, sin(angle));
  angle += 0.02;

  return rate;
}

int main(void) {

  log_info("Starting " PROG);

  y4m2_output *out = y4m2_output_file(stdout);

  out = timebend_filter_cb(out, calc_rate, NULL);

  out = frameinfo_filter(out);
  out = progress_filter(out, PROGRESS_RATE);

  y4m2_parse(stdin, out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
