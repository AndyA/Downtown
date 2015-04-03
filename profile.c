/* profile.c */

#include "json.h"
#include "profile.h"
#include "util.h"

static void unpack(profile *p) {
  p->baseline = json_get_real(jd_get_ks(&p->config, "baseline", 0), &p->len);
}

profile *profile_load(const char *filename) {
  profile *p = alloc(sizeof(profile));
  p->filename = sstrdup(filename);
  json_load_file(&p->config, p->filename);
  unpack(p);
  return p;
}

void profile_free(profile *p) {
  jd_release(&p->config);
  free(p->filename);
  free(p->baseline);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
