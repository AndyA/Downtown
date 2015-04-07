/* sampler.h */

#ifndef SAMPLER_H_
#define SAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define sampler_byte2double(x) ((((double) x) - 128) / 128)

typedef struct sampler_params sampler_params;
struct sampler_params {
  sampler_params *next;
  char *name;
  char *text;
  double value;
};

typedef struct sampler_context sampler_context;

typedef size_t (*sampler_init_func)(sampler_context *ctx);
typedef double *(*sampler_sample_func)(sampler_context *ctx, const uint8_t *in);
typedef void (*sampler_free_func)(sampler_context *ctx);

typedef struct {
  const char *name;
  const char *default_config;
  sampler_init_func init;
  sampler_sample_func sample;
  sampler_free_func free;
} sampler_info;

struct sampler_context {
  const sampler_info *class;
  char *name;
  sampler_params *params;
  unsigned width, height;
  double *buf;
  char *spec;
  void *user;
};

sampler_params *sampler_new_params();
sampler_params *sampler_parse_params(const char *spec);
sampler_params *sampler_find_param(sampler_params *sp, const char *name);
sampler_params *sampler_set_param(sampler_params *sp, const char *name, const char *text, double value);
sampler_params *sampler_merge_params(const sampler_params *a, const sampler_params *b);
sampler_params *sampler_clone_params(const sampler_params *sp);

double sampler_require_double(sampler_params *sp, const char *name);
const char *sampler_require_text(sampler_params *sp, const char *name);
const char *sampler_optional_text(sampler_params *sp, const char *name);
void sampler_free_params(sampler_params *sp);

void sampler_register(const sampler_info *info);

sampler_context *sampler_new(const char *spec, const char *name);
void sampler_free(sampler_context *ctx);
size_t sampler_init(sampler_context *ctx, unsigned w, unsigned h);
double *sampler_sample(sampler_context *ctx, const uint8_t *in);
char *sampler_spec(sampler_context *ctx);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
