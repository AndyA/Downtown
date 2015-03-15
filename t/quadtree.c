/* t/quadtree.c */


#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "quadtree.h"
#include "tap.h"
#include "util.h"

struct prox {
  int x, y, dist;
};

static int dist(int x0, int y0, int x1, int y1) {
  int dx = x0 - x1;
  int dy = y0 - y1;
  return dx * dx + dy * dy;
}

static void test_quadtree(void) {
  static const int width = 100;
  static const int height = 120;
  static const int grid = 4;

  quadtree *qt = quadtree_new(width, height);
  if (!ok(qt->dim == 128, "dim is power of two")) {
    diag("dim = %d", qt->dim);
  }

  unsigned tag = 0;
  for (int x = 0; x < width; x += grid) {
    for (int y = 0; y < height; y += grid) {
      quadtree_add(qt, x, y, ++tag);
    }
  }

  for (int x = 0; x < width; x += grid) {
    for (int y = 0; y < height; y += grid) {
      quadtree_point *near = quadtree_nearest(qt, x, y);
      if (!ok(near->x == x && near->y == y, "found %d, %d", x, y)) {
        diag("expected %d, %d; got %d, %d", x, y, near->x, near->y);
      }
    }
  }

  for (int x = 0; x < width - grid; x += 3) {
    for (int y = 0; y < height - grid; y += 5) {
      struct prox pt[4];

      pt[0].x = pt[2].x = grid * (x / grid);
      pt[0].y = pt[1].y = grid * (y / grid);
      pt[1].x = pt[3].x = grid * (x / grid) + grid;
      pt[2].y = pt[3].y = grid * (y / grid) + grid;

      int best = 9999;
      for (int i = 0; i < 4; i++) {
        pt[i].dist = dist(x, y, pt[i].x, pt[i].y);
        if (pt[i].dist < best) best = pt[i].dist;
      }

      quadtree_point *near = quadtree_nearest(qt, x, y);
      int ndist = dist(x, y, near->x, near->y);
      if (!ok(ndist == best, "Found best point")) {
        diag("for %d, %d", x, y);
        for (int i = 0; i < 4; i++) {
          if (pt[i].dist == best) diag("  expected %d, %d", pt[i].x, pt[i].y);
        }
        diag("  got %d, %d", near->x, near->y);
      }
    }
  }
  /*  quadtree_dump(qt, stderr);*/

  quadtree_free(qt);
}

void test_main(void) {
  test_quadtree();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
