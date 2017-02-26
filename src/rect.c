#include "angband.h"

#include <assert.h>

point_t point(int x, int y)
{
    point_t p;
    p.x = x;
    p.y = y;
    return p;
}
point_t point_add(point_t p1, point_t p2)
{
    return point(
        p1.x + p2.x,
        p1.y + p2.y
    );
}
point_t point_subtract(point_t p1, point_t p2)
{
    return point(
        p1.x - p2.x,
        p1.y - p2.y
    );
}
int point_compare(point_t p1, point_t p2)
{
    if (p1.y < p2.y)
        return -1;
    if (p1.y > p2.y)
        return 1;
    if (p1.x < p2.x)
        return -1;
    if (p1.x > p2.x)
        return 1;
    return 0;
}

/* Trivial rectangle utility to make code a bit more readable */
rect_t rect(int x, int y, int cx, int cy)
{
    rect_t r;
    r.x = x;
    r.y = y;
    r.cx = cx;
    r.cy = cy;
    return r;
}

rect_t rect_invalid(void)
{
    return rect(0, 0, 0, 0);
}

point_t rect_topleft(rect_t r)
{
    return point(r.x, r.y);
}

point_t rect_center(rect_t r)
{
    return point(r.x + r.cx/2, r.y + r.cy/2);
}

bool rect_is_valid(rect_t r)
{
    return r.cx > 0 && r.cy > 0;
}

bool rect_contains_pt(rect_t r, int x, int y)
{
    return rect_is_valid(r)
        && (r.x <= x && x < r.x + r.cx)
        && (r.y <= y && y < r.y + r.cy);
}

bool rect_contains(rect_t r1, rect_t r2)
{
    return rect_is_valid(r1)
        && rect_is_valid(r2)
        && rect_contains_pt(r1, r2.x, r2.y)
        && rect_contains_pt(r1, r2.x + r2.cx - 1, r2.y + r2.cy - 1);
}

rect_t rect_intersect(rect_t r1, rect_t r2)
{
    rect_t result = {0};

    if (rect_is_valid(r1) && rect_is_valid(r2))
    {
        int left = MAX(r1.x, r2.x);
        int right = MIN(r1.x + r1.cx, r2.x + r2.cx);
        int top = MAX(r1.y, r2.y);
        int bottom = MIN(r1.y + r1.cy, r2.y + r2.cy);
        int cx = right - left;
        int cy = bottom - top;

        if (cx > 0 && cy > 0)
            result = rect(left, top, cx, cy);
    }
    return result;
}

rect_t rect_translate(rect_t r, int dx, int dy)
{
    rect_t result = {0};
    if (rect_is_valid(r))
        result = rect(r.x + dx, r.y + dy, r.cx, r.cy);
    return result;
}

int rect_area(rect_t r)
{
    int result = 0;
    if (rect_is_valid(r))
        result = r.cx * r.cy;
    return result;
}

/************************************************************************
 * Coordinate Transforms
 ***********************************************************************/
transform_ptr transform_alloc(int which, rect_t src)
{
    transform_ptr result = malloc(sizeof(transform_t));

    /* avoid SIGSEGV */
    if (src.cx > MAX_HGT - 2)
        which &= ~01;

    result->which = which;
    result->src = src;
    if (which & 01) /* odd number of rotations: (w,h) -> (h,w) */
        result->dest = rect(0, 0, src.cy, src.cx);
    else
        result->dest = rect(0, 0, src.cx, src.cy);

    /* force symmetry wrt center ... hmmm ... */
    if (which)
    {
        if (result->dest.cx % 2 == 0) result->dest.cx++;
        if (result->dest.cy % 2 == 0) result->dest.cy++;
    }
    return result;
}
transform_ptr transform_alloc_random(rect_t src)
{
    int which = 0;
    /* how many rotations? (0 .. 3) */
    if (one_in_(2) && src.cx <= MAX_HGT - 2 && src.cy*100/src.cx > 70 )
        which |= 01;
    if (one_in_(2))
        which |= 02;
    /* how many flips? (0 .. 1) */
    if (one_in_(2))
        which |= 04;
    return transform_alloc(which, src);
}

point_t transform_point(transform_ptr xform, point_t p)
{
    int x,y,i;
    point_t src_center = rect_center(xform->src);
    point_t dest_center = rect_center(xform->dest);
    point_t result = {0};

    assert(rect_contains_pt(xform->src, p.x, p.y));

    /* convert from screen mapping mode to a center of src rect 
     * coordinate system, inverting the sense of the y-axis */
    x = p.x - src_center.x;
    y = src_center.y - p.y;

    /* transform by
     * [1] rotating counter clockwise: (x,y) -> (-y,x) */
    for (i = 0; i < (xform->which & 03); i++)
    {
        int tmp = x;
        x = -y;
        y = tmp;
    }
    /* [2] flipping: (x,y) -> (-x,y) */
    if (xform->which & 04)
        x = -x;

    /* convert to destination rect using screen mapping coordinates
     * suitable for cave[y][x] access. */
    result.x = dest_center.x + x;
    result.y = dest_center.y - y;

    assert(rect_contains_pt(xform->dest, result.x, result.y));
    return result;
}

void transform_free(transform_ptr x)
{
    if (x) free(x);
}
