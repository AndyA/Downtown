/* tb_convolve.c */

#include <math.h>
#include <stdlib.h>

#include "tb_convolve.h"
#include "util.h"

#define NOWT 0.000000001

tb_convolve *tb_convolve_new(unsigned len,
                             const double *coef) {
  return tb_convolve_new_signed(len, coef, coef);
}

tb_convolve *tb_convolve_new_signed(unsigned len,
                                    const double *pos_coef,
                                    const double *neg_coef) {
  tb_convolve *c = (tb_convolve *) alloc(sizeof(tb_convolve));
  c->len = len;
  c->pos_coef = memdup(pos_coef, sizeof(double) * len);
  if (pos_coef == neg_coef)
    c->neg_coef = c->pos_coef;
  else
    c->neg_coef = memdup(neg_coef, sizeof(double) * len);
  return c;
}

void tb_convolve_free(tb_convolve *c) {
  if (c) {
    if (c->pos_coef != c->neg_coef) free(c->pos_coef);
    free(c->neg_coef);
    free(c);
  }
}

static double _scale_coef(const double *coef, double pos, double sa) {
  if (sa < NOWT) return 0;
  int p = (int) pos;
  double sum = coef[p] * MIN(sa, 1 - (pos - (double) p));
  p++;
  for (int i = 1; i < (int)(sa - p); i++)
    sum += coef[p++];
  double rem = sa - (double) p;
  if (rem > 0.00001) sum += coef[p] * rem;
  return sum / sa;
}

static double _calc(const tb_convolve *c, double n, double pos, double sa, double home) {
  double *coef_p = n < home ? c->neg_coef : c->pos_coef;
  return n * _scale_coef(coef_p, pos, sa);
}

#define SA(s) (fabs(s) < NOWT ? centre : 1 / (s))

double tb_convolve_calc(const tb_convolve *c, const double *in, unsigned len, unsigned pos) {
  double cpos, done = 0, sum = 0, centre = (double)c->len  / 2;
  double home = in[pos];
  double home_sa = SA(home);
  unsigned p;

  for (cpos = centre - home_sa / 2, p = pos; cpos >= 0 && p-- > 0;) {
    double sample = in[p];
    double sa = SA(sample);
    double start = MAX(0, cpos - sa);
    double span = cpos - start;
    sum += _calc(c, sample, start, span, home);
    cpos -= sa;
    done += span;
  }

  for (cpos = centre - home_sa / 2, p = pos; cpos < (double) c->len && p < len; p++) {
    double sample = in[p];
    double sa = SA(sample);
    double start = cpos;
    double span = MIN(sa, (double)c->len - start);
    sum += _calc(c, sample, start, span, home);
    cpos += sa;
    done += span;
  }

  return sum * (double) c->len / done;
}

void tb_convolve_apply(const tb_convolve *c, double *out, const double *in, unsigned len) {
  for (unsigned i = 0; i < len; i++)
    out[i] = tb_convolve_calc(c, in, len, i);
}

double tb_convolve_elapsed(const double *series, unsigned len) {
  double total = 0;
  for (unsigned i = 0; i < len; i++)
    total += 1 / series[i];
  return total;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
