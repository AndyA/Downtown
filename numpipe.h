/* numpipe.h */

#ifndef NUMPIPE_H_
#define NUMPIPE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { NUMPIPE_FIRST, NUMPIPE_AVERAGE, NUMPIPE_LAST } numpipe_policy;
typedef struct {
  numpipe_policy policy;
  double value;
  double v_first, v_last, v_total;
  unsigned v_seen;
} numpipe;

numpipe *numpipe_new(numpipe_policy policy, double v);

numpipe *numpipe_new_first(double v);
numpipe *numpipe_new_average(double v);
numpipe *numpipe_new_last(double v);

void numpipe_free(numpipe *np);

void numpipe_put(numpipe *np, double v);
double numpipe_get(numpipe *np);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
