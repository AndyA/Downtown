/* y4m2png.c */

#include <errno.h>
#include <png.h>
#include <stdio.h>

#include "colour.h"
#include "util.h"
#include "yuv4mpeg2.h"

static png_voidp _png_alloc(png_structp png_ptr, png_size_t size) {
  (void) png_ptr;
  return alloc(size);
}

static void _png_free(png_structp png_ptr, png_voidp ptr) {
  (void) png_ptr;
  free(ptr);
}

static void _png_write(png_structp png_ptr, png_bytep buf, png_size_t sz) {
  FILE *out = (FILE *) png_get_io_ptr(png_ptr);
  size_t done = fwrite(buf, 1, sz, out);
  if (done != sz) die("Write failed: %s", strerror(errno));
}

static void _png_flush(png_structp png_ptr) {
  (void) png_ptr;
}

static void _png_yuv_to_colour(const y4m2_frame *in, unsigned row, colour_bytes *out) {
  uint8_t *plane[Y4M2_N_PLANE];
  unsigned xs[Y4M2_N_PLANE], ys[Y4M2_N_PLANE];
  unsigned width = in->i.width;

  y4m2__plane_map(in, plane, xs, ys);

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    for (unsigned x = 0; x < width; x++) {
      out[x].c[p] = plane[p][(row >> ys[p]) * (width >> xs[p]) + (x >> xs[p])];
    }
  }
}

static void _png_colour_to_rgb(const colour_bytes *in, unsigned width, uint8_t *out) {
  colour_bytes rgb;
  for (unsigned x = 0; x < width; x++) {
    colour_b_yuv2rgb(&in[x], &rgb);
    out[x * 3 + 0] = rgb.c[cR];
    out[x * 3 + 1] = rgb.c[cG];
    out[x * 3 + 2] = rgb.c[cB];
  }
}

void y4m2_png_write(const y4m2_frame *frame, FILE *out) {
  png_structp png_ptr;
  png_infop info_ptr;
  colour_bytes *col_buf;
  uint8_t *row_buf;
  unsigned width = frame->i.width;
  unsigned height = frame->i.height;
  int err;

  png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,
                                      NULL, NULL, NULL, NULL,
                                      _png_alloc, _png_free);
  if (!png_ptr) goto fail;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) goto fail;

  if (err = setjmp(png_jmpbuf(png_ptr)), err) goto fail;

  png_set_write_fn(png_ptr, (void *) out, _png_write, _png_flush);

  png_set_IHDR(png_ptr, info_ptr,
               width, height, 8,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);

  col_buf = alloc(sizeof(colour_bytes) * width);
  row_buf = alloc(3 * width);

  for (unsigned row = 0; row < height; row++) {
    _png_yuv_to_colour(frame, row, col_buf);
    _png_colour_to_rgb(col_buf, width, row_buf);
    png_write_row(png_ptr, row_buf);
  }

  png_write_end(png_ptr, info_ptr);

  free(row_buf);
  free(col_buf);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return;

fail:
  die("Failed to write PNG");
}

void y4m2_png_write_file(const y4m2_frame *frame, const char *name) {
  FILE *out = fopen(name, "wb");
  if (!out) die("Can't write %s: %s", name, strerror(errno));
  y4m2_png_write(frame, out);
  fclose(out);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
