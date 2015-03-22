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
  double *data;
} average;

average *average_new(unsigned len);
void average_free(average *avg);

unsigned average_used(const average *avg);
double average_get(const average *avg);
double average_pop(average *avg);
double average_push(average *avg, double datum);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
