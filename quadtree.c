/* quadtree.c */

#include <stdlib.h>

#include "util.h"
#include "quadtree.h"

static unsigned _next_power(int i) {
  for (unsigned bit = 0;; bit++) {
    int x = 1 << bit;
    if (x >= i) return x;
  }
}

quadtree *quadtree_new(int w, int h) {
  quadtree *qt = alloc(sizeof(quadtree));
  qt->w = w;
  qt->h = h;
  qt->dim = _next_power(MAX(w, h));
  return qt;
}

static void _free(quadtree_node *nd) {
  if (nd) {
    for (unsigned i = 0; i < 4; i++) _free(nd->kids[i]);
    free(nd);
  }
}

void quadtree_free(quadtree *qt) {
  _free(qt->root);
  free(qt);
}

static void _dump(quadtree_node *nd, FILE *out, unsigned depth, int x0, int y0, int x1, int y1) {
  if (nd) {
    unsigned i;

    for (i = 0; i < depth; i++) fprintf(out, "  ");
    fprintf(out, "[%d, %d, %d, %d]", x0, y0, x1, y1);
    if (nd->internal) fprintf(out, " internal");
    for (i = 0; i < nd->used; i++) fprintf(out, " <%u> [%d, %d]", nd->pt[i].tag, nd->pt[i].x, nd->pt[i].y);
    fprintf(out, "\n");

    int xm = (x0 + x1) / 2;
    int ym = (y0 + y1) / 2;

    _dump(nd->kids[0], out, depth + 1, x0, y0, xm,  ym);
    _dump(nd->kids[1], out, depth + 1, xm, y0, x1,  ym);
    _dump(nd->kids[2], out, depth + 1, x0, ym, xm,  y1);
    _dump(nd->kids[3], out, depth + 1, xm, ym, x1,  y1);
  }
}

void quadtree_dump(quadtree *qt, FILE *out) {
  fprintf(out, "size: %dx%d, dim: %d\n", qt->w, qt->h, qt->dim);
  _dump(qt->root, out, 1, 0, 0, qt->dim, qt->dim);
}

static quadtree_node *_add(quadtree_node *nd, const quadtree_point *pt, int x0, int y0, int x1, int y1) {
  if (!nd) nd = alloc(sizeof(quadtree_node));

  if (x0 == x1) die("Tree overloaded");

  if (nd->internal) {
    int xm = (x0 + x1) / 2;
    int ym = (y0 + y1) / 2;
    if (pt->x < xm && pt->y < ym)
      nd->kids[0] = _add(nd->kids[0], pt, x0, y0, xm, ym);
    else if (pt->x >= xm && pt->y < ym)
      nd->kids[1] = _add(nd->kids[1], pt, xm, y0, x1, ym);
    else if (pt->x < xm && pt->y >= ym)
      nd->kids[2] = _add(nd->kids[2], pt, x0, ym, xm, y1);
    else
      nd->kids[3] = _add(nd->kids[3], pt, xm, ym, x1, y1);
    return nd;
  }

  if (nd->used < quadtree_POINTS) {
    nd->pt[nd->used++] = *pt;
    return nd;
  }

  /* upgrade to internal node */
  nd->internal = 1;
  for (unsigned i = 0; i < nd->used; i++)
    nd = _add(nd, &nd->pt[i], x0, y0, x1, y1);
  nd->used = 0;
  return nd;
}

void quadtree_add_point(quadtree *qt, const quadtree_point *pt) {
  if (pt->x >= 0 && pt->x < qt->w && pt->y >= 0 && pt->y <= qt->h)
    qt->root = _add(qt->root, pt, 0, 0, qt->dim, qt->dim);
}

void quadtree_add(quadtree *qt, int x, int y, unsigned tag) {
  quadtree_point pt;
  pt.x = x;
  pt.y = y;
  pt.tag = tag;
  quadtree_add_point(qt, &pt);
}

quadtree_point *_nearest(quadtree_node *nd, int x, int y,
                         int x0, int y0, int x4, int y4,
                         int *best, quadtree_point **best_pt) {
  if (!nd) return NULL;

  if (nd->internal) {
    int x2 = (x0 + x4) / 2;
    int y2 = (y0 + y4) / 2;
    int x1 = (x0 + x2) / 2;
    int y1 = (x0 + x2) / 2;
    int x3 = (x2 + x4) / 2;
    int y3 = (y2 + y4) / 2;

    if (x < x3 && y < y3) _nearest(nd->kids[0], x, y, x0, y0, x2, y2, best, best_pt);
    if (x >= x1 && y < y3) _nearest(nd->kids[1], x, y, x2, y0, x4, y2, best, best_pt);
    if (x < x3 && y >= y1) _nearest(nd->kids[0], x, y, x0, y2, x2, y4, best, best_pt);
    if (x >= x1 && y >= y1) _nearest(nd->kids[1], x, y, x2, y2, x4, y4, best, best_pt);
  }

  for (unsigned i = 0; i < nd->used; i++) {
    int dx = nd->pt[i].x - x;
    int dy = nd->pt[i].y - y;
    int dist = dx * dx + dy * dy;
    if (NULL == *best_pt || dist < *best) {
      *best = dist;
      *best_pt = &nd->pt[i];
    }
  }

  return *best_pt;
}

quadtree_point *quadtree_nearest(quadtree *qt, int x, int y) {
  int best = 0;
  quadtree_point *best_pt = NULL;
  return _nearest(qt->root, x, y, 0, 0, qt->dim, qt->dim, &best, &best_pt);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
