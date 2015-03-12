/* downtown.c */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fftw3.h>

#include "downtown.h"
#include "util.h"
#include "yuv4mpeg2.h"
#include "zigzag.h"

#define PROG      "downtown"
#define OUTWIDTH  1920
#define OUTHEIGHT 1080

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
  fftw_plan plan;
  double *ibuf, *obuf;
  double *work;
  int len;
  int osize;
  int vscale;
  int voffset;
} fft_context;

typedef struct {
  unsigned long frame_count;
  y4m2_output *next;
  y4m2_frame *buf;
  y4m2_frame *out_buf;
  y4m2_parameters *out_parms;
  fft_context fftc[Y4M2_N_PLANE];
} context;

static double cfg_gain = 1.0;
static permute_func cfg_permute = zigzag_permute;
static int cfg_mono = 0;
static int cfg_width = OUTWIDTH;
static int cfg_height = OUTHEIGHT;

static permute_method permuters[] = {
  { .name = "zigzag",  .f = zigzag_permute },
  { .name = "raster",  .f = zigzag_raster },
  { .name = "weave",  .f = zigzag_weave },
};

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] <file>...\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -g, --gain <gain>         Signal gain\n"
          "  -m, --mono                Only process luma\n"
          "  -p, --permute <algo>      Select permution algorithm\n"
          "  -s, --size <w>x<h>        Output size\n"
          "\n"
         );
  exit(1);
}

static y4m2_parameters *parm_adj_size(const y4m2_parameters *parms, unsigned w, unsigned h) {
  y4m2_parameters *np = y4m2_clone_parms(parms);
  y4m2_parameters *delta = y4m2_new_parms();
  char nbuf[30];

  sprintf(nbuf, "%u", w);
  y4m2_set_parm(delta, "W", nbuf);

  sprintf(nbuf, "%u", h);
  y4m2_set_parm(delta, "H", nbuf);

  y4m2_merge_parms(np, delta);
  y4m2_free_parms(delta);

  return np;
}

static void init_fft_context(fft_context *c, int len, int stripe) {
  c->len = len;
  c->ibuf = fftw_malloc(sizeof(double) * len);
  if (!c->ibuf) goto oom;
  c->obuf = fftw_malloc(sizeof(double) * len);
  if (!c->obuf) goto oom;
  c->plan = fftw_plan_r2r_1d(len, c->ibuf, c->obuf, FFTW_R2HC, 0);
  if (!c->plan) die("Can't create FFTW_R2HC plan");

  int olen = len / 2;
  c->vscale = 1;
  while (olen / c->vscale > stripe)
    c->vscale *= 2;

  c->voffset = (stripe - olen / c->vscale) / 2;

  return;

oom:
  die("Out of memory");
}

static void free_fft_context(fft_context *c) {
  fftw_destroy_plan(c->plan);
  fftw_free(c->ibuf);
  fftw_free(c->obuf);
  fftw_free(c->work);
}


static void free_context(context *c) {
  if (c->buf) y4m2_release_frame(c->buf);
  if (c->out_buf) y4m2_release_frame(c->out_buf);
  y4m2_free_parms(c->out_parms);
  y4m2_emit_end(c->next);
  for (int pl = 0; pl < Y4M2_N_PLANE; pl++) {
    free_fft_context(&c->fftc[pl]);
  }
}

static void b2da(double *out, const uint8_t *in, size_t len) {
  for (unsigned i = 0; i < len; i++) {
    out[i] = (double) in[i] / 255;
  }
}

static double *scale_fft(fft_context *c) {
  int size = (c->len + 1) / 2;
  int osize = size / c->vscale;

  if (!c->work) {
    c->osize = osize;
    c->work = fftw_malloc(sizeof(double) * osize);
  }
  double *out = c->work;

  for (int i = 1; i < size; i += c->vscale) {
    double sum = 0;
    for (int j = 0; j < c->vscale; j++) {
      double sr = c->obuf[i + j];
      double si = c->obuf[c->len - i - j];
      sum += sqrt(sr * sr + si * si);
    }
    *out++ = sum / c->vscale;
  }

  return c->work;
}

static void min_max(const double *d, size_t len, double *pmin, double *pmax, double *pavg) {
  double min, max, total = 0;
  for (unsigned i = 0; i < len; i++) {
    if (i) {
      if (d[i] < min) min = d[i];
      if (d[i] > max) max = d[i];
    }
    else {
      min = max = d[i];
    }
    total += d[i];
  }
  if (pmin) *pmin = min;
  if (pmax) *pmax = max;
  if (pavg) *pavg = total / len;
}

static void fft2b(uint8_t *out, int step, fft_context *c, int omin, int omax) {
  double min, max, avg;

  double *sb = scale_fft(c);
  min_max(sb, c->osize, &min, &max, &avg);

  for (int i = 0; i < c->osize; i++) {
    double dv = cfg_gain * (sb[i] - min) / (max - min);
    int iv = dv * (omax - omin) + omin;
    *out = MIN(MAX(omin, iv), omax);
    out += step;
  }
}

static void scroll_left(uint8_t *buf, int w, int h, uint8_t fill) {
  for (int y = 0; y < h; y++) {
    memmove(buf + y * w, buf + y * w + 1, w - 1);
    buf[y * w + w - 1] = fill;
  }
}

static void process_frame(context *c, const y4m2_frame *frame) {

  if (!c->out_buf) {
    c->out_buf = y4m2_new_frame(c->out_parms);
    c->buf = y4m2_like_frame(frame);
  }

  y4m2_frame *ofr = c->out_buf;
  int max_plane = cfg_mono ? Y4M2_Y_PLANE + 1 : Y4M2_N_PLANE;

  for (int pl = 0; pl < max_plane; pl++) {
    if (c->frame_count & (frame->i.plane[pl].xs - 1)) continue;
    fft_context *fc = &c->fftc[pl];
    int w = frame->i.width / frame->i.plane[pl].xs;
    int h = frame->i.height / frame->i.plane[pl].ys;

    int ow = ofr->i.width / ofr->i.plane[pl].xs;
    int oh = ofr->i.height / ofr->i.plane[pl].ys;

    int len = w * h;

    if (!fc->plan) init_fft_context(fc, len, oh);

    cfg_permute(frame->plane[pl], c->buf->plane[pl], w, h);
    b2da(fc->ibuf, c->buf->plane[pl], len);
    fftw_execute(fc->plan);
    scroll_left(ofr->plane[pl], ow, oh, ofr->i.plane[pl].fill);
    fft2b(ofr->plane[pl] + (oh - 1 - fc->voffset) * ow + ow - 1, -ow, fc, 16, 240);
  }
  c->frame_count++;
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  (void) parms;
  (void) frame;
  context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    c->out_parms = parm_adj_size(parms, cfg_width, cfg_height);
    y4m2_emit_start(c->next, c->out_parms);
    break;

  case Y4M2_FRAME:
    process_frame(c, frame);
    y4m2_emit_frame(c->next, c->out_parms, c->out_buf);
    break;

  case Y4M2_END:
    free_context(c);
    y4m2_emit_end(c->next);
    break;

  }
}

static double parse_double(const char *num) {
  char *ep;
  double v = strtod(num, &ep);
  if (ep == num || *ep) die("Bad number: %s", num);
  return v;
}

static permute_func parse_permute(const char *perm) {
  for (unsigned i = 0; i < sizeof(permuters) / sizeof(permuters[0]); i++) {
    if (0 == strcmp(permuters[i].name, perm)) return permuters[i].f;
  }
  die("Bad permuter: %s", perm);
  return NULL;
}

static void parse_size(const char *size, int *wp, int *hp) {
  const char *sp;
  char *ep;

  int w = strtoul(size, &ep, 10);
  if (ep == size || (*ep != ':' && *ep != 'x')) goto bad;

  sp = ep + 1;

  int h = strtoul(sp, &ep, 10);
  if (ep == sp || *ep) goto bad;

  *wp = w;
  *hp = h;

  return;

bad:
  die("Bad size: %s", size);

}

static void parse_options(int *argc, char ***argv) {
  int ch, oidx;

  static struct option opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"gain", required_argument, NULL, 'g'},
    {"mono", no_argument, NULL, 'm'},
    {"permute", required_argument, NULL, 'p'},
    {"size", required_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "g:p:s:h", opts, &oidx), ch != -1) {
    switch (ch) {

    case 'g':
      cfg_gain = parse_double(optarg);
      break;

    case 'm':
      cfg_mono = 1;
      break;

    case 'p':
      cfg_permute = parse_permute(optarg);
      break;

    case 's':
      parse_size(optarg, &cfg_width, &cfg_height);
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

  parse_options(&argc, &argv);
  if (argc != 0) usage();

  memset(&ctx, 0, sizeof(ctx));

  ctx.next = y4m2_output_file(stdout);

  y4m2_output *out = y4m2_output_next(callback, &ctx);
  y4m2_parse(stdin, out);
  y4m2_free_output(ctx.next);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
