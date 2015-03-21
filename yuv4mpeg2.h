/* yuv4mpeg2.h */

#ifndef YUV4MPEG2_H_
#define YUV4MPEG2_H_

#include <stdint.h>
#include <stdio.h>

#include "colour.h"

#define Y4M2_FIRST 'A'
#define Y4M2_LAST 'Z'
#define Y4M2_PARMS (Y4M2_LAST-Y4M2_FIRST+1)

typedef struct {
  char *parm[Y4M2_PARMS];
} y4m2_parameters;

enum {Y4M2_Y_PLANE, Y4M2_Cb_PLANE, Y4M2_Cr_PLANE, Y4M2_N_PLANE};

typedef struct {
  unsigned xs, ys;
  unsigned stride;
  size_t size;
  unsigned fill;
} y4m2_plane_info;

typedef struct {
  unsigned width;
  unsigned height;
  size_t size;
  y4m2_plane_info plane[Y4M2_N_PLANE];
} y4m2_frame_info;

typedef void (*y4m2_free_func)(void *);

typedef struct {
  void *value;
  y4m2_free_func destructor;
  unsigned refs;
} y4m2_note_value;

typedef struct y4m2_note y4m2_note;
struct y4m2_note {
  y4m2_note *next;
  char *name;
  y4m2_note_value *v;
};

typedef struct y4m2_frame y4m2_frame;
struct y4m2_frame {
  unsigned refcnt;
  y4m2_frame_info i;
  uint8_t *buf;
  uint8_t *plane[Y4M2_N_PLANE];
  uint64_t sequence;
  double elapsed;
  y4m2_note *notes;
  unsigned is_window;
  y4m2_frame *parent; /* if window */
};

typedef enum { Y4M2_START, Y4M2_FRAME, Y4M2_END } y4m2_reason;

typedef void (*y4m2_callback)(y4m2_reason reason,
                              const y4m2_parameters *parms,
                              y4m2_frame *frame,
                              void *ctx);

typedef struct {
  y4m2_callback cb;
  void *ctx;
} y4m2_output;

extern int y4m2__frames_allocated;

/* Internal, exported for tests */

void y4m2__parse_parms(y4m2_parameters *parms, char *buf);
void y4m2__format_parms(FILE *out, const y4m2_parameters *parms);
void y4m2__plane_map(const y4m2_frame *in, unsigned xs[Y4M2_N_PLANE], unsigned ys[Y4M2_N_PLANE]);

/* Paramter handling */

y4m2_parameters *y4m2_new_parms(void);
void y4m2_free_parms(y4m2_parameters *parms);
y4m2_parameters *y4m2_merge_parms(y4m2_parameters *parms, const y4m2_parameters *merge);
y4m2_parameters *y4m2_clone_parms(const y4m2_parameters *orig);
const char *y4m2_get_parm(const y4m2_parameters *parms, const char *name);
void y4m2_set_parm(y4m2_parameters *parm, const char *name, const char *value);
y4m2_parameters *y4m2_adjust_parms(const y4m2_parameters *parms, const char *fmt, ...);

/* Frame lifecycle */

void y4m2_parse_frame_info(y4m2_frame_info *info, const y4m2_parameters *parms);
y4m2_frame *y4m2_new_frame_info(const y4m2_frame_info *info);
y4m2_frame *y4m2_new_frame(const y4m2_parameters *parms);
y4m2_frame *y4m2_like_frame(const y4m2_frame *frame);
y4m2_frame *y4m2_clone_frame(const y4m2_frame *frame);
y4m2_frame *y4m2_clear_frame(y4m2_frame *frame);
y4m2_frame *y4m2_retain_frame(y4m2_frame *frame);
void y4m2_release_frame(y4m2_frame *frame);

/* Frame notes */

void y4m2_set_note(y4m2_frame *frame, const char *name, void *value, y4m2_free_func destructor);
void y4m2_remove_notes(y4m2_frame *frame);
void *y4m2_find_note(const y4m2_frame *frame, const char *name);
void *y4m2_need_note(const y4m2_frame *frame, const char *name);
void y4m2_copy_notes(y4m2_frame *dst, const y4m2_frame *src);
int y4m2_has_notes(const y4m2_frame *frame);

/* Pipeline */

int y4m2_parse(FILE *in, y4m2_output *out);
int y4m2_emit_start(y4m2_output *out, const y4m2_parameters *parms);
int y4m2_emit_frame(y4m2_output *out, const y4m2_parameters *parms, y4m2_frame *frame);
int y4m2_emit_end(y4m2_output *out);
int y4m2_emit(y4m2_output *out, y4m2_reason reason,
              const y4m2_parameters *parms,
              y4m2_frame *frame);
y4m2_output *y4m2_output_file(FILE *out);
y4m2_output *y4m2_output_next(y4m2_callback cb, void *ctx);
y4m2_output *y4m2_output_null(void);
void y4m2_free_output(y4m2_output *out);

/* To, from float */

size_t y4m2_frame_to_float(const y4m2_frame *in, colour_floats *out);
void y4m2_float_to_frame(const colour_floats *in, y4m2_frame *out);

/* Drawing */

void y4m2_draw_point(y4m2_frame *frame, int x, int y, int vy, int vu, int vv);
void y4m2_draw_line(y4m2_frame *frame, int x0, int y0, int x1, int y1, int vy, int vu, int vv);

y4m2_frame *y4m2_window(y4m2_frame *frame, int x, int y, int w, int h);

/* Misc */

#define y4m2_tell_me_about_stride(frame) \
  y4m2__tell_me_about_stride(__FILE__, __LINE__, frame)

void y4m2__tell_me_about_stride(const char *file, int line, const y4m2_frame *frame);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
