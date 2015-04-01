/* average.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "average.h"
#include "util.h"

static double identity(double x) {
  return x;
}

average *average_new(unsigned len) {
  average *avg = alloc(sizeof(average));
  avg->data = alloc(sizeof(double) * len);
  avg->len = len;
  avg->min = avg->max = NAN;
  avg->in_func = identity;
  avg->out_func = identity;
  return avg;
}

average *average_logarithmic(average *avg) {
  avg->in_func = log;
  avg->out_func = exp;
  return avg;
}

average *average_new_log(unsigned len) {
  return average_logarithmic(average_new(len));
}

void average_free(average *avg) {
  if (avg) {
    free(avg->data);
    free(avg);
  }
}

unsigned average_used(const average *avg) {
  if (avg->full) return avg->len;
  int used = avg->in - avg->out;
  return used < 0 ? used + (int) avg->len : used;
}

int average_ready(const average *avg) {
  return average_used(avg) >= avg->len / 2;
}

double average_get(const average *avg) {
  return avg->out_func(avg->total / (double) average_used(avg));
}

double average_pop(average *avg) {
  if (avg->in == avg->out && !avg->full) return NAN;
  double datum = avg->data[avg->out++];
  if (avg->out == avg->len) avg->out = 0;
  avg->total -= datum;
  avg->full = 0;
  avg->min = NAN;
  avg->max = NAN;
  return datum;
}

double average_push(average *avg, double datum) {
  double xdat = avg->in_func(datum);
  if (avg->full)(void) average_pop(avg);
  avg->data[avg->in++] = xdat;
  if (avg->in == avg->len) avg->in = 0;
  if (avg->in == avg->out) avg->full = 1;
  avg->total += xdat;
  avg->min = NAN;
  avg->max = NAN;
  return average_get(avg);
}

static void min_max(average *avg) {
  avg->min = NAN;
  avg->max = NAN;
  unsigned pos = avg->out;
  for (unsigned used = average_used(avg); used; used--) {
    double datum = avg->data[pos++];
    if (pos == avg->len) pos = 0;
    if (isnan(avg->min) || datum < avg->min) avg->min = datum;
    if (isnan(avg->max) || datum > avg->max) avg->max = datum;
  }
}

double average_min(average *avg) {
  if (isnan(avg->min) || isnan(avg->max)) min_max(avg);
  return avg->out_func(avg->min);
}

double average_max(average *avg) {
  if (isnan(avg->min) || isnan(avg->max)) min_max(avg);
  return avg->out_func(avg->max);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
