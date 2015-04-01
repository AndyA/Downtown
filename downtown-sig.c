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
#include "dumpframe.h"
#include "frameinfo.h"
#include "histogram.h"
#include "log.h"
#include "merge.h"
#include "progress.h"
#include "resample.h"
#include "sampler.h"
#include "util.h"
#include "voronoi.h"
#include "yuv4mpeg2.h"
#include "zigzag.h"

#define PROG      "downtown-sig"
#define OUTWIDTH  1920
#define OUTHEIGHT 1080
#define MARGIN    32
#define GUTTER    16

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
  double min, max, total;
  unsigned long count;
} range_stats;

typedef struct {
  unsigned long frame_count;
  y4m2_output *next;
  y4m2_frame *out_buf;
  y4m2_parameters *out_parms;
  fft_context plane_info[Y4M2_N_PLANE];
  range_stats stats[Y4M2_N_PLANE];
  FILE *fo;
} context;

typedef struct string_list {
  struct string_list *next;
  char *v;
} string_list;

static const char *plane_name[] = { "Y", "Cb", "Cr" };

static double cfg_gain = 1.0;
static char *cfg_sampler = "spiral";
static int cfg_histogram = 0;
static int cfg_mono = 0;
static int cfg_centre = 0;
static int cfg_auto = 0;
static int cfg_delta = 0;
static int cfg_width = OUTWIDTH;
static int cfg_height = OUTHEIGHT;
static int cfg_merge = 1;
static string_list *cfg_graph = NULL;
static char *cfg_output = NULL;

#define MAX_PLANE (cfg_mono ? Y4M2_Y_PLANE + 1 : Y4M2_N_PLANE)

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] < <in.y4m2> > <out.y4m2>\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --centre              Centre frames\n"
          "  -d, --delta               Work on diff between frames\n"
          "  -g, --gain <gain>         Signal gain\n"
          "  -G, --graph <field>       Graph field\n"
          "  -H, --histogram           Histogram equalisation\n"
          "  -m, --mono                Only process luma\n"
          "  -M, --merge <n>           Merge every <n> input frames\n"
          "  -o, --output <file>       FFT output file\n"
          "  -q, --quiet               No log output\n"
          "  -s, --size <w>x<h>        Output size\n"
          "  -S, --sampler <algo>      Select sampler algorithm\n"
          "\n"
         );
  exit(1);
}

static string_list *sl_put(string_list *sl, const char *s) {
  if (sl) {
    sl->next = sl_put(sl->next, s);
    return sl;
  }

  sl = alloc(sizeof(string_list));
  sl->v = sstrdup(s);
  return sl;
}

static void sl_free(string_list *sl) {
  if (sl) {
    sl_free(sl->next);
    free(sl->v);
    free(sl);
  }
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
  if (c->out_buf) y4m2_release_frame(c->out_buf);
  y4m2_free_parms(c->out_parms);
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

static void put_stats(range_stats *st, double min, double max, double avg) {
  if (st->count++ == 0) {
    st->min = min;
    st->max = max;
    st->total = avg;
  }
  else {
    if (min < st->min) st->min = min;
    if (max > st->max) st->max = max;
    st->total += avg;
  }
}

static void scroll_left(uint8_t *buf, int w, int h, uint8_t fill) {
  for (int y = 0; y < h; y++) {
    memmove(buf + y * w, buf + y * w + 1, w - 1);
    buf[y * w + w - 1] = fill;
  }
}

static void layout_display(context *c, const y4m2_frame *frame) {
  double avail = frame->i.height - MARGIN * 2 - GUTTER * MAX_PLANE - 1;
  double lanes = 0;
  unsigned pl;


  static const char *lane_colour[] = { "#fff", "#99f", "#f99" };

  for (pl = 0; pl < Y4M2_N_PLANE; pl++) {
    colour_bytes rgb;
    colour_parse_rgb(&rgb, lane_colour[pl]);
    colour_b_rgb2yuv(&rgb, &c->plane_info[pl].col);
  }

  for (pl = 0; pl < MAX_PLANE; pl++)
    lanes += 1.0 / (double)(frame->i.plane[pl].xs * frame->i.plane[pl].ys);

  double lane_width = avail / lanes;
  double ypos = MARGIN;

  for (pl = 0; pl < MAX_PLANE; pl++) {
    fft_context *fc = &c->plane_info[pl];
    fc->vpos = (int)(ypos + 0.5);
    double vsize = lane_width / (frame->i.plane[pl].xs * frame->i.plane[pl].ys);
    fc->vsize = (int) vsize;
    log_debug("lane for %s is at %d, height %d", plane_name[pl], fc->vpos, fc->vsize);
    ypos += GUTTER + vsize;
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
  fprintf(fl, "{\"frame\":%llu,\"planes\":[", (unsigned long long) frame->sequence);
  for (unsigned pl = 0; pl < MAX_PLANE; pl++) {
    fft_context *fc = &c->plane_info[pl];
    fprintf(fl, "%s[", pl ? ", " : "");
    for (unsigned i = 0; i < (unsigned) fc->rs_size; i++)
      fprintf(fl, "%s%g", i ? ", " : "", fc->raw_sig[i]);
    fprintf(fl, "]");
  }
  fprintf(fl, "]}\n");
}

static void plot_frame_sig(fft_context *c, y4m2_frame *frame, range_stats *st) {
  double min, max, avg;

  double *sb = c->display_sig;

  min_max(sb, c->vsize, &min, &max, &avg);
  put_stats(st, min, max, avg);

  if (!cfg_auto) {
    min = 0;
    max = 500;
  }

  int omin = 16;
  int omax = 240;

  for (int i = 0; i < c->vsize; i++) {
    double dv = cfg_gain * (sb[i] - min) / (max - min);
    int iv = dv * (omax - omin) + omin;
    y4m2_draw_point(frame, frame->i.width - 1, frame->i.height - 1 - c->vpos - i,
                    MIN(MAX(omin, iv), omax), c->col.c[cCb], c->col.c[cCr]);
  }
}

static void process_frame(context *c, const y4m2_frame *frame) {
  int pl;

  if (!c->out_buf) {
    c->out_buf = y4m2_new_frame(c->out_parms);
    layout_display(c, c->out_buf);
  }

  y4m2_frame *ofr = c->out_buf;

  for (pl = 0; pl < Y4M2_N_PLANE; pl++) {
    if (c->frame_count & (ofr->i.plane[pl].xs - 1)) continue;
    int ow = ofr->i.width / ofr->i.plane[pl].xs;
    int oh = ofr->i.height / ofr->i.plane[pl].ys;
    scroll_left(ofr->plane[pl], ow, oh, ofr->i.plane[pl].fill);
  }

  for (pl = 0; pl < MAX_PLANE; pl++) {
    if (c->frame_count & (frame->i.plane[pl].xs - 1)) continue;
    fft_context *fc = &c->plane_info[pl];

    int w = frame->i.width / frame->i.plane[pl].xs;
    int h = frame->i.height / frame->i.plane[pl].ys;

    if (!fc->sampler) create_sampler(fc, cfg_sampler, plane_name[pl], w, h);

    double *sam = sampler_sample(fc->sampler, frame->plane[pl]);

    if (!fc->plan) init_fft_context(fc);

    memcpy(fc->ibuf, sam, fc->len);
    fftw_execute(fc->plan);
    process_fft(fc);

    if (!fc->display_sig) fc->display_sig = fftw_malloc(sizeof(double) * fc->vsize);
    resample_double(fc->display_sig, fc->vsize, fc->raw_sig, fc->rs_size);

    plot_frame_sig(fc, ofr, &c->stats[pl]);
  }

  if (c->fo) write_log(c, c->fo, frame);

  c->frame_count++;
}


static void dump_stats(context *c) {
  for (int pl = 0; pl < MAX_PLANE; pl++) {
    range_stats *st = &c->stats[pl];
    log_info("stats for %s: (min=%.3f, avg=%.3f, max=%.3f)",
             plane_name[pl], st->min, st->total / st->count, st->max);
  }
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  context *c = ctx;

  switch (reason) {

  case Y4M2_START:
    c->out_parms = y4m2_adjust_parms(parms, "W%d H%d A1:1 C444", cfg_width, cfg_height);
    y4m2_emit_start(c->next, c->out_parms);
    break;

  case Y4M2_FRAME:
    process_frame(c, frame);
    y4m2_copy_notes(c->out_buf, frame);
    y4m2_emit_frame(c->next, c->out_parms, y4m2_retain_frame(c->out_buf));
    y4m2_release_frame(frame);
    break;

  case Y4M2_END:
    y4m2_emit_end(c->next);
    dump_stats(c);
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
    {"centre", no_argument, NULL, 'c'},
    {"center", no_argument, NULL, 'c'},
    {"delta", no_argument, NULL, 'd'},
    {"graph", required_argument, NULL, 'G'},
    {"gain", required_argument, NULL, 'g'},
    {"auto", no_argument, NULL, 'a'},
    {"histogram", no_argument, NULL, 'H'},
    {"mono", no_argument, NULL, 'm'},
    {"merge", required_argument, NULL, 'M'},
    {"output", required_argument, NULL, 'o'},
    {"quiet", no_argument, NULL, 'q'},
    {"sampler", required_argument, NULL, 'S'},
    {"size", required_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "g:G:s:S:M:o:acdmhHq", opts, &oidx), ch != -1) {
    switch (ch) {

    case 'a':
      cfg_auto = 1;
      break;

    case 'c':
      cfg_centre = 1;
      break;

    case 'd':
      cfg_delta = 1;
      break;

    case 'G':
      cfg_graph = sl_put(cfg_graph, optarg);
      break;

    case 'g':
      cfg_gain = parse_double(optarg);
      break;

    case 'H':
      cfg_histogram = 1;
      break;

    case 'm':
      cfg_mono = 1;
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

static void register_samplers() {
  zigzag_register();
  voronoi_register();
}

static y4m2_output *add_graph(y4m2_output *out, const char *spec) {
  char *sbuf = sstrdup(spec);
  char *c1 = strchr(sbuf, ':');
  char *c2 = c1 ? strchr(c1 + 1, ':') : NULL;
  if (c1) *c1 = '\0';
  if (c2) *c2 = '\0';
  const char *note = c2 ? sbuf : "frameinfo.Y";
  const char *field = c2 ? c1 + 1 : sbuf;
  const char *colour = c1 ? (c2 ? c2 + 1 : c1 + 1) : "#f00";
  log_debug("Adding graph: %s:%s:%s", note, field, colour);

  out = frameinfo_grapher(out, note, field, colour);
  free(sbuf);
  return out;
}

int main(int argc, char *argv[]) {
  context ctx;

  register_samplers();

  parse_options(&argc, &argv);
  if (argc != 0) usage();

  log_info("Starting " PROG);

  memset(&ctx, 0, sizeof(ctx));

  if (cfg_output) {
    ctx.fo = fopen(cfg_output, "w");
    if (!ctx.fo) die("Can't write %s: %s", cfg_output, strerror(errno));
  }

  ctx.next = y4m2_output_file(stdout);

  for (string_list *sl = cfg_graph; sl; sl = sl->next)
    ctx.next = add_graph(ctx.next, sl->v);

  y4m2_output *out = y4m2_output_next(callback, &ctx);

  if (cfg_centre) out = centre_filter(out);
  if (cfg_delta) out = delta_filter(out);
  if (cfg_histogram) out = histogram_filter(out);
  if (cfg_merge > 1) out = merge_filter(out, cfg_merge);

  out = frameinfo_filter(out);
  out = progress_filter(out, PROGRESS_RATE);
  /*  out = dumpframe_filter(out, "dump/fr%08d.png", 25);*/

  y4m2_parse(stdin, out);
  /*  y4m2_free_output(ctx.next);*/
  sl_free(cfg_graph);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
