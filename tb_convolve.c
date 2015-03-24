/* tb_convolve.c */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "tb_convolve.h"
#include "util.h"

#define NOWT 0.000000001

#define LOG(x) (c->linear ? (x) : log(x))
#define EXP(x) (c->linear ? (x) : exp(x))

tb_convolve *tb_convolve_new(unsigned len,
                             const double *coef) {
  return tb_convolve_new_signed(len, coef, coef);
}

static double *dup_coef(const double *in, unsigned len) {
  double *out = alloc(sizeof(double) * len + 1); /* pad to avoid range check later */
  memcpy(out, in, sizeof(double) * len);
  return out;
}


tb_convolve *tb_convolve_new_signed(unsigned len,
                                    const double *pos_coef,
                                    const double *neg_coef) {
  tb_convolve *c = (tb_convolve *) alloc(sizeof(tb_convolve));
  c->len = len;
  c->hwm = len - 1;
  c->prescale = 1;
  c->pos_coef = dup_coef(pos_coef, len);
  c->neg_coef = pos_coef == neg_coef ? c->pos_coef : dup_coef(neg_coef, len);
  return c;
}

void tb_convolve_free(tb_convolve *c) {
  if (c) {
    if (c->pos_coef != c->neg_coef) free(c->pos_coef);
    free(c->neg_coef);
    free(c);
  }
}

tb_convolve *tb_convolve_set_prescale(tb_convolve *c, double prescale) {
  c->prescale = prescale;
  return c;
}

tb_convolve *tb_convolve_set_linear(tb_convolve *c, int linear) {
  c->linear = linear;
  return c;
}

double tb_convolve__sample(const double *coef, double pos, double span) {
  double sum = 0;
  for (int p = (int) pos;; p++) {
    double left = MAX(0, pos - (double) p);
    double right = MIN(1, pos + span - (double) p);
    if (right < 0) break;
    double s0 = coef[p];
    double s1 = coef[p + 1];
    double ls = s0 * (1 - left) + s1 * left;
    double rs = s0 * (1 - right) + s1 * right;
    sum += (ls + rs) * (right - left) / 2;
  }
  return sum;
}

static double _calc(const tb_convolve *c, double n, double pos, double span, double home) {
  double *coef_p = n < home ? c->neg_coef : c->pos_coef;
  double sc = tb_convolve__sample(coef_p, pos, span);
  return LOG(n * c->prescale) * sc;
}

#define SA(s) (fabs(s) < NOWT ? centre : 1 / (s))

double tb_convolve_calc(const tb_convolve *c, const double *in, unsigned len, unsigned pos) {
  double cpos, sum = 0, centre = (double) c->hwm / 2;
  double home = in[pos];
  double home_sa = SA(home);
  double done = 0;
  unsigned p;

  for (cpos = centre - home_sa / 2, p = pos; cpos >= 0 && p-- > 0;) {
    double sample = in[p];
    double sa = SA(sample);
    double start = MAX(0, cpos - sa);
    double span = MIN(cpos, (double)c->hwm) - start;
    sum += _calc(c, sample, start, span, home);
    cpos -= sa;
    done += span;
  }

  for (cpos = centre - home_sa / 2, p = pos; cpos < (double) c->hwm && p < len; p++) {
    double sample = in[p];
    double sa = SA(sample);
    double start = MAX(0, cpos);
    double span = MIN(sa, (double)c->hwm - start);
    sum += _calc(c, sample, start, span, home);
    cpos += sa;
    done += span;
  }

  return EXP(sum * (double) c->hwm / done) / c->prescale;
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
    ipos += chunk * rate;
    opos += chunk;
  }

  return opos;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
