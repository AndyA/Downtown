/* average.h */

#ifndef AVERAGE_H_
#define AVERAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned len;
  unsigned in, out;
  unsigned full;
  double total;
  double min, max;
  double *data;
  double (*in_func)(double x);
  double (*out_func)(double x);
} average;

average *average_new(unsigned len);
average *average_logarithmic(average *avg);
average *average_new_log(unsigned len);
void average_free(average *avg);

unsigned average_used(const average *avg);
int average_ready(const average *avg);
double average_get(const average *avg);
double average_min(average *avg);
double average_max(average *avg);
double average_pop(average *avg);
double average_push(average *avg, double datum);

void average_range(average *avg, double *min, double *max);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
