/* quadtree.c */

#include <stdlib.h>
#include <string.h>

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
    for (i = 0; i < nd->used; i++)
      fprintf(out, " <%u> [%d, %d]%s", nd->pt[i].tag, nd->pt[i].x, nd->pt[i].y,
              (nd->pt[i].x < x0 || nd->pt[i].x >= x1 || nd->pt[i].y < y0 || nd->pt[i].y >= y1)
              ? " **** OUTSIDE PARENT ****" : ""
             );
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
  if (x0 == x1) die("Tree overloaded");

  if (!nd) nd = alloc(sizeof(quadtree_node));

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
  return _add(nd, pt, x0, y0, x1, y1);
}

int quadtree_add_point(quadtree *qt, const quadtree_point *pt) {
  qt->added++;
  if (pt->x >= 0 && pt->x < qt->w && pt->y >= 0 && pt->y <= qt->h) {
    qt->root = _add(qt->root, pt, 0, 0, qt->dim, qt->dim);
    qt->used++;
    return 1;
  }
  return 0;
}

int quadtree_add(quadtree *qt, int x, int y, unsigned tag) {
  quadtree_point pt;
  pt.x = x;
  pt.y = y;
  pt.tag = tag;
  return quadtree_add_point(qt, &pt);
}

static int _dist(int x0, int y0, int x1, int y1) {
  int dx = x0 - x1;
  int dy = y0 - y1;
  return dx * dx + dy * dy;
}

struct nearest_work {
  int best;
  const quadtree_point *best_pt;
  unsigned checked;
};

struct nearest_node {
  quadtree_node *nd;
  int x0, y0, x2, y2;
  int dist;
};

static int node_dist(const struct nearest_node *nd, int x, int y) {
  int dx = x < nd->x0 ? nd->x0 - x : x >= nd->x2 ? nd->x2 - x - 1 : 0;
  int dy = y < nd->y0 ? nd->y0 - y : y >= nd->y2 ? nd->y2 - y - 1 : 0;
  return dx * dx + dy * dy;
}

static int cmp_nn_dist(const void *a, const void *b) {
  const struct nearest_node *an = a;
  const struct nearest_node *bn = b;
  return an->dist < bn->dist ? -1 : an->dist > bn->dist ? 1 : 0;
}

static void _nearest(const quadtree_node *nd, int x, int y,
                     int x0, int y0, int x4, int y4,
                     struct nearest_work *wrk) {
  if (!nd) return;

  if (nd->internal) {
    struct nearest_node near[4];
    unsigned near_used = 0;
    unsigned i;

    int kw = (x4 - x0) / 2;
    int kh = (y4 - y0) / 2;
    for (i = 0; i < 4; i++) {
      if (!nd->kids[i]) continue;
      int dx = (i & 1) ? kw : 0;
      int dy = (i & 2) ? kh : 0;
      near[near_used].nd = nd->kids[i];
      near[near_used].x0 = x0 + dx;
      near[near_used].y0 = y0 + dy;
      near[near_used].x2 = near[near_used].x0 + kw;
      near[near_used].y2 = near[near_used].y0 + kh;
      near[near_used].dist = node_dist(&near[near_used], x, y);
      near_used++;
    }

    qsort(near, near_used, sizeof(struct nearest_node), cmp_nn_dist);

    for (i = 0; i < near_used; i++) {
      if (wrk->best_pt && wrk->best < node_dist(&near[i], x, y)) break;
      _nearest(near[i].nd, x, y, near[i].x0, near[i].y0, near[i].x2, near[i].y2, wrk);
    }
  }

  for (unsigned i = 0; i < nd->used; i++) {
    int dist = _dist(x, y, nd->pt[i].x, nd->pt[i].y);
    if (NULL == wrk->best_pt || dist < wrk->best) {
      wrk->best = dist;
      wrk->best_pt = &nd->pt[i];
    }
  }
  wrk->checked += nd->used;
}

const quadtree_point *quadtree_nearest(quadtree *qt, int x, int y) {
  struct nearest_work work;
  memset(&work, 0, sizeof(work));
  _nearest(qt->root, x, y, 0, 0, qt->dim, qt->dim, &work);
  /*  fprintf(stderr, "# checked %u\n", work.checked);*/
  return work.best_pt;
}

unsigned quadtree_used(quadtree *qt) {
  return qt->used;
}

unsigned quadtree_added(quadtree *qt) {
  return qt->added;
}

static quadtree_point *_get(quadtree_node *nd, quadtree_point *pt) {
  if (!nd) return pt;
  for (unsigned i = 0; i < 4; i++)
    pt = _get(nd->kids[i], pt);
  memcpy(pt, nd->pt, sizeof(quadtree_point) * nd->used);
  return pt + nd->used;
}

void quadtree_get(quadtree *qt, quadtree_point *pt) {
  _get(qt->root, pt);
}

static int cmp_tag(const void *a, const void *b) {
  const quadtree_point *pa = a;
  const quadtree_point *pb = b;
  return pa->tag < pb->tag ? -1 : pa->tag > pb->tag ? 1 : 0;
}

void quadtree_get_sorted(quadtree *qt, quadtree_point *pt) {
  quadtree_get(qt, pt);
  qsort(pt, qt->used, sizeof(quadtree_point), cmp_tag);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
