/* sampler.c */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "sampler.h"
#include "util.h"

typedef struct sampler_info_list sampler_info_list;
struct sampler_info_list {
  sampler_info_list *next;
  sampler_info i;
};

static sampler_info_list *samplers = NULL;

sampler_params *sampler_new_params() {
  return alloc(sizeof(sampler_params));
}

/* Parse sampler params. Param strings look like
 *
 *   name1=123.4,name2=91203
 *
 * Parameters must currently be numeric.
 */
sampler_params *sampler_parse_params(const char *spec) {
  sampler_params *sp = sampler_new_params();
  const char *cp = spec;
  char *ep;
  double v;

  const char *np = cp;
  while (*cp && (isalpha(*cp) || (cp != np && isdigit(*cp)))) cp++;
  if (np == cp) die("Missing name");
  if (*cp != '=') die("Missing '='");
  const char *ne = cp++;
  v = strtod(cp, &ep);
  if (ep == cp) die("Bad number");

  sp->name = alloc(ne - np + 1);
  memcpy(sp->name, np, ne - np);
  sp->value = v;

  cp = ep;
  if (*cp == ',') {
    sp->next = sampler_parse_params(cp + 1);
    return sp;
  }

  if (*cp) die("Syntax error: %s", cp);

  return sp;
}

void sampler_free_params(sampler_params *sp) {
  if (sp) {
    sampler_free_params(sp->next);
    free(sp->name);
    free(sp);
  }
}

void sampler_register(const sampler_info *info) {
  sampler_info_list *si = alloc(sizeof(sampler_info_list));
  memcpy(&si->i, info, sizeof(*info));
  si->next = samplers;
  samplers = si;
}

static sampler_info_list *find_sampler(const char *name) {
  for (sampler_info_list *si  = samplers; si; si = si->next) {
    if (0 == strcmp(name, si->i.name)) return si;
  }
  return NULL;
}

/* Spec syntax:
 *
 *   <name>[:<params>]
 *
 */
sampler_context *sampler_new(const char *spec) {
  sampler_info_list *si;
  sampler_params *sp;
  sampler_context *ctx;

  const char *colon = strchr(spec, ':');

  if (colon) {
    char *name = alloc(colon - spec + 1);
    memcpy(name, spec, colon - spec);
    si = find_sampler(name);
    sp = sampler_parse_params(colon + 1);
    free(name);
  }
  else {
    si = find_sampler(spec);
    sp = NULL;
  }

  if (!si) die("No sampler matches %s", spec);

  ctx = alloc(sizeof(sampler_context));
  ctx->class = &si->i;
  ctx->params = sp;

  return ctx;
}

void sampler_free(sampler_context *ctx) {
  if (ctx->class->free) ctx->class->free(ctx);
  sampler_free_params(ctx->params);
  free(ctx->buf);
  free(ctx);
}

sampler_context *sampler_init(sampler_context *ctx, unsigned w, unsigned h) {
  ctx->width = w;
  ctx->height = h;
  if (ctx->class->init) ctx->class->init(ctx);
  return ctx;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
