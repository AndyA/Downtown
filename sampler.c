/* sampler.c */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "sampler.h"
#include "util.h"

sampler_params *sampler_new_params() {
  return alloc(sizeof(sampler_params));
}

static void ignore(const char *m, ...) {}
#define die ignore

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

  if (*cp == ',') sp->next = sampler_parse_params(cp + 1);
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

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
