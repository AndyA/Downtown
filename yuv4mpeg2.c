/* yuv4mpeg2.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colour.h"
#include "util.h"
#include "yuv4mpeg2.h"

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

y4m2_parameters *y4m2_adjust_parms(const y4m2_parameters *parms, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  char *spec = vssprintf(fmt, ap);
  va_end(ap);

  y4m2_parameters *np = y4m2_clone_parms(parms);
  y4m2_parameters *delta = y4m2_new_parms();

  y4m2__parse_parms(delta, spec);

  y4m2_merge_parms(np, delta);
  y4m2_free_parms(delta);

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

static void set_planes(y4m2_frame_info *info,
                       unsigned xsY, unsigned ysY,
                       unsigned xsCb, unsigned ysCb,
                       unsigned xsCr, unsigned ysCr) {
  size_t pix_size = info->width * info->height;

  info->plane[Y4M2_Y_PLANE].xs = xsY;
  info->plane[Y4M2_Y_PLANE].ys = ysY;
  info->plane[Y4M2_Y_PLANE].size = pix_size / (xsY * ysY);
  info->plane[Y4M2_Cb_PLANE].xs = xsCb;
  info->plane[Y4M2_Cb_PLANE].ys = ysCb;
  info->plane[Y4M2_Cb_PLANE].size = pix_size / (xsCb * ysCb);
  info->plane[Y4M2_Cr_PLANE].xs = xsCr;
  info->plane[Y4M2_Cr_PLANE].ys = ysCr;
  info->plane[Y4M2_Cr_PLANE].size = pix_size / (xsCr * ysCr);

  info->size = info->plane[Y4M2_Y_PLANE].size +
               info->plane[Y4M2_Cb_PLANE].size +
               info->plane[Y4M2_Cr_PLANE].size;
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
    y4m2_remove_notes(frame);
    free(frame->buf);
    free(frame);
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

static void _free_note(y4m2_note *note) {
  if (note) {
    free(note->name);
    if (note->destructor) note->destructor(note->value);
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

static y4m2_note *_set_note(y4m2_note *note, const char *name, void *value, y4m2_free_func destructor) {
  if (!note) {
    if (!value) return NULL;
    note = alloc(sizeof(y4m2_note));
    note->name = sstrdup(name);
  }
  else if (strcmp(note->name, name)) {
    note->next = _set_note(note->next, name, value, destructor);
    return note;
  }
  if (!value) {
    y4m2_note *next = note->next;
    _free_note(note);
    return next;
  }
  if (note->destructor) note->destructor(note->value);
  note->value = value;
  note->destructor = destructor;
  return note;
}

void y4m2_set_note(y4m2_frame *frame, const char *name, void *value, y4m2_free_func destructor) {
  frame->notes = _set_note(frame->notes, name, value, destructor);
}

void *y4m2_find_note(y4m2_frame *frame, const char *name) {
  for (y4m2_note *note = frame->notes; note; note = note->next)
    if (0 == strcmp(note->name, name))
      return note->value;
  return NULL;
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

int y4m2_parse(FILE *in, y4m2_output *out) {
  size_t buf_size = 0;
  char *buf = NULL;
  y4m2_parameters *global = NULL;
  uint64_t sequence = 0;

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
    }
    else if (tail = is_word(buf, tag[Y4M2_FRAME]), tail) {
      y4m2_parameters *parms = y4m2_new_parms();
      y4m2__parse_parms(parms, tail);

      y4m2_parameters *merged = y4m2_clone_parms(global);
      y4m2_merge_parms(merged, parms);

      y4m2_frame *frame = y4m2_new_frame(merged);
      frame->sequence = sequence++;
      size_t got = fread(frame->buf, 1, frame->i.size, in);
      if (got != frame->i.size) die("Short read");
      y4m2_emit_frame(out, parms, frame);
      y4m2_release_frame(frame);
      y4m2_free_parms(parms);
      y4m2_free_parms(merged);
    }
    else {
      die("Bad stream");
    }
  }

done:

  y4m2_emit_end(out);

  return 0;
}

int y4m2_emit_start(y4m2_output *out, const y4m2_parameters *parms) {
  switch (out->type) {
  case Y4M2_OUTPUT_FILE:
    fputs(tag[Y4M2_START], out->o.f);
    y4m2__format_parms(out->o.f, parms);
    fputc(0x0A, out->o.f);
    break;
  case Y4M2_OUTPUT_NEXT:
    out->o.n.cb(Y4M2_START, parms, NULL, out->o.n.ctx);
    break;
  }
  return 0;
}

int y4m2_emit_frame(y4m2_output *out, const y4m2_parameters *parms, y4m2_frame *frame) {
  switch (out->type) {
  case Y4M2_OUTPUT_FILE:
    fputs(tag[Y4M2_FRAME], out->o.f);
    y4m2__format_parms(out->o.f, parms);
    fputc(0x0A, out->o.f);
    fwrite(frame->buf, 1, frame->i.size, out->o.f);
    break;
  case Y4M2_OUTPUT_NEXT:
    out->o.n.cb(Y4M2_FRAME, parms, frame, out->o.n.ctx);
    break;
  }
  return 0;
}

int y4m2_emit_end(y4m2_output *out) {
  switch (out->type) {
  case Y4M2_OUTPUT_FILE:
    break;
  case Y4M2_OUTPUT_NEXT:
    out->o.n.cb(Y4M2_END, NULL, NULL, out->o.n.ctx);
    y4m2_free_output(out);
    break;
  }
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

y4m2_output *y4m2_output_file(FILE *out) {
  y4m2_output *o = alloc(sizeof(y4m2_output));
  o->type = Y4M2_OUTPUT_FILE;
  o->o.f = out;
  return o;
}

y4m2_output *y4m2_output_next(y4m2_callback cb, void *ctx) {
  y4m2_output *o = alloc(sizeof(y4m2_output));
  o->type = Y4M2_OUTPUT_NEXT;
  o->o.n.cb = cb;
  o->o.n.ctx = ctx;
  return o;
}

void y4m2_free_output(y4m2_output *out) {
  free(out);
}

static unsigned y4m2__log2(unsigned x) {
  unsigned shift = 0;
  while (x > (1u << shift)) shift++;
  return shift;
}

static void y4m2__plane_map(const y4m2_frame *in,
                            uint8_t *plane[Y4M2_N_PLANE],
                            unsigned xs[Y4M2_N_PLANE],
                            unsigned ys[Y4M2_N_PLANE]) {
  uint8_t *bp = in->buf;

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    plane[p] = bp;
    xs[p] = y4m2__log2(in->i.plane[p].xs);
    ys[p] = y4m2__log2(in->i.plane[p].ys);
    bp += in->i.plane[p].size;
  }
}

/* colourspace */

size_t y4m2_frame_to_float(const y4m2_frame *in, colour_floats *out) {
  uint8_t *plane[Y4M2_N_PLANE];
  unsigned xs[Y4M2_N_PLANE], ys[Y4M2_N_PLANE];
  unsigned width = in->i.width;
  unsigned height = in->i.height;

  y4m2__plane_map(in, plane, xs, ys);

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    for (unsigned y = 0; y < height; y++) {
      for (unsigned x = 0; x < width; x++) {
        out[y * width + x].c[p] =
          plane[p][(y >> ys[p]) * (width >> xs[p]) + (x >> xs[p])];
      }
    }
  }

  return width * height;
}

void y4m2_float_to_frame(const colour_floats *in, y4m2_frame *out) {
  uint8_t *plane[Y4M2_N_PLANE];
  unsigned xs[Y4M2_N_PLANE], ys[Y4M2_N_PLANE];
  unsigned width = out->i.width;
  unsigned height = out->i.height;

  y4m2__plane_map(out, plane, xs, ys);

  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
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
        plane[p][y * plw + x] = (uint8_t)sample;
      }
    }
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
