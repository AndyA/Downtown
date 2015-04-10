/* downtown-filter.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "centre.h"
#include "delta.h"
#include "downtown.h"
#include "frameinfo.h"
#include "histogram.h"
#include "log.h"
#include "merge.h"
#include "progress.h"
#include "scale.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define PROG      "downtown-filter"

static int cfg_centre = 0;
static int cfg_delta = 0;
static int cfg_histogram = 0;
static int cfg_merge = 1;
static char *cfg_size = NULL;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] < <in.y4m2> > <out.y4m2>\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --centre              Centre frames\n"
          "  -d, --delta               Work on diff between frames\n"
          "  -H, --histogram           Histogram equalisation\n"
          "  -M, --merge <n>           Merge every <n> input frames\n"
          "  -q, --quiet               No log output\n"
          "  -s, --size <w>x<h>        Scale frames\n"
          "\n"
         );
  exit(1);
}

static void parse_size(const char *size, unsigned *wp, unsigned *hp) {
  const char *sp;
  char *ep;

  unsigned w = strtoul(size, &ep, 10);
  if (ep == size || (*ep != ':' && *ep != 'x')) goto bad;

  sp = ep + 1;

  unsigned h = strtoul(sp, &ep, 10);
  if (ep == sp || *ep) goto bad;

  *wp = w;
  *hp = h;

  return;

bad:
  die("Bad size: %s", size);
}

static double parse_double(const char *num) {
  char *ep;
  double v = strtod(num, &ep);
  if (ep == num || *ep) die("Bad number: %s", num);
  return v;
}

static void parse_options(int *argc, char ***argv) {
  int ch, oidx;

  static struct option opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"centre", no_argument, NULL, 'c'},
    {"center", no_argument, NULL, 'c'},
    {"delta", no_argument, NULL, 'd'},
    {"histogram", no_argument, NULL, 'H'},
    {"merge", required_argument, NULL, 'M'},
    {"quiet", no_argument, NULL, 'q'},
    {"size", required_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "M:s:hHcdq", opts, &oidx), ch != -1) {
    switch (ch) {

    case 'c':
      cfg_centre = 1;
      break;

    case 'd':
      cfg_delta = 1;
      break;

    case 'H':
      cfg_histogram = 1;
      break;

    case 'M':
      cfg_merge = (int) parse_double(optarg);
      break;

    case 'q':
      log_level = ERROR;
      break;

    case 's':
      cfg_size = optarg;
      break;

    case 'h':
    default:
      usage();
      break;

    }
  }

  *argc -= optind;
  *argv += optind;
}

int main(int argc, char *argv[]) {
  parse_options(&argc, &argv);
  if (argc != 0) usage();

  log_info("Starting " PROG);

  y4m2_output *out = y4m2_output_file(stdout);

  if (cfg_centre) out = centre_filter(out);
  if (cfg_delta) out = delta_filter(out);
  if (cfg_histogram) out = histogram_filter(out);
  if (cfg_merge > 1) out = merge_filter(out, cfg_merge);
  if (cfg_size) {
    unsigned w, h;
    parse_size(cfg_size, &w, &h);
    out = scale_filter(out, w, h);
  }

  out = frameinfo_filter(out);
  out = progress_filter(out, PROGRESS_RATE);

  y4m2_parse(stdin, out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
