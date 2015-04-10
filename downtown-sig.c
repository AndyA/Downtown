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
#include "profile.h"
#include "resample.h"
#include "sampler.h"
#include "util.h"
#include "voronoi.h"
#include "yuv4mpeg2.h"
#include "zigzag.h"

#define PROG      "downtown-sig"
#define SAMPLER   "spiral"

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
  FILE *fh_sig;
  FILE *fh_raw;

  profile *prof;
} context;

static char *cfg_sampler = NULL;
static int cfg_histogram = 0;
static int cfg_centre = 0;
static int cfg_delta = 0;
static int cfg_merge = 1;
static char *cfg_input = "-";
static char *cfg_output = NULL;
static char *cfg_raw = NULL;
static char *cfg_profile = NULL;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] < <in.y4m2> > <out.y4m2>\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --centre              Centre frames\n"
          "  -d, --delta               Work on diff between frames\n"
          "  -H, --histogram           Histogram equalisation\n"
          "  -i, --input <file.yuv>    Input file (default stdin)\n"
          "  -M, --merge <n>           Merge every <n> input frames\n"
          "  -o, --output <file>       signature output file\n"
          "  -p, --profile <file.json> Use profile\n"
          "  -q, --quiet               No log output\n"
          "  -r, --raw <file>          raw FFT output file\n"
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
  profile_free(c->prof);
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

static jd_var *jd_doubles(jd_var *out, const double *in, size_t len) {
  jd_var *slot = jd_push(jd_set_array(out, len), len);
  for (unsigned i = 0; i < len; i++)
    jd_set_real(&slot[i], in[i]);
  return out;
}

static void write_raw_header(context *c, FILE *fl) {
  scope {
    fft_context *fc = &c->plane_info[Y4M2_Y_PLANE];
    sampler_context *sam = fc->sampler;

    jd_var *rec = jd_nhv(4);
    jd_set_string(jd_get_ks(rec, "sampler", 1), sampler_spec(sam));
    jd_set_int(jd_get_ks(rec, "width", 1), sam->width);
    jd_set_int(jd_get_ks(rec, "height", 1), sam->height);

    jd_fprintf(fl, "%J\n", rec);
  }

}

static void write_raw(context *c, FILE *fl, const y4m2_frame *frame) {
  scope {
    fft_context *fc = &c->plane_info[Y4M2_Y_PLANE];

    jd_var *rec = jd_nhv(4);
    jd_set_int(jd_get_ks(rec, "frame", 1), frame->sequence);
    jd_var *pln = jd_set_array(jd_get_ks(rec, "planes", 1), 1);
    jd_doubles(jd_push(pln, 1), fc->raw_sig, fc->rs_size);
    jd_fprintf(fl, "%J\n", rec);
  }
}

static void write_sig(context *c, FILE *fl, const y4m2_frame *frame) {
  if (!c->prof) die("Can't write a signature without a profile");
  fft_context *fc = &c->plane_info[Y4M2_Y_PLANE];
  char sig[profile_SIGNATURE_BITS + 1];
  profile_signature(c->prof, sig, fc->raw_sig, fc->rs_size);
  fprintf(fl, "%14llu %s\n", (unsigned long long) frame->sequence, sig);
}

static void create_sampler(context *c, fft_context *fc, int w, int h) {
  if (cfg_profile) {
    if (cfg_sampler) die("Can't use --sampler with --profile");
    log_info("Loading profile %s", cfg_profile);
    c->prof = profile_load(cfg_profile);
    fc->sampler = profile_sampler(c->prof, &fc->len);
  }
  else {
    const char *spec = cfg_sampler ? cfg_sampler : SAMPLER;
    log_info("Creating sampler %s", spec);
    fc->sampler = sampler_new(spec, "sampler");
    fc->len = sampler_init(fc->sampler, w, h);
  }
}

static void process_frame(context *c, const y4m2_frame *frame) {
  int pl;

  for (pl = Y4M2_Y_PLANE; pl == Y4M2_Y_PLANE; pl++) {
    fft_context *fc = &c->plane_info[pl];

    int w = frame->i.width / frame->i.plane[pl].xs;
    int h = frame->i.height / frame->i.plane[pl].ys;

    /* TODO - make profile sampler if available */
    if (!fc->sampler) {
      create_sampler(c, fc, w, h);
      if (pl == Y4M2_Y_PLANE && c->fh_raw)
        write_raw_header(c, c->fh_raw);
    }

    double *sam = sampler_sample(fc->sampler, frame->plane[pl]);

    if (!fc->plan) init_fft_context(fc);

    memcpy(fc->ibuf, sam, fc->len);
    fftw_execute(fc->plan);
    process_fft(fc);
  }

  if (c->fh_sig) write_sig(c, c->fh_sig, frame);
  if (c->fh_raw) write_raw(c, c->fh_raw, frame);

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
    {"input", required_argument, NULL, 'i'},
    {"histogram", no_argument, NULL, 'H'},
    {"merge", required_argument, NULL, 'M'},
    {"profile", required_argument, NULL, 'p'},
    {"output", required_argument, NULL, 'o'},
    {"quiet", no_argument, NULL, 'q'},
    {"raw", required_argument, NULL, 'r'},
    {"sampler", required_argument, NULL, 'S'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "S:M:i:o:r:cdhHq", opts, &oidx), ch != -1) {
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

    case 'i':
      cfg_input = optarg;
      break;

    case 'M':
      cfg_merge = (int) parse_double(optarg);
      break;

    case 'o':
      cfg_output = optarg;
      break;

    case 'p':
      cfg_profile = optarg;
      break;

    case 'q':
      log_level = ERROR;
      break;

    case 'r':
      cfg_raw = optarg;
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

static FILE *openout(const char *filename) {
  if (!filename) return NULL;
  if (!strcmp(filename, "-")) return stdout;
  FILE *fl = fopen(filename, "w");
  if (!fl) die("Can't write %s: %s", filename, strerror(errno));
  return fl;
}

static FILE *openin(const char *filename) {
  if (!filename) return NULL;
  if (!strcmp(filename, "-")) return stdin;
  FILE *fl = fopen(filename, "r");
  if (!fl) die("Can't read %s: %s", filename, strerror(errno));
  return fl;
}

static void closeio(FILE *fl) {
  if (fl && fl != stdin && fl != stdout && fl != stderr) fclose(fl);
}

int main(int argc, char *argv[]) {
  context ctx;

  downtown_init();

  parse_options(&argc, &argv);
  if (argc != 0) usage();

  log_info("Starting " PROG);

  memset(&ctx, 0, sizeof(ctx));

  ctx.fh_sig = openout(cfg_output);
  ctx.fh_raw = openout(cfg_raw);
  FILE *inh = openin(cfg_input);

  ctx.next = y4m2_output_null();

  y4m2_output *out = y4m2_output_next(callback, &ctx);

  if (cfg_centre) out = centre_filter(out);
  if (cfg_delta) out = delta_filter(out);
  if (cfg_histogram) out = histogram_filter(out);
  if (cfg_merge > 1) out = merge_filter(out, cfg_merge);

  /*  out = frameinfo_filter(out);*/
  out = progress_filter(out, PROGRESS_RATE);

  y4m2_parse(inh, out);

  closeio(ctx.fh_sig);
  closeio(ctx.fh_raw);
  closeio(inh);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
