/* json.h */

#ifndef JSON_H_
#define JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "jsondata.h"

jd_var *json_load_json(jd_var *out, FILE *f);
jd_var *json_load_file(jd_var *out, const char *fn);
double *json_get_real(jd_var *ar, size_t *sizep);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
