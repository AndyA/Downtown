/* yuv4mpeg2.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "colour.h"
#include "util.h"
#include "yuv4mpeg2.h"

int y4m2__frames_allocated = 0;

// TODO
//
//  Should probably have the null and file outputs release frames rather than the parser
//  and require filters to release any frames that they drop and retain any they hang on to

static const char *tag[] = {
  "YUV4MPEG2", "FRAME"
};

y4m2_parameters *y4m2_new_parms(void) {
  return alloc(sizeof(y4m2_parameters));
}

void y4m2_free_parms(y4m2_parameters *parms) {
  int i;
  if (parms) {
    for (i = 0; i < Y4M2_PARMS; i++)
      free(parms->parm[i]);
    free(parms);
  }
}

static void y4m2__set(char **slot, const char *value) {
  if (*slot) free(*slot);
  if (value) {
    size_t l = strlen(value);
    *slot = alloc(l + 1);
    memcpy(*slot, value, l + 1);
  }
  else {
    *slot = NULL;
  }
}

y4m2_parameters *y4m2_merge_parms(y4m2_parameters *parms, const y4m2_parameters *merge) {
  if (merge)
    for (int i = 0; i < Y4M2_PARMS; i++)
      if (merge->parm[i])
        y4m2__set(&(parms->parm[i]), merge->parm[i]);
  return parms;
}

y4m2_parameters *y4m2_clone_parms(const y4m2_parameters *orig) {
  return y4m2_merge_parms(y4m2_new_parms(), orig);
}

int y4m2__get_index(const char *name) {
  if (!name)
    return -1;
  if (strlen(name) != 1 || name[0] < Y4M2_FIRST || name[0] > Y4M2_LAST)
    return -1;
  return  name[0] - Y4M2_FIRST;
}

const char *y4m2_get_parm(const y4m2_parameters *parms, const char *name) {
  int idx = y4m2__get_index(name);
  return idx >= 0 ? parms->parm[idx] : NULL;
}

void y4m2_set_parm(y4m2_parameters *parms, const char *name, const char *value) {
  int idx = y4m2__get_index(name);
  if (idx < 0) die("Bad parameter name: %s", name);
  y4m2__set(&(parms->parm[idx]), value);
}

static y4m2_parameters *_adjust(const y4m2_parameters *parms, const char *fmt, va_list ap) {
  char *spec = vssprintf(fmt, ap);

  y4m2_parameters *np = parms ? y4m2_clone_parms(parms) : y4m2_new_parms();
  y4m2_parameters *delta = y4m2_new_parms();

  y4m2__parse_parms(delta, spec);

  y4m2_merge_parms(np, delta);
  y4m2_free_parms(delta);
  free(spec);

  return np;
}

y4m2_parameters *y4m2_adjust_parms(const y4m2_parameters *parms, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  y4m2_parameters *np = _adjust(parms, fmt, ap);
  va_end(ap);

  return np;
}

static unsigned parse_num(const char *s) {
  if (s) {
    char *ep;
    unsigned n = (unsigned) strtoul(s, &ep, 10);
    if (ep > s && *ep == '\0') return n;
  }
  die("Bad number");
  return 0;
}

static void set_plane(y4m2_plane_info *pl, unsigned w, unsigned h, unsigned xs, unsigned ys) {
  pl->xs = xs;
  pl->ys = ys;

  pl->stride = w / xs;

  pl->size = (w * h) / (xs * ys);
}

static void set_planes(y4m2_frame_info *info,
                       unsigned xsY, unsigned ysY,
                       unsigned xsCb, unsigned ysCb,
                       unsigned xsCr, unsigned ysCr) {

  set_plane(&info->plane[Y4M2_Y_PLANE], info->width, info->height, xsY, ysY);
  set_plane(&info->plane[Y4M2_Cb_PLANE], info->width, info->height, xsCb, ysCb);
  set_plane(&info->plane[Y4M2_Cr_PLANE], info->width, info->height, xsCr, ysCr);

  info->size = 0;
  for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++)
    info->size += info->plane[pl].size;
}

void y4m2_parse_frame_info(y4m2_frame_info *info, const y4m2_parameters *parms) {
  static uint8_t pl_fill[Y4M2_N_PLANE] = { 16, 128, 128 };

  info->width = parse_num(y4m2_get_parm(parms, "W"));
  info->height = parse_num(y4m2_get_parm(parms, "H"));

  const char *cs = y4m2_get_parm(parms, "C");
  if (!cs) cs = "420";
  if (!strcmp("420", cs) ||
      !strcmp("420jpeg", cs) ||
      !strcmp("420mpeg2", cs) ||
      !strcmp("420paldv", cs)) {
    set_planes(info, 1, 1, 2, 2, 2, 2);
  }
  else if (!strcmp("422", cs)) {
    set_planes(info, 1, 1, 2, 1, 2, 1);
  }
  else if (!strcmp("444", cs)) {
    set_planes(info, 1, 1, 1, 1, 1, 1);
  }
  else {
    die("Unknown colourspace %s\n", cs);
  }

  for (int i = 0; i < Y4M2_N_PLANE; i++) {
    info->plane[i].fill = pl_fill[i];
  }
}

y4m2_frame *y4m2_clear_frame(y4m2_frame *frame) {
  for (int i = 0; i < Y4M2_N_PLANE; i++)
    memset(frame->plane[i], frame->i.plane[i].fill, frame->i.plane[i].size);
  return frame;
}

y4m2_frame *y4m2_new_frame_info(const y4m2_frame_info *info) {
  y4m2_frame *frame = alloc(sizeof(y4m2_frame));
  uint8_t *buf = frame->buf = alloc(info->size);

  for (int i = 0; i < Y4M2_N_PLANE; i++) {
    frame->plane[i] = buf;
    buf += info->plane[i].size;
  }

  frame->i = *info;
  y4m2__frames_allocated++;
  return y4m2_clear_frame(y4m2_retain_frame(frame));
}

y4m2_frame *y4m2_like_frame(const y4m2_frame *frame) {
  return y4m2_new_frame_info(&frame->i);
}

y4m2_frame *y4m2_new_frame(const y4m2_parameters *parms) {
  y4m2_frame_info info;
  y4m2_parse_frame_info(&info, parms);
  return y4m2_new_frame_info(&info);
}

y4m2_frame *y4m2_clone_frame(const y4m2_frame *frame) {

  y4m2_frame *nf = y4m2_like_frame(frame);
  memcpy(nf->buf, frame->buf, frame->i.size);
  return nf;
}

void y4m2_free_frame(y4m2_frame *frame) {
  if (frame) {
    if (frame->parent)
      y4m2_release_frame(frame->parent);
    else
      free(frame->buf);
    y4m2_remove_notes(frame);
    free(frame);
    y4m2__frames_allocated--;
  }
}

y4m2_frame *y4m2_retain_frame(y4m2_frame *frame) {
  if (frame) frame->refcnt++;
  return frame;
}

void y4m2_release_frame(y4m2_frame *frame) {
  if (frame && --frame->refcnt == 0)
    y4m2_free_frame(frame);
}

static y4m2_note_value *_retain_value(y4m2_note_value *v) {
  if (v) v->refs++;
  return v;
}

static void _free_value(y4m2_note_value *v) {
  if (v->destructor) v->destructor(v->value);
  free(v);
}

static void _release_value(y4m2_note_value *v) {
  if (v && --v->refs == 0)
    _free_value(v);
}

static y4m2_note_value *_new_value(void *value, y4m2_free_func destructor) {
  if (!value) return NULL;
  y4m2_note_value *v = alloc(sizeof(y4m2_note_value));
  v->value = value;
  v->destructor = destructor;
  return v;
}

static void _free_note(y4m2_note *note) {
  if (note) {
    free(note->name);
    _release_value(note->v);
    free(note);
  }
}

static void _free_notes(y4m2_note *note) {
  if (note) {
    _free_notes(note->next);
    _free_note(note);
  }
}

void y4m2_remove_notes(y4m2_frame *frame) {
  _free_notes(frame->notes);
  frame->notes = NULL;
}

static void _set_note_value(y4m2_note *note, y4m2_note_value *v) {
  _retain_value(v);
  _release_value(note->v);
  note->v = v;
}

static y4m2_note *_set_note(y4m2_note *note, const char *name, y4m2_note_value *v) {
  if (!note) {
    if (!v) return NULL;
    note = alloc(sizeof(y4m2_note));
    note->name = sstrdup(name);
  }
  else if (strcmp(note->name, name)) {
    note->next = _set_note(note->next, name, v);
    return note;
  }
  if (!v) {
    y4m2_note *next = note->next;
    _free_note(note);
    return next;
  }
  _set_note_value(note, v);
  return note;
}

void y4m2_set_note(y4m2_frame *frame, const char *name, void *value, y4m2_free_func destructor) {
  frame->notes = _set_note(frame->notes, name, _new_value(value, destructor));
}

void *y4m2_find_note(const y4m2_frame *frame, const char *name) {
  for (y4m2_note *note = frame->notes; note; note = note->next)
    if (0 == strcmp(note->name, name))
      return note->v->value;
  return NULL;
}

void *y4m2_need_note(const y4m2_frame *frame, const char *name) {
  void *note = y4m2_find_note(frame, name);
  if (!note) die("Can't find note %s", name);
  return note;
}

static y4m2_note *_clone_note(const y4m2_note *src) {
  if (!src) return NULL;
  y4m2_note *dst = alloc(sizeof(y4m2_note));
  dst->name = sstrdup(src->name);
  dst->v = _retain_value(src->v);
  dst->next = _clone_note(src->next);
  return dst;
}

void y4m2_copy_notes(y4m2_frame *dst, const y4m2_frame *src) {
  y4m2_remove_notes(dst);
  dst->notes = _clone_note(src->notes);
}

int y4m2_has_notes(const y4m2_frame *frame) {
  return !!frame->notes;
}

static char *is_word(char *buf, const char *match) {
  size_t l = strlen(match);
  if (strlen(buf) >= l && memcmp(buf, match, l) == 0 && buf[l] <= ' ')
    return buf + l;
  return NULL;
}

void y4m2__parse_parms(y4m2_parameters *parms, char *buf) {
  char name[2];
  for (;;) {
    while (*buf == ' ') buf++;
    if (*buf < ' ') break;
    name[0] = *buf++;
    name[1] = '\0';
    char *vp = buf;
    while (*buf > ' ') buf++;
    char t = *buf;
    *buf = '\0';
    y4m2_set_parm(parms, name, vp);
    *buf = t;
  }
}

void y4m2__format_parms(FILE *out, const y4m2_parameters *parms) {
  for (int i = 0; i < Y4M2_PARMS; i++)
    if (parms->parm[i])
      fprintf(out, " %c%s", Y4M2_FIRST + i, parms->parm[i]);
}

static double _frame_duration(y4m2_parameters *parms) {
  const char *rate = y4m2_get_parm(parms, "F");
  char *ep;

  if (!rate) return 0;

  double num = strtod(rate, &ep);
  if (ep == rate || *ep != ':') die("Missing colon in %s", rate);
  double den = strtod(ep + 1, &ep);
  if (*ep) die("Bad rate: %s", rate);
  return den / num; /* wrong way round */
}

static void check_frames(int *allocated) {
  int delta = y4m2__frames_allocated - *allocated;
  if (delta) {
    int change = abs(delta);
    log_debug("%d frame%s %s (total %d)",
              change, (change == 1 ? " was" : "s were"),
              (delta < 0 ? "freed" : "allocated"),
              y4m2__frames_allocated);
    *allocated = y4m2__frames_allocated;
  }
}

int y4m2_parse(FILE *in, y4m2_output *out) {
  size_t buf_size = 0;
  char *buf = NULL;
  y4m2_parameters *global = NULL;
  uint64_t sequence = 0;
  double elapsed = 0;
  int frames_allocated = y4m2__frames_allocated;

  for (;;) {
    int c = getc(in);
    unsigned pos = 0;
    for (;;) {
      if (pos == buf_size) {
        buf_size *= 2;
        if (buf_size < 1024) buf_size = 1024;
        char *nb = realloc(buf, buf_size);
        if (NULL == nb) abort();
        buf = nb;
      }
      if (c == EOF) goto done;
      if (c < ' ') {
        buf[pos++] = '\0';
        break;
      }
      buf[pos++] = c;
      c = getc(in);
    }
    if (c == EOF) break;

    char *tail;
    if (tail = is_word(buf, tag[Y4M2_START]), tail) {
      if (global) y4m2_free_parms(global);
      global = y4m2_new_parms();
      y4m2__parse_parms(global, tail);
      y4m2_emit_start(out, global);
      check_frames(&frames_allocated);
    }
    else if (tail = is_word(buf, tag[Y4M2_FRAME]), tail) {
      y4m2_parameters *parms = y4m2_new_parms();
      y4m2__parse_parms(parms, tail);

      y4m2_parameters *merged = y4m2_clone_parms(global);
      y4m2_merge_parms(merged, parms);

      y4m2_frame *frame = y4m2_new_frame(merged);
      frame->sequence = sequence++;
      frame->elapsed = elapsed += _frame_duration(merged);
      size_t got = fread(frame->buf, 1, frame->i.size, in);
      if (got != frame->i.size) die("Short read");
      y4m2_emit_frame(out, parms, frame);
      y4m2_release_frame(frame);
      y4m2_free_parms(parms);
      y4m2_free_parms(merged);
      check_frames(&frames_allocated);
    }
    else {
      die("Bad stream");
    }
  }

done:

  y4m2_emit_end(out);
  check_frames(&frames_allocated);

  return 0;
}

int y4m2_emit_start(y4m2_output *out, const y4m2_parameters *parms) {
  out->cb(Y4M2_START, parms, NULL, out->ctx);
  return 0;
}

int y4m2_emit_frame(y4m2_output *out, const y4m2_parameters *parms, y4m2_frame *frame) {
  out->cb(Y4M2_FRAME, parms, frame, out->ctx);
  return 0;
}

int y4m2_emit_end(y4m2_output *out) {
  out->cb(Y4M2_END, NULL, NULL, out->ctx);
  y4m2_free_output(out);
  return 0;
}

int y4m2_emit(y4m2_output *out, y4m2_reason reason,
              const y4m2_parameters *parms,
              y4m2_frame *frame) {
  switch (reason) {
  case Y4M2_START:
    return y4m2_emit_start(out, parms);
  case Y4M2_FRAME:
    return y4m2_emit_frame(out, parms, frame);
  case Y4M2_END:
    return y4m2_emit_end(out);
  default:
    return -1;
  }
}

y4m2_output *y4m2_output_next(y4m2_callback cb, void *ctx) {
  y4m2_output *o = alloc(sizeof(y4m2_output));
  o->cb = cb;
  o->ctx = ctx;
  return o;
}

static void _file_callback(y4m2_reason reason, const y4m2_parameters *parms, y4m2_frame *frame, void *ctx) {
  FILE *fl = (FILE *) ctx;

  switch (reason) {

  case Y4M2_START:
    fputs(tag[Y4M2_START], fl);
    y4m2__format_parms(fl, parms);
    fputc(0x0A, fl);
    break;

  case Y4M2_FRAME:
    y4m2_tell_me_about_stride(frame);
    fputs(tag[Y4M2_FRAME], fl);
    y4m2__format_parms(fl, parms);
    fputc(0x0A, fl);
    fwrite(frame->buf, 1, frame->i.size, fl);
    break;

  case Y4M2_END:
    break;
  }
}

y4m2_output *y4m2_output_file(FILE *out) {
  return y4m2_output_next(_file_callback, out);
}

static void _null_callback(y4m2_reason reason, const y4m2_parameters *parms, y4m2_frame *frame, void *ctx) {
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

y4m2_output *y4m2_output_null(void) {
  return y4m2_output_next(_null_callback, NULL);
}

void y4m2_free_output(y4m2_output *out) {
  free(out);
}

static unsigned y4m2__log2(unsigned x) {
  unsigned shift = 0;
  while (x > (1u << shift)) shift++;
  return shift;
}

void y4m2__plane_map(const y4m2_frame *in,
                     unsigned xs[Y4M2_N_PLANE],
                     unsigned ys[Y4M2_N_PLANE]) {

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    xs[p] = y4m2__log2(in->i.plane[p].xs);
    ys[p] = y4m2__log2(in->i.plane[p].ys);
  }
}

/* colourspace */

size_t y4m2_frame_to_float(const y4m2_frame *in, colour_floats *out) {
  unsigned xs[Y4M2_N_PLANE], ys[Y4M2_N_PLANE];
  unsigned width = in->i.width;
  unsigned height = in->i.height;

  y4m2__plane_map(in, xs, ys);

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    unsigned stride = in->i.plane[p].stride;
    for (unsigned y = 0; y < height; y++) {
      for (unsigned x = 0; x < width; x++) {
        out[y * width + x].c[p] = in->plane[p][(y >> ys[p]) * stride + (x >> xs[p])];
      }
    }
  }

  return width * height;
}

void y4m2_float_to_frame(const colour_floats *in, y4m2_frame *out) {
  unsigned xs[Y4M2_N_PLANE], ys[Y4M2_N_PLANE];
  unsigned width = out->i.width;
  unsigned height = out->i.height;

  y4m2__plane_map(out, xs, ys);

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    unsigned stride = out->i.plane[p].stride;
    unsigned plw = width >> xs[p];
    unsigned plh = height >> ys[p];
    unsigned pxw = out->i.plane[p].xs;
    unsigned pxh = out->i.plane[p].ys;
    double area = pxw * pxh;

    for (unsigned y = 0; y < plh; y++) {
      for (unsigned x = 0; x < plw; x++) {
        double sum = 0;

        for (unsigned yy = 0; yy < pxh; yy++)
          for (unsigned xx = 0; xx < pxw; xx++)
            sum += in[((y << ys[p]) + yy) * width + (x << xs[p]) + xx].c[p];

        double sample = sum / area;
        if (sample < 0) sample = 0;
        if (sample > 255) sample = 255;
        out->plane[p][y * stride + x] = (uint8_t)sample;
      }
    }
  }
}

static void _draw_point(y4m2_frame *frame, int pl, int x, int y, int v) {
  int xx = x / frame->i.plane[pl].xs;
  int yy = y / frame->i.plane[pl].ys;
  int w = frame->i.width / frame->i.plane[pl].xs;
  int h = frame->i.height / frame->i.plane[pl].ys;
  int stride = frame->i.plane[pl].stride;

  if (xx >= 0 && xx < w && yy >= 0 && yy < h)
    *(frame->plane[pl] + xx + yy * stride) = v;
}

void y4m2_draw_point(y4m2_frame *frame, int x, int y, int vy, int vu, int vv) {
  _draw_point(frame, Y4M2_Y_PLANE, x, y, vy);
  _draw_point(frame, Y4M2_Cb_PLANE, x, y, vu);
  _draw_point(frame, Y4M2_Cr_PLANE, x, y, vv);
}

#define TSWAP(t, x, y) do { t _t = x; x = y; y = _t; } while (0)
#define SWAP(x, y) TSWAP(int, x, y)

static void _draw_line(y4m2_frame *frame, int pl, int x0, int y0, int x1, int y1, int vv, int inclast) {
  int xx0 = x0 / frame->i.plane[pl].xs;
  int yy0 = y0 / frame->i.plane[pl].ys;
  int xx1 = x1 / frame->i.plane[pl].xs;
  int yy1 = y1 / frame->i.plane[pl].ys;

  int w = frame->i.width / frame->i.plane[pl].xs;
  int h = frame->i.height / frame->i.plane[pl].ys;
  int stride = frame->i.plane[pl].stride;

  uint8_t *base = frame->plane[pl];

  int dx = xx1 - xx0;
  int dy = yy1 - yy0;
  int acc, s, l;
  int incfirst = 1;

  if (abs(dx) > abs(dy)) {
    /* closer to horizontal */
    if (dx < 0) {
      SWAP(xx0, xx1);
      SWAP(yy0, yy1);
      SWAP(incfirst, inclast);
      dx = -dx;
      dy = -dy;
    }
    if (dy < 0) {
      s = -1;
      dy = -dy;
    }
    else {
      s = 1;
    }
    acc = (dx + dy) >> 1;
    l = dx;
    if (!incfirst) {
      l--;
      acc -= dy;
      if (acc <= 0) {
        acc += dx;
        yy0 += s;
      }
    }
    if (!inclast) {
      l--;
    }
    while (l-- >= 0) {
      if (xx0 >= 0 && xx0 < w && yy0 >= 0 && yy0 < h)
        base[ xx0 + yy0 * stride ] = vv;
      xx0++;
      acc -= dy;
      if (acc <= 0) {
        acc += dx;
        yy0 += s;
      }
    }
  }
  else {
    /* closer to vertical */
    if (dy < 0) {
      SWAP(xx0, xx1);
      SWAP(yy0, yy1);
      SWAP(incfirst, inclast);
      dx = -dx;
      dy = -dy;
    }
    if (dx < 0) {
      s = -1;
      dx = -dx;
    }
    else {
      s = 1;
    }
    acc = (dx + dy) >> 1;
    l = dy;
    if (!incfirst) {
      l--;
      acc -= dx;
      if (acc <= 0) {
        acc += dy;
        xx0 += s;
      }
    }
    if (!inclast) {
      l--;
    }
    while (l-- >= 0) {
      if (xx0 >= 0 && xx0 < w && yy0 >= 0 && yy0 < h)
        base[ xx0 + yy0 * stride ] = vv;
      yy0++;
      acc -= dx;
      if (acc <= 0) {
        acc += dy;
        xx0 += s;
      }
    }
  }
}

static int _clip(int x0, int y0, int x1, int y1, int w, int h) {

  /* corners */
  if (x0 < 0 && y0 < 0 && (x1 < 0 || y1 < 0)) return 0;
  if (x0 >= w && y0 < 0 && (x1 >= w || y1 < 0)) return 0;
  if (x1 < 0 && y0 >= h && (x1 < 0 || y1 >= h)) return 0;
  if (x1 >= w && y0 >= h && (x1 >= w || y1 >= h)) return 0;

  /* edges */
  if (x0 < 0 && x1 < 0) return 0;
  if (y0 < 0 && y1 < 0) return 0;
  if (x0 >= w && x1 >= w) return 0;
  if (y0 >= h && y1 >= h) return 0;

  return 1;
}

void y4m2_draw_line(y4m2_frame *frame, int x0, int y0, int x1, int y1, int vy, int vu, int vv) {
  if (_clip(x0, y0, x1, y1, frame->i.width, frame->i.height)) {
    _draw_line(frame, Y4M2_Y_PLANE, x0, y0, x1, y1, vy, 1);
    _draw_line(frame, Y4M2_Cb_PLANE, x0, y0, x1, y1, vu, 1);
    _draw_line(frame, Y4M2_Cr_PLANE, x0, y0, x1, y1, vv, 1);
  }
}

static int _is_window(const y4m2_frame *frame) {
  for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++)
    if (frame->i.plane[pl].stride != frame->i.width / frame->i.plane[pl].xs)
      return 1;
  return 0;
}

void y4m2_tell_me_about_stride(const y4m2_frame *frame) {
  if (_is_window(frame))
    die("To process this frame you need to know about the"
        "'stride' member in y4m2_plane_info");
}

y4m2_frame *y4m2_window(y4m2_frame *frame, int x, int y, int w, int h) {
  if (x < 0 || y < 0 || x + w > (int) frame->i.width || y + h > (int) frame->i.height)
    die("Window outside frame");

  y4m2_frame *window = alloc(sizeof(y4m2_frame));
  *window = *frame;

  window->refcnt = 1;
  window->parent = frame;

  y4m2_frame_info *wfi = &window->i;

  wfi->width = w;
  wfi->height = h;

  for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++) {
    y4m2_plane_info *wpi = &wfi->plane[pl];
    window->plane[pl] += x / wpi->xs + y / wpi->ys * wpi->stride;
  }

  return window;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
