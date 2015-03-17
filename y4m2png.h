/* y4m2png.h */

#ifndef Y4M2PNG_H_
#define Y4M2PNG_H_

#ifdef __cplusplus
extern "C" {
#endif

  void y4m2_png_write(const y4m2_frame *frame, FILE *out);
  void y4m2_png_write_file(const y4m2_frame *frame, const char *name);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
