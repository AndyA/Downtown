/* sampler.c */

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
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
 *   name1=123.4,name2=91203,name3='Some text'
 *
 */
sampler_params *sampler_parse_params(const char *spec) {
  sampler_params *sp = sampler_new_params();
  const char *cp = spec;
  char *ep;

  sp->value = NAN;

  const char *np = cp;
  while (*cp && (isalpha(*cp) || *cp == '_' || (cp != np && isdigit(*cp)))) cp++;
  if (np == cp) die("Missing name");
  if (*cp != '=') die("Missing '=' (at %s)", cp);
  const char *ne = cp++;

  const char *vp, *ve, *vn;

  if (*cp == '"' || *cp == '\'') {
    char quote = *cp++;
    vp = cp;
    ve = strchr(vp, quote);
    if (!ve) die("Missing %c", quote);
    vn = ve + 1;
  }
  else {
    vp = cp;
    ve = strchr(vp, ',');
    if (!ve) ve = vp + strlen(vp);
    vn = ve;
    double v = strtod(vp, &ep);
    if (ep == ve) sp->value = v;
  }

  sp->name = alloc(ne - np + 1);
  memcpy(sp->name, np, ne - np);
  sp->text = alloc(ve - vp + 1);
  memcpy(sp->text, vp, ve - vp);

  cp = vn;
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
    free(sp->text);
    free(sp);
  }
}

sampler_params *sampler_find_param(sampler_params *sp, const char *name) {
  for (; sp; sp = sp->next) if (0 == strcmp(name, sp->name)) return sp;
  return NULL;
}

static sampler_params *_require(sampler_params *sp, const char *name) {
  sampler_params *pp = sampler_find_param(sp, name);
  if (!pp) die("Missing param %s", name);
  return pp;
}

double sampler_require_double(sampler_params *sp, const char *name) {
  sampler_params *pp = _require(sp, name);
  if (isnan(pp->value)) die("%s is not a number", name);
  return pp->value;
}

const char *sampler_require_text(sampler_params *sp, const char *name) {
  return _require(sp, name)->text;
}

const char *sampler_optional_text(sampler_params *sp, const char *name) {
  sampler_params *pp = sampler_find_param(sp, name);
  return pp ? pp->text : NULL;
}

sampler_params *sampler_set_param(sampler_params *sp, const char *name, const char *text, double value) {
  if (!sp) {
    sp = sampler_new_params();
    sp->name = sstrdup(name);
  }
  else if (strcmp(sp->name, name)) {
    sp->next = sampler_set_param(sp->next, name, text, value);
    return sp;
  }

  free(sp->text);
  sp->text = sstrdup(text);
  sp->value = value;
  return sp;
}

static sampler_params *_merge_params(sampler_params *sp, const sampler_params *delta) {
  for (; delta; delta = delta->next)
    sp = sampler_set_param(sp, delta->name, delta->text, delta->value);
  return sp;
}

sampler_params *sampler_merge_params(const sampler_params *a, const sampler_params *b) {
  return _merge_params(_merge_params(NULL, a), b);
}

sampler_params *sampler_clone_params(const sampler_params *sp) {
  return _merge_params(NULL, sp);
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
sampler_context *sampler_new(const char *spec, const char *name) {
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
  ctx->name = sstrdup(name);

  if (si->i.default_config)  {
    ctx->params = sampler_merge_params(sampler_parse_params(si->i.default_config), sp);
    sampler_free_params(sp);
  }
  else {
    ctx->params = sp;
  }

  log_info("Sampler: %s", spec);
  for (sampler_params *pp = ctx->params; pp; pp = pp->next)
    if (isnan(pp->value))
      log_info("  %-15s = '%s'", pp->name, pp->text);
    else
      log_info("  %-15s = %f", pp->name, pp->value);

  return ctx;
}

void sampler_free(sampler_context *ctx) {
  if (ctx) {
    if (ctx->class->free) ctx->class->free(ctx);
    sampler_free_params(ctx->params);
    free(ctx->name);
    free(ctx->buf);
    free(ctx);
  }
}

size_t sampler_init(sampler_context *ctx, unsigned w, unsigned h) {
  ctx->width = w;
  ctx->height = h;
  if (ctx->class->init) return ctx->class->init(ctx);
  return 0;
}

double *sampler_sample(sampler_context *ctx, const uint8_t *in) {
  return ctx->class->sample(ctx, in);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
