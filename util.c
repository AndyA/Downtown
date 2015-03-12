/* util.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

void die(const char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  fprintf(stderr, "Fatal :");
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
