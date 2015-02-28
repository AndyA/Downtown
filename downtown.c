/* downtown.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yuv4mpeg2.h"

static void help(void) {
  printf("Syntax: downtown <config file>...\n");
}

static void callback(y4m2_reason reason,
                     const y4m2_parameters *parms,
                     y4m2_frame *frame,
                     void *ctx) {
  (void) parms;
  (void) frame;
  (void) ctx;

  switch (reason) {
  case Y4M2_START:
    break;
  case Y4M2_FRAME:
    break;
  case Y4M2_END:
    break;
  }
}

int main(int argc, char *argv[]) {
  if (argc == 1 || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))) {
    help();
    return 1;
  }

  y4m2_output *out = y4m2_output_next(callback, NULL);
  y4m2_parse(stdin, out);
  y4m2_free_output(out);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
