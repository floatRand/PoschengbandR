#ifndef INCLUDED_RECT_H
#define INCLUDED_RECT_H

#include "h-basic.h"

struct point_s
{
    int x, y;
};
typedef struct point_s point_t;

struct rect_s
{
    int  x,  y;
    int cx, cy;
};
typedef struct rect_s rect_t;

extern point_t point(int x, int y);
extern point_t point_add(point_t p1, point_t p2);
extern point_t point_subtract(point_t p1, point_t p2);
extern int     point_compare(point_t p1, point_t p2);

extern rect_t  rect(int x, int y, int cx, int cy);
extern rect_t  rect_invalid(void);
extern point_t rect_topleft(rect_t r);
extern point_t rect_center(rect_t r);
extern bool    rect_is_valid(rect_t r);
extern bool    rect_contains_pt(rect_t r, int x, int y);
extern bool    rect_contains(rect_t r1, rect_t r2);
extern rect_t  rect_intersect(rect_t r1, rect_t r2);
extern rect_t  rect_translate(rect_t r, int dx, int dy);
extern int     rect_area(rect_t r);

/* Coodinate transforms are a bijection from points in a src rect to points in dest rect */
struct transform_s
{
    int    which;
    rect_t src;
    rect_t dest;
};
typedef struct transform_s transform_t, *transform_ptr;

/* First, you need to make a transform ... this will calculate a default
 * dest rect for you, which you may subsequently translate if desired. */
transform_ptr transform_alloc(int which, rect_t src);
transform_ptr transform_alloc_random(rect_t src);

/* Then, you map points from the src rect to points in the dest rect.*/
point_t       transform_point(transform_ptr x, point_t p);

/* When done, free up the memory, OK? */
void          transform_free(transform_ptr x);
#endif
