/* json.c */

#include <errno.h>
#include <jd_pretty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "util.h"

static jd_var *load_string(jd_var *out, FILE *f) {
  char buf[0x10000];
  size_t got;

  jd_set_empty_string(out, 1);
  while (got = fread(buf, 1, sizeof(buf), f), got)
    jd_append_bytes(out, buf, got);
  return out;
}

jd_var *json_load(jd_var *out, FILE *f) {
  jd_var json = JD_INIT;
  jd_from_json(out, load_string(&json, f));
  jd_release(&json);
  return out;
}

jd_var *json_load_file(jd_var *out, const char *fn) {
  FILE *fl = fopen(fn, "r");
  if (!fl) jd_throw("Can't read %s: %s\n", fn, strerror(errno));
  jd_var *v = json_load(out, fl);
  fclose(fl);
  return v;
}

double *json_get_real(jd_var *ar, size_t *sizep) {
  size_t size = jd_count(ar);
  double *data = alloc_no_clear(sizeof(double) * size);
  jd_var *slot = jd_get_idx(ar, 0);
  for (unsigned i = 0; i < size; i++)
    data[i] = jd_get_real(&slot[i]);
  if (sizep) *sizep = size;
  return data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
