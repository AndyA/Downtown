/* tb_convolve.c */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
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

double tb_convolve__sample(const double *coef, double pos, double sa) {
  double sum = 0;
  for (int p = (int) pos;; p++) {
    double left = MAX(0, pos - (double) p);
    double right = MIN(1, pos + sa - (double) p);
    if (right < 0) break;
    double s0 = coef[p];
    double s1 = coef[p + 1];
    double ls = s0 * (1 - left) + s1 * left;
    double rs = s0 * (1 - right) + s1 * right;
    sum += (ls + rs) * (right - left) / 2;
  }
  return sum;
}

static double _calc(const tb_convolve *c, double n, double pos, double sa, double home) {
  double *coef_p = n < home ? c->neg_coef : c->pos_coef;
  double sc = tb_convolve__sample(coef_p, pos, sa);
  /*  log_debug("    _calc(c=%p, n=%f, pos=%f, sa=%f, home=%f) -> %f (sc=%f)",*/
  /*            c, n, pos, sa, home, n * sc, sc);*/
  return n * sc;
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
    double span = MIN(cpos, (double)c->len) - start;
    sum += _calc(c, sample, start, span, home);
    cpos -= sa;
    done += span;
  }

  for (cpos = centre - home_sa / 2, p = pos; cpos < (double) c->len && p < len; p++) {
    double sample = in[p];
    double sa = SA(sample);
    double start = MAX(0, cpos);
    double span = MIN(sa, (double)c->len - start);
    sum += _calc(c, sample, start, span, home);
    cpos += sa;
    done += span;
  }

  /*  log_debug("    sum=%f, len=%f, done=%f", sum, (double)c->len, done);*/

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

double tb_convolve_translate(const double *in, unsigned ilen, double *out, unsigned olen) {
  memset(out, 0, sizeof(double) * olen);

  double ipos, opos;
  for (ipos = 0, opos = 0; ipos < (double) ilen && opos < (double) olen;) {
    unsigned ip = (unsigned) ipos;
    unsigned op = (unsigned) opos;
    double rate = in[ip];
    double igot = ((double) ip + 1 - ipos) / rate;
    double owant = (double) op + 1 - opos;
    double chunk = MIN(igot, owant);
    out[op] += tb_convolve__sample(in, ipos, chunk);
    /*    fprintf(stdout, "# ipos=%f, opos=%f, rate=%f, igot=%f, owant=%f, chunk=%f\n",*/
    /*            ipos, opos, rate, igot, owant, chunk);*/
    ipos += chunk * rate;
    opos += chunk;
  }

  return opos;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
