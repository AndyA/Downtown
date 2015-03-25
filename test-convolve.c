/* test-convolve.c */

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "numlist.h"
#include "tb_convolve.h"
#include "util.h"

#define PROG      "test-convolve"

#define NOWT        0.000000001
#define CLIP      500

typedef struct data_series {
  struct data_series *next;
  char *name;
  double *data;
  size_t len;
  tb_convolve *c;
} data_series;

static char *cfg_output = NULL;
static double cfg_count = 10;
static double cfg_prescale = 1;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] -o <output.dat> <data.dat> <kernel.dat>...\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --count    <n>        Number of iterations (default: 10)\n"
          "  -p, --prescale <n>        Prescale data\n"
          "  -o, --output   <file>     Set output file pattern. Include %%d or similar\n"
          "\n"
         );
  exit(1);
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
    {"count", required_argument, NULL, 'c'},
    {"output", required_argument, NULL, 'o'},
    {"prescale", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "ho:c:p:", opts, &oidx), ch != -1) {
    switch (ch) {

    case 'c':
      cfg_count = parse_double(optarg);
      break;

    case 'o':
      cfg_output = optarg;
      break;

    case 'p':
      cfg_prescale = parse_double(optarg);
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

static numlist *read_numlist(FILE *in, int end) {
  numlist *nl = NULL;
  int c = fgetc(in);
  for (;;) {
    double r;
    while (c != EOF && isspace(c)) c = fgetc(in);
    if (c == end) return nl;
    if (c == EOF) die("Unexpected EOF");
    ungetc(c, in);
    int rc = fscanf(in, "%lf", &r);
    if (rc == EOF && errno) die("Read error: %s", strerror(errno));
    if (rc < 1) die("Bad syntax");
    nl = numlist_putn(nl, r);
    c = fgetc(in);
  }
}

static tb_convolve *read_convolver(FILE *in) {
  int c = fgetc(in);

  while (c != EOF && isspace(c)) c = fgetc(in);
  if (c == '[') {
    numlist *pos = read_numlist(in, ']');
    c = fgetc(in);
    while (c != EOF && isspace(c)) c = fgetc(in);
    if (c != '[') die("Missing negative coefs");
    numlist *neg = read_numlist(in, ']');
    unsigned psize = (unsigned) numlist_size(pos);
    unsigned nsize = (unsigned) numlist_size(neg);
    if (psize != nsize)
      die("Coeficents aren't the same size: pos=%u, neg=%u", psize, nsize);

    return tb_convolve_new_signed(psize, numlist_drain(pos, NULL), numlist_drain(neg, NULL));
  }

  ungetc(c, in);
  numlist *nl = read_numlist(in, EOF);
  size_t size;
  double *coef = numlist_drain(nl, &size);

  return tb_convolve_new((unsigned) size, coef);
}

static double *read_numbers(FILE *fl, size_t *sizep) {
  numlist *nl = NULL;
  for (;;) {
    double r;
    int rc = fscanf(fl, "%lf", &r);
    if (rc == EOF) {
      if (errno) die("Read error: %s", strerror(errno));
      break;
    }
    if (rc < 1) die("Bad syntax");
    nl = numlist_putn(nl, r);
  }
  return numlist_drain(nl, sizep);
}

static double *read_numfile(const char *name, size_t *sizep) {
  FILE *fl = fopen(name, "r");
  if (!fl) die("Can't read %s: %s", name, strerror(errno));
  double *d = read_numbers(fl, sizep);
  fclose(fl);
  return d;
}


static tb_convolve *read_convfile(const char *name) {
  FILE *fl = fopen(name, "r");
  if (!fl) die("Can't read %s: %s", name, strerror(errno));
  tb_convolve *c = read_convolver(fl);
  fclose(fl);
  return c;
}

static data_series *put_ds(data_series *ds, const char *name, double *data, size_t len, tb_convolve *c) {
  if (!ds) {
    ds = alloc(sizeof(data_series));
    ds->name = sstrdup(name);
    ds->data = data;
    ds->len = len;
    ds->c = c;
    return ds;
  }
  ds->next = put_ds(ds->next, name, data, len, c);
  return ds;
}

static data_series *put_series(data_series *ds, const char *name, double *data, size_t len) {
  return put_ds(ds, name, data, len, NULL);
}

static data_series *put_convolve(data_series *ds, const char *name, tb_convolve *c) {
  return put_ds(ds, name, 0, 0, c);
}

static void dump_data(const char *name, double *data, size_t len) {
  mkparents(name, 0777);
  FILE *fl = fopen(name, "w");
  if (!fl) die("Can't write %s: %s", name, strerror(errno));
  for (unsigned i = 0; i < len; i++) fprintf(fl, "%f\n", data[i]);
  fclose(fl);
}

static void free_series(data_series *ds) {
  if (ds) {
    free(ds->name);
    free_series(ds->next);
    tb_convolve_free(ds->c);
    free(ds->data);
    free(ds);
  }
}

static void convolve(data_series *data, data_series *kernel) {
  if (!kernel->c) {
    log_debug("  Creating tb_convolve for %s", kernel->name);
    kernel->c = tb_convolve_new(kernel->len, kernel->data);
  }

  double tmp[data->len];
  log_debug("  Applying %s to %s", kernel->name, data->name);
  tb_convolve_set_prescale(kernel->c, cfg_prescale);
  tb_convolve_apply(kernel->c, tmp, data->data, data->len);
  for (unsigned i = 0; i < data->len; i++)
    data->data[i] = MIN(MAX(NOWT, tmp[i]), CLIP);
}

int main(int argc, char *argv[]) {
  log_info("Starting " PROG);

  parse_options(&argc, &argv);
  if (argc < 2 || !cfg_output) usage();

  size_t size;
  double *data = read_numfile(argv[0], &size);
  data_series *ds = put_series(NULL, argv[0], data, size);
  log_debug("Read %llu data from %s", (unsigned long long) size, argv[0]);

  for (int i = 1; i < argc; i++) {
    tb_convolve *c = read_convfile(argv[i]);
    ds = put_convolve(ds, argv[i], c);
    log_debug("Read convolver %s", argv[i]);
  }

  for (int r = 0; r <= (int) cfg_count; r++) {
    if (r > 0) {
      log_debug("Round %d", r);
      for (data_series *kds = ds->next; kds; kds = kds->next)
        convolve(ds, kds);
    }
    char *outfile = ssprintf(cfg_output, r);
    log_debug("  Writing data to %s", outfile);
    dump_data(outfile, ds->data, ds->len);
    free(outfile);
  }

  free_series(ds);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
