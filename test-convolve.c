/* test-convolve.c */

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

typedef struct data_series {
  struct data_series *next;
  char *name;
  double *data;
  size_t len;
  tb_convolve *c;
} data_series;

static char *cfg_output = NULL;
static double cfg_count = 10;

static void usage() {
  fprintf(stderr, "Usage: " PROG " [options] -o <output.dat> <data.dat> <kernel.dat>...\n\n"
          "Options:\n"
          "  -h, --help                See this message\n"
          "  -c, --count               Number of iterations (default: 10)\n"
          "  -o, --output              Set output pattern. Include %%d or similar\n"
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
    {NULL, 0, NULL, 0}
  };

  while (ch = getopt_long(*argc, *argv, "ho:c:", opts, &oidx), ch != -1) {
    switch (ch) {

    case 'c':
      cfg_count = parse_double(optarg);
      break;

    case 'o':
      cfg_output = optarg;
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

static data_series *put_series(data_series *ds, const char *name, double *data, size_t len) {
  if (!ds) {
    ds = alloc(sizeof(data_series));
    ds->name = sstrdup(name);
    ds->data = data;
    ds->len = len;
    return ds;
  }
  ds->next = put_series(ds->next, name, data, len);
  return ds;
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
  tb_convolve_apply(kernel->c, tmp, data->data, data->len);
  memcpy(data->data, tmp, sizeof(double) * data->len);
}

int main(int argc, char *argv[]) {
  data_series *ds = NULL;

  log_info("Starting " PROG);

  parse_options(&argc, &argv);
  if (argc < 2 || !cfg_output) usage();

  for (int i = 0; i < argc; i++) {
    size_t size;
    double *data = read_numfile(argv[i], &size);
    ds = put_series(ds, argv[i], data, size);
    log_debug("Read %llu data from %s", (unsigned long long) size, argv[i]);
  }

  for (int r = 1; r <= (int) cfg_count; r++) {
    log_debug("Round %d", r);
    for (data_series *kds = ds->next; kds; kds = kds->next) {
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
