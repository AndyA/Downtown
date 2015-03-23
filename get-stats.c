/* get-stats.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "delta.h"
#include "downtown.h"
#include "frameinfo.h"
#include "injector.h"
#include "log.h"
#include "progress.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define PROG      "get-stats"
#define FRAMEINFO "frameinfo.Y"

#define PLANE_NAMES \
  X(Y)  \
  X(Cb) \
  X(Cr)

static int cfg_delta = 0;
static int cfg_raw   = 0;

typedef struct {
#define X(p) frameinfo p;
  PLANE_NAMES
#undef X
} frameinfo_set;

static int frame_no;

static frameinfo_set last_raw;
static frameinfo_set last_delta;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] < <in.y4m2> > <out.y4m2>\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -d, --delta               Generate stats for delta frames\n"
          "  -r, --raw                 Generate stats for raw frames\n"
          "\n"
         );
  exit(1);
}

static void parse_options(int *argc, char ***argv) {
  int ch, oidx;

  static struct option opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"delta", no_argument, NULL, 'd'},
    {"raw", no_argument, NULL, 'r'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "dhr", opts, &oidx), ch != -1) {
    switch (ch) {

    case 'd':
      cfg_delta = 1;
      break;

    case 'r':
      cfg_raw = 1;
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

static void get_notes(y4m2_frame *frame, frameinfo_set *set) {
#define X(f) \
  frameinfo *f = (frameinfo *) y4m2_find_note(frame, "frameinfo." #f); \
  if (f) set->f = *f;
  PLANE_NAMES
#undef X
}

static y4m2_frame *catch_raw(y4m2_frame *frame, void *ctx) {
  (void) ctx;
  get_notes(frame, &last_raw);
  return frame;
}

static y4m2_frame *catch_delta(y4m2_frame *frame, void *ctx) {
  (void) ctx;
  get_notes(frame, &last_delta);
  return frame;
}

static void print_names(const char *kind, const char *plane) {
  printf(
#define X(f) ",%s_%s_%s"
    FRAMEINFO_FIELDS
#undef X
#define X(f) , kind, plane, #f
    FRAMEINFO_FIELDS
#undef X
  );
}

static void print_set_header(const char *kind) {
#define X(p) print_names(kind, #p);
  PLANE_NAMES
#undef X
}

static void print_values(const frameinfo *fi) {
  printf(
#define X(f) ",%g"
    FRAMEINFO_FIELDS
#undef X
#define X(f) , fi->f
    FRAMEINFO_FIELDS
#undef X
  );
}

static void print_set(const frameinfo_set *set) {
#define X(p) print_values(&set->p);
  PLANE_NAMES
#undef X
}

static void print_header(void) {
  printf("frame");
  if (cfg_raw) print_set_header("raw");
  if (cfg_delta) print_set_header("delta");
  printf("\n");
}

static y4m2_frame *print_info(y4m2_frame *frame, void *ctx) {
  (void) ctx;

  printf("%d", frame_no++);
  if (cfg_raw) print_set(&last_raw);
  if (cfg_delta) print_set(&last_delta);
  printf("\n");

  return frame;
}

int main(int argc, char *argv[]) {
  parse_options(&argc, &argv);
  if (argc != 0) usage();

  log_info("Starting " PROG);

  print_header();

  y4m2_output *out = y4m2_output_null();
  out = injector_filter(out, print_info, NULL);

  if (cfg_delta) {
    out = injector_filter(out, catch_delta, NULL);
    out = frameinfo_filter(out);
    out = delta_filter(out);
  }

  if (cfg_raw) {
    out = injector_filter(out, catch_raw, NULL);
    out = frameinfo_filter(out);
  }

  out = progress_filter(out, PROGRESS_RATE);

  y4m2_parse(stdin, out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
