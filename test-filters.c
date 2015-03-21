/* test-filters.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "centre.h"
#include "delay.h"
#include "delta.h"
#include "dumpframe.h"
#include "frameinfo.h"
#include "histogram.h"
#include "injector.h"
#include "merge.h"
#include "progress.h"
#include "splitter.h"
#include "timebend.h"

#include "downtown.h"
#include "log.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define PROG      "test-filters"
#define FRAMEINFO "frameinfo.Y"

int main(void) {

  log_info("Starting " PROG);

  y4m2_output *out = y4m2_output_file(stdout);


  
  out = progress_filter(out, PROGRESS_RATE);
  y4m2_parse(stdin, out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
