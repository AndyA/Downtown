/* average.c */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "average.h"
#include "util.h"

average *average_new(unsigned len) {
  average *avg = alloc(sizeof(average));
  avg->data = alloc(sizeof(double) * len);
  avg->len = len;
  return avg;
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
  return used < 0 ? used + avg->len : used;
}

double average_get(const average *avg) {
  return avg->total / (double) average_used(avg);
}

double average_pop(average *avg) {
  if (avg->in == avg->out && !avg->full) return NAN;
  double datum = avg->data[avg->out++];
  if (avg->out == avg->len) avg->out = 0;
  avg->total -= datum;
  avg->full = 0;
  return datum;
}

double average_push(average *avg, double datum) {
  if (avg->full)(void) average_pop(avg);
  avg->data[avg->in++] = datum;
  if (avg->in == avg->len) avg->in = 0;
  if (avg->in == avg->out) avg->full = 1;
  avg->total += datum;
  return average_get(avg);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
