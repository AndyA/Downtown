/* yuv4mpeg2.c */

#include <stdlib.h>
#include <string.h>

#include "colour.h"
#include "framework.h"
#include "tap.h"
#include "util.h"
#include "yuv4mpeg2.h"

#define countof(x) ((int)(sizeof(x)/sizeof((x)[0])))

static void test_parms(void) {
  y4m2_parameters *p = y4m2_new_parms();

  ok(p != NULL, "y4m2_new_parms");

  y4m2_set_parm(p, "W", "1920");
  y4m2_set_parm(p, "H", "1080");

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(NULL == y4m2_get_parm(p, "a"), "out of range");
  ok(NULL == y4m2_get_parm(p, "AAAA"), "too long");
  ok(NULL == y4m2_get_parm(p, "Z"), "missing");

  y4m2_parameters *p2 = y4m2_clone_parms(p);

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(!strcmp("1920", y4m2_get_parm(p2, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p2, "H")), "get H");

  y4m2_set_parm(p, "W", "1280");
  y4m2_set_parm(p2, "H", "1024");

  ok(!strcmp("1280", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(!strcmp("1920", y4m2_get_parm(p2, "W")), "get W");
  ok(!strcmp("1024", y4m2_get_parm(p2, "H")), "get H");

  y4m2_free_parms(p);
  y4m2_free_parms(p2);
}

static void test_adjust_parms(void) {
  y4m2_parameters *p = y4m2_new_parms();

  ok(p != NULL, "y4m2_new_parms");

  y4m2_set_parm(p, "W", "1920");
  y4m2_set_parm(p, "H", "1080");
  y4m2_set_parm(p, "A", "1:1");

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get original W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get original H");
  ok(!strcmp("1:1", y4m2_get_parm(p, "A")), "get original A");

  y4m2_parameters *p2 = y4m2_adjust_parms(p, "W%u H%u", 1280, 720);

  ok(!strcmp("1280", y4m2_get_parm(p2, "W")), "get adjusted W");
  ok(!strcmp("720", y4m2_get_parm(p2, "H")), "get adjusted H");
  ok(!strcmp("1:1", y4m2_get_parm(p2, "A")), "get adjusted A");

  y4m2_free_parms(p);
  y4m2_free_parms(p2);
}

static void test_parse(void) {
  y4m2_parameters *p = y4m2_new_parms();

  char pstr[] = "W1920 H1080 A1:1 Ip F25:1 C420\n";

  y4m2__parse_parms(p, pstr);

  ok(!strcmp("1920", y4m2_get_parm(p, "W")), "get W");
  ok(!strcmp("1080", y4m2_get_parm(p, "H")), "get H");
  ok(!strcmp("1:1", y4m2_get_parm(p, "A")), "get A");
  ok(!strcmp("p", y4m2_get_parm(p, "I")), "get I");
  ok(!strcmp("25:1", y4m2_get_parm(p, "F")), "get F");
  ok(!strcmp("420", y4m2_get_parm(p, "C")), "get C");

  y4m2_frame_info info;
  y4m2_parse_frame_info(&info, p);
  ok(info.size == 1920 * 1080 * 3 / 2, "frame size");

  y4m2_free_parms(p);
}

static void random_frame(y4m2_frame *frame) {
  uint8_t *bp = frame->buf;
  for (unsigned p = 0; p < Y4M2_N_PLANE; p++) {
    unsigned min = 16;
    unsigned max = (p == Y4M2_Y_PLANE ? 235 : 240);
    for (unsigned i = 0; i < frame->i.plane[p].size; i++) {
      uint8_t sample;
      do {
        sample = rand() & 0xFF;
      }
      while (sample < min || sample > max);
      *bp++ = sample;
    }
  }
}

static void test_float(void) {
  y4m2_parameters *p = y4m2_new_parms();
  char pstr[] = "W1280 H720 A1:1 Ip F25:1 C420\n";
  y4m2__parse_parms(p, pstr);

  y4m2_frame *frame = y4m2_new_frame(p);

  size_t fsize = frame->i.width * frame->i.height;
  colour_floats *ff = jd_alloc(sizeof(colour_floats) * fsize);

  random_frame(frame);
  y4m2_frame *tmp = y4m2_like_frame(frame);

  y4m2_frame_to_float(frame, ff);
  y4m2_float_to_frame(ff, tmp);

  ok(memcmp(tmp->buf, frame->buf, frame->i.size) == 0,
     "frame_to_float, float_to_frame");

  jd_free(ff);
  y4m2_release_frame(tmp);
  y4m2_release_frame(frame);
  y4m2_free_parms(p);
}

static unsigned free_called = 0;
static void my_free(void *p) {
  free_called++;
  free(p);
}

static void test_notes(void) {
  y4m2_parameters *p = y4m2_new_parms();
  char pstr[] = "W1280 H720 A1:1 Ip F25:1 C420\n";
  y4m2__parse_parms(p, pstr);

  y4m2_frame *frame = y4m2_new_frame(p);

  ok(y4m2_find_note(frame, "a.note") == NULL, "NULL for missing note");

  y4m2_set_note(frame, "a.note", "This is a message", NULL);
  y4m2_set_note(frame, "b.note", "This is another message", NULL);

  void *a_note = y4m2_find_note(frame, "a.note");
  ok(a_note && !strcmp(a_note, "This is a message"), "a.note found");

  void *b_note = y4m2_find_note(frame, "b.note");
  ok(b_note && !strcmp(b_note, "This is another message"), "b.note found");

  y4m2_set_note(frame, "a.note", "Everything changes", NULL);

  a_note = y4m2_find_note(frame, "a.note");
  ok(a_note && !strcmp(a_note, "Everything changes"), "a.note updated");

  b_note = y4m2_find_note(frame, "b.note");
  ok(b_note && !strcmp(b_note, "This is another message"), "b.note unchanged");

  free_called = 0;
  y4m2_set_note(frame, "b.note", sstrdup("Allocated"), my_free);

  ok(free_called == 0, "free not called yet");
  b_note = y4m2_find_note(frame, "b.note");
  ok(b_note && !strcmp(b_note, "Allocated"), "b.note changed");

  y4m2_set_note(frame, "b.note", NULL, NULL);
  ok(!y4m2_find_note(frame, "b.note"), "b.note deleted");
  ok(free_called == 1, "which called free");

  a_note = y4m2_find_note(frame, "a.note");
  ok(a_note && !strcmp(a_note, "Everything changes"), "a.note still there");

  free_called = 0;
  y4m2_set_note(frame, "c.note", sstrdup("Clone"), my_free);
  y4m2_frame *frame2 = y4m2_clone_frame(frame);
  y4m2_copy_notes(frame2, frame);

  y4m2_release_frame(frame);

  ok(free_called == 0, "not freed yet");

  a_note = y4m2_find_note(frame2, "a.note");
  ok(a_note && !strcmp(a_note, "Everything changes"), "a.note cloned");

  void *c_note = y4m2_find_note(frame2, "c.note");
  ok(c_note && !strcmp(c_note, "Clone"), "c.note cloned");

  y4m2_release_frame(frame2);

  ok(free_called == 1, "freed when clone is destroyed");
}

static void check_corners(const char *desc, const y4m2_frame *frame, const int *col) {
  for (int x = 0; x < (int) frame->i.width; x += frame->i.width - 1) {
    for (int y = 0; y < (int) frame->i.height; y += frame->i.height - 1) {
      for (unsigned pl = 0; pl < Y4M2_N_PLANE; pl++) {

        int xx = x / frame->i.plane[pl].xs;
        int yy = y / frame->i.plane[pl].ys;

        uint8_t got = frame->plane[pl][xx + yy * frame->i.plane[pl].stride];
        int want = col[pl];

        if (!ok(want == got, "%s: point[%d, %d, %u] == %d?", desc, xx, yy, pl, want)) {
          diag("wanted %3d", want);
          diag("   got %3d", got);
        }
      }
    }
  }
}

typedef void (*drawfunc)(y4m2_frame *frame, const int *col);
typedef y4m2_frame *(*windfunc)(y4m2_frame *frame);

static void draw_corners(y4m2_frame *frame, const int *col) {
  y4m2_draw_point(frame, 0, 0, col[0], col[1], col[2]);
  y4m2_draw_point(frame, frame->i.width - 1, 0, col[0], col[1], col[2]);
  y4m2_draw_point(frame, 0, frame->i.height - 1, col[0], col[1], col[2]);
  y4m2_draw_point(frame, frame->i.width - 1, frame->i.height - 1, col[0], col[1], col[2]);
}

static void draw_cross(y4m2_frame *frame, const int *col) {
  y4m2_draw_line(frame, 0, 0, frame->i.width - 1, frame->i.height - 1, col[0], col[1], col[2]);
  y4m2_draw_line(frame, 0, frame->i.height - 1, frame->i.width - 1, 0, col[0], col[1], col[2]);
}

static void draw_reversed_cross(y4m2_frame *frame, const int *col) {
  y4m2_draw_line(frame, frame->i.width - 1, frame->i.height - 1, 0, 0, col[0], col[1], col[2]);
  y4m2_draw_line(frame, frame->i.width - 1, 0, 0, frame->i.height - 1, col[0], col[1], col[2]);
}

static y4m2_frame *_window(y4m2_frame *window, const y4m2_frame *frame, int x, int y, int w, int h) {
  if (w < 4 || h < 4) return NULL;
  return y4m2_window(window, frame, x, y, w, h);
}

static y4m2_frame *wind_none(y4m2_frame *window, y4m2_frame *frame) {
  (void) window;
  return frame;
}

static y4m2_frame *wind_inset_4(y4m2_frame *window, y4m2_frame *frame) {
  return _window(window, frame, 4, 4, frame->i.width - 8, frame->i.height - 8);
}

static void test_drawing(void) {
  static const char *spec[] = {
    "W100 H80 A1:1 C%s",
    "W16 H16 A1:1 C%s",
    "W1000 H4 A1:1 C%s",
    "W4 H1000 A1:1 C%s",
    "W4 H4 A1:1 C%s"
  };

  static const char *chans[] = { "420", "422", "444" };

  static struct {
    const char *name;
    drawfunc df;
  } draw[] = {
    { "corners", draw_corners},
    { "cross", draw_cross},
    { "reversed cross", draw_reversed_cross},
  };

  static struct {
    const char *name;
    windfunc wf;
  } drill[] = {
    {"no window", wind_none},
    {"inset 4", wind_inset_4}
  };

  for (int i = 0; i < countof(spec); i++) {
    for (int j = 0; j < countof(chans); j++) {
      char *cfg = ssprintf(spec[i], chans[j]);
      y4m2_parameters *parms = y4m2_adjust_parms(NULL, "%s", cfg);
      for (int k = 0; k < countof(draw); k++) {
        const int col[] = { 111, 87, 13 };
        for (int type = 0; type < countof(drill); type++) {
          char *desc = ssprintf("%s in {%d, %d, %d} on {%s} frame (%s)",
                                draw[k].name, col[0], col[1], col[2], cfg,
                                drill[type].name);

          y4m2_frame *frame = y4m2_new_frame(parms);
          y4m2_frame window;
          y4m2_frame *target = drill[type].wf(&window, frame);

          if (target) {
            int is_window = target == &window;
            ok(!!is_window == !!target->is_window,
               "%s: is_window %sset", desc, is_window ? "" : "not ");
            draw[k].df(target, col);
            check_corners(desc, target, col);
          }

          y4m2_release_frame(frame);

          free(desc);
        }
      }
      y4m2_free_parms(parms);
      free(cfg);
    }
  }
}

void test_main(void) {
  test_parms();
  test_adjust_parms();
  test_parse();
  test_float();
  test_notes();
  test_drawing();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
