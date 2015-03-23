/* numpipe.c */

#include <math.h>

#include "numpipe.h"
#include "util.h"

numpipe *numpipe_new(numpipe_policy policy, double v) {
  numpipe *np = alloc(sizeof(numpipe));
  np->policy = policy;
  np->value = v;
  return np;
}


numpipe *numpipe_new_first(double v) {
  return numpipe_new(NUMPIPE_FIRST, v);
}

numpipe *numpipe_new_average(double v) {
  return numpipe_new(NUMPIPE_AVERAGE, v);
}

numpipe *numpipe_new_last(double v) {
  return numpipe_new(NUMPIPE_LAST, v);
}


void numpipe_free(numpipe *np) {
  free(np);
}

void numpipe_put(numpipe *np, double v) {
  np->v_last = v;
  if (np->v_seen == 0) np->v_first = v;
  np->v_total += v;
  np->v_seen++;
}

static double _get(numpipe *np) {
  switch (np->policy) {
  case NUMPIPE_FIRST:
    return np->v_first;
  case NUMPIPE_AVERAGE:
    return np->v_total / (double) np->v_seen;
  case NUMPIPE_LAST:
    return np->v_last;
  }
  return 0;
}

double numpipe_get(numpipe *np) {
  if (np->v_seen == 0) return np->value;
  np->value = _get(np);
  np->v_first = NAN;
  np->v_last = NAN;
  np->v_total = 0;
  np->v_seen = 0;
  return np->value;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
