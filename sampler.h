/* sampler.h */

#ifndef SAMPLER_H_
#define SAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sampler_params sampler_params;

struct sampler_params {
  sampler_params *next;
  char *name;
  double value;
};

typedef struct {
} sampler;

sampler_params *sampler_new_params();
sampler_params *sampler_parse_params(const char *spec);
void sampler_free_params(sampler_params *sp);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
