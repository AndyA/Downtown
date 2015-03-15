/* quadtree.h */

#ifndef QUADTREE_H_
#define QUADTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define quadtree_POINTS 20

  typedef struct {
    int x, y;
    unsigned tag;
  } quadtree_point;

  typedef struct quadtree_node quadtree_node;
  struct quadtree_node {
    quadtree_node *kids[4];
    unsigned internal;
    quadtree_point pt[quadtree_POINTS];
    size_t used;
  };

  typedef struct {
    quadtree_node *root;
    int w, h;
    int dim;
    unsigned used;
  } quadtree;

  quadtree *quadtree_new(int w, int h);
  void quadtree_free(quadtree *qt);
  void quadtree_add_point(quadtree *qt, const quadtree_point *pt);
  void quadtree_add(quadtree *qt, int x, int y, unsigned tag);
  void quadtree_dump(quadtree *qt, FILE *out);
  const quadtree_point *quadtree_nearest(quadtree *qt, int x, int y);
  unsigned quadtree_used(quadtree *qt);
  void quadtree_get(quadtree *qt, quadtree_point *pt);
  void quadtree_get_sorted(quadtree *qt, quadtree_point *pt);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
