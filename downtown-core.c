/* downtown-core.c */

#include <stdlib.h>

#include "downtown.h"
#include "voronoi.h"
#include "zigzag.h"

void downtown_init(void) {
  /* add new samplers here */
  zigzag_register();
  voronoi_register();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
