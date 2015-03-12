/* main.c */

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fftw3.h>
#include <jd_pretty.h>
#include <sndfile.h>

#include "soundchip.h"

#define PROG "soundchip"

extern const char *config;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] <file>...\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --config cfg.json     Load config\n"
         );
  exit(1);
}

jd_var *load_string(jd_var *out, FILE *f) {
  char buf[0x10000];
  size_t got;

  jd_set_empty_string(out, 1);
  while (got = fread(buf, 1, sizeof(buf), f), got)
    jd_append_bytes(out, buf, got);
  return out;
}

jd_var *load_json(jd_var *out, FILE *f) {
  jd_var json = JD_INIT;
  jd_from_json(out, load_string(&json, f));
  jd_release(&json);
  return out;
}

jd_var *load_file(jd_var *out, const char *fn) {
  FILE *fl = fopen(fn, "r");
  if (!fl) jd_throw("Can't read %s: %s\n", fn, strerror(errno));
  jd_var *v = load_json(out, fl);
  fclose(fl);
  return v;
}

static void load_config(jd_var *ctx, const char *fn) {
  scope {
    jd_var *cfg = load_file(jd_nv(), fn);
    jd_merge(ctx, cfg, 1);
  }
}

static void parse_options(jd_var *ctx, int *argc, char ***argv) {
  int ch, oidx;

  static struct option opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"config", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "c:h", opts, &oidx), ch != -1) {
    switch (ch) {
    case 'c':
      load_config(ctx, optarg);
      break;
    case 'h':
    default:
      usage();
      break;
    }
  }

  *argc -= optind;
  *argv += optind;

  if (*argc == 0) {
    usage();
  }
}

static void stuff_fft(double *fft, double *src, size_t size, unsigned stride, double *window) {
  unsigned i;

  for (i = 0; i < size; i++) {
    double sample = src[i * stride];
    if (window) sample *= window[i];
    fft[i] = sample;
  }
}

static void process(jd_var *ctx, const char *name) {
  SF_INFO info;
  SNDFILE *in;
  unsigned i, j, c;
  size_t got;

  if (in = sf_open(name, SFM_READ, &info), !in)
    jd_throw("Can't read %s: %s", name, sf_strerror(NULL));

  unsigned vscale = (unsigned) jd_get_int(jd_get_ks(ctx, "vscale", 0));
  unsigned maxwidth = (unsigned) jd_get_int(jd_get_ks(ctx, "maxwidth", 0));
  unsigned window = (unsigned) jd_get_int(jd_get_ks(ctx, "window", 0));
  size_t stride = (size_t) jd_get_int(jd_get_ks(ctx, "stride", 0));
  if (stride == 0) jd_throw("Stride not set / zero");

  unsigned width = (info.frames + stride - 1) / stride;
  unsigned height = stride / vscale;
  /*  printf("%s: %dx%d\n", name, width, height);*/

  double *ibuf = fftw_malloc(sizeof(double) * stride * 2);
  if (!ibuf) goto oom;
  double *obuf = fftw_malloc(sizeof(double) * stride * 2);
  if (!obuf) goto oom;

  fftw_plan plan = fftw_plan_r2r_1d(stride * 2, ibuf, obuf, FFTW_R2HC, 0);
  if (!plan) jd_throw("Can't create fftw3 plan");

  double *abuf[2];
  unsigned ab = 0;
  for (i = 0; i < 2 ; i++) {
    abuf[i] = malloc(sizeof(double) * info.channels * stride);
    if (!abuf[i]) goto oom;
  }

  double *hamming = NULL;
  if (window) {
    hamming = malloc(sizeof(double) * stride * 2);
    if (!hamming) goto oom;
    for (i = 0; i < stride * 2; i++) {
      hamming[i] = 0.5 * (1 - cos(2 * M_PI * i / (stride * 2 - 1)));
    }
  }

  while (1) {
    unsigned cb = ab++ & 1;

    got = sf_readf_double(in, abuf[cb], stride);
    memset(abuf[cb] + got, 0, sizeof(double) * (stride - got));
    if (got == 0) break;
    if (ab < 2) continue;

    for (c = 0; c < info.channels; c++) {
      stuff_fft(ibuf, abuf[cb ^ 1] + c, stride, info.channels, hamming);
      stuff_fft(ibuf + stride, abuf[cb] + c, stride, info.channels, hamming ? hamming + stride : NULL);
      fftw_execute(plan);
      printf("%d", c);
      for (i = 1; i < (stride * 2 + 1) / 2; i += vscale) {
        double sum = 0;
        for (j = 0; j < vscale; j++) {
          double sr = obuf[i + j];
          double si = obuf[stride * 2 - i - j];
          sum += sqrt(sr * sr + si * si);
        }
        printf(" %g", sum / vscale);
      }
      printf("\n");
    }

  }

  fftw_destroy_plan(plan);
  fftw_free(ibuf);
  fftw_free(obuf);
  for (i = 0; i < 2; i++)
    free(abuf[i]);
  free(hamming);
  sf_close(in);
  return;

oom:
  jd_throw("Out of memory");
}

static void init_context(jd_var *ctx) {
  jd_from_jsons(ctx, config);
}

int main(int argc, char *argv[]) {
  scope {
    jd_var *ctx = jd_nhv(10);
    init_context(ctx);
    parse_options(ctx, &argc, &argv);
    for (int i = 0; i < argc; i++) {
      process(ctx, argv[i]);
    }
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
