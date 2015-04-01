/* downtown-sig.c */

#include <getopt.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fftw3.h>

#include "centre.h"
#include "delta.h"
#include "downtown.h"
#include "histogram.h"
#include "log.h"
#include "merge.h"
#include "progress.h"
#include "resample.h"
#include "sampler.h"
#include "signature.h"
#include "util.h"
#include "voronoi.h"
#include "yuv4mpeg2.h"
#include "zigzag.h"

#define PROG      "downtown-sig"

typedef struct {
  fftw_plan plan;
  sampler_context *sampler;
  double *ibuf, *obuf;
  size_t len;

  double *raw_sig;;
  int rs_size;

  double *display_sig;
  int vpos;
  int vsize;
  colour_bytes col;

} fft_context;

typedef struct {
  unsigned long frame_count;
  y4m2_output *next;
  fft_context plane_info[Y4M2_N_PLANE];
  FILE *fo;
} context;

static const char *plane_name[] = { "Y", "Cb", "Cr" };

static char *cfg_sampler = "spiral";
static int cfg_histogram = 0;
static int cfg_centre = 0;
static int cfg_delta = 0;
static int cfg_merge = 1;
static char *cfg_output = NULL;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] < <in.y4m2> > <out.y4m2>\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --centre              Centre frames\n"
          "  -d, --delta               Work on diff between frames\n"
          "  -H, --histogram           Histogram equalisation\n"
          "  -M, --merge <n>           Merge every <n> input frames\n"
          "  -o, --output <file>       FFT output file\n"
          "  -q, --quiet               No log output\n"
          "  -S, --sampler <algo>      Select sampler algorithm\n"
          "\n"
         );
  exit(1);
}

static void free_fft_context(fft_context *c) {
  if (c) {
    fftw_destroy_plan(c->plan);
    fftw_free(c->ibuf);
    fftw_free(c->obuf);
    fftw_free(c->raw_sig);
    fftw_free(c->display_sig);
    sampler_free(c->sampler);
  }
}

static void free_context(context *c) {
  if (c->fo) fclose(c->fo);
  for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
    free_fft_context(&c->plane_info[pl]);
  }
}

static void process_fft(fft_context *c) {
  int size = (c->len + 1) / 2 - 1;

  if (!c->raw_sig) {
    c->rs_size = size;
    c->raw_sig = fftw_malloc(sizeof(double) * c->rs_size);
  }

  double *out = c->raw_sig;
  for (int i = 1; i <= size; i++) {
    double sr = c->obuf[i];
    double si = c->obuf[c->len - i];
    *out++ = sqrt(sr * sr + si * si);
  }
}

static void create_sampler(fft_context *fc, const char *spec, const char *name, int w, int h) {
  fc->sampler = sampler_new(spec, name);
  fc->len = sampler_init(fc->sampler, w, h);
  log_debug("Sampler for %s will return %u samples", name, fc->len);
}

static void init_fft_context(fft_context *c) {

  c->ibuf = fftw_malloc(sizeof(double) * c->len);
  if (!c->ibuf) goto oom;

  c->obuf = fftw_malloc(sizeof(double) * c->len);
  if (!c->obuf) goto oom;

  c->plan = fftw_plan_r2r_1d(c->len, c->ibuf, c->obuf, FFTW_R2HC, 0);
  if (!c->plan) die("Can't create FFTW_R2HC plan");

  return;

oom:
  die("Out of memory");
}

static void write_log(context *c, FILE *fl, const y4m2_frame *frame) {
  fft_context *fc = &c->plane_info[Y4M2_Y_PLANE];
  char sig[sig_SIGNATURE_BITS + 1];
  sig_signature(sig, fc->raw_sig, fc->rs_size);
  fprintf(fl, "%14llu %s\n", (unsigned long long) frame->sequence, sig);
}

static void process_frame(context *c, const y4m2_frame *frame) {
  int pl;

  for (pl = Y4M2_Y_PLANE; pl == Y4M2_Y_PLANE; pl++) {
    fft_context *fc = &c->plane_info[pl];

    int w = frame->i.width / frame->i.plane[pl].xs;
    int h = frame->i.height / frame->i.plane[pl].ys;

    if (!fc->sampler) create_sampler(fc, cfg_sampler, plane_name[pl], w, h);

    double *sam = sampler_sample(fc->sampler, frame->plane[pl]);

    if (!fc->plan) init_fft_context(fc);

    memcpy(fc->ibuf, sam, fc->len);
    fftw_execute(fc->plan);
    process_fft(fc);
  }

  if (c->fo) write_log(c, c->fo, frame);

  c->frame_count++;
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
    process_frame(c, frame);
    y4m2_emit_frame(c->next, parms, frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    free_context(c);
    break;
  }
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
    {"output", required_argument, NULL, 'o'},
    {"quiet", no_argument, NULL, 'q'},
    {"sampler", required_argument, NULL, 'S'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "S:M:o:cdhHq", opts, &oidx), ch != -1) {
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

    case 'o':
      cfg_output = optarg;
      break;

    case 'q':
      log_level = ERROR;
      break;

    case 'S':
      cfg_sampler = optarg;
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
  context ctx;

  downtown_init();

  parse_options(&argc, &argv);
  if (argc != 0) usage();

  log_info("Starting " PROG);

  memset(&ctx, 0, sizeof(ctx));

  ctx.fo = stdout;
  if (cfg_output) {
    ctx.fo = fopen(cfg_output, "w");
    if (!ctx.fo) die("Can't write %s: %s", cfg_output, strerror(errno));
  }

  ctx.next = y4m2_output_null();

  y4m2_output *out = y4m2_output_next(callback, &ctx);

  if (cfg_centre) out = centre_filter(out);
  if (cfg_delta) out = delta_filter(out);
  if (cfg_histogram) out = histogram_filter(out);
  if (cfg_merge > 1) out = merge_filter(out, cfg_merge);

  /*  out = frameinfo_filter(out);*/
  out = progress_filter(out, PROGRESS_RATE);

  y4m2_parse(stdin, out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
