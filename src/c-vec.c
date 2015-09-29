#include "c-vec.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct vec_s
{
    vptr      *objs;
    int        size;
    int        len;
    vec_free_f free;
};

static void _grow(vec_ptr vec, int size)
{
    if (size > vec->size)
    {
        vptr   *old_objs = vec->objs;
        int     i;

        vec->size = vec->size * 2;
        if (vec->size < size)
            vec->size = size;

        vec->objs = malloc(vec->size * sizeof(vptr));
        memset(vec->objs, 0, vec->size * sizeof(vptr)); /* unnecessary */

        for (i = 0; i < vec->len; i++)
            vec->objs[i] = old_objs[i];

        free(old_objs);
    }
}

vec_ptr vec_alloc(vec_free_f free)
{
    vec_ptr result = malloc(sizeof(vec_t));
    result->objs = 0;
    result->size = 0;
    result->len = 0;
    result->free = free;
    return result;
}

void vec_free(vec_ptr vec)
{
    vec_clear(vec);
    free(vec);
}

void vec_add(vec_ptr vec, vptr obj)
{
    int i = vec->len;
    
    if (i >= vec->size)
        _grow(vec, i + 1);

    vec->len++;
    vec->objs[i] = obj;
}

void vec_clear(vec_ptr vec)
{
    if (vec->len)
    {
        if (vec->free)
        {
            int i;
            for (i = 0; i < vec->len; i++)
                vec->free(vec->objs[i]);
        }
        memset(vec->objs, 0, vec->size * sizeof(vptr)); /* unnecessary */
        vec->len = 0;
    }
}

vptr vec_get(vec_ptr vec, int i)
{
    vptr res = 0;
    assert(i >= 0 && i < vec->len);
    if (i < vec->len)
    {
        assert(vec->objs);
        res = vec->objs[i];
    }
    return res;
}

void vec_set(vec_ptr vec, int i, vptr obj)
{
    assert(i >= 0 && i < vec->len);
    if (i < vec->len)
    {
        assert(vec->objs);
        vec->objs[i] = obj;
    }
}

int vec_length(vec_ptr vec)
{
    return vec->len;
}

void vec_for_each(vec_ptr vec, vec_item_f f)
{
    int i;
    for (i = 0; i < vec->len; i++)
        f(vec->objs[i]);
}

static void _insertion_sort(vptr vec[], int left, int right, vec_cmp_f f)
{
    int j;
    for (j = left + 1; j <= right; j++)
    {
        vptr key = vec[j];
        int  i = j - 1;
        while (i >= left && f(vec[i], key) > 0)
        {
            vec[i + 1] = vec[i];
            i--;
        }
        vec[i + 1] = key;
    }
}

static void swap(vptr vec[], int i, int j)
{
    vptr t = vec[i];
    vec[i] = vec[j];
    vec[j] = t;
}

static int _partition(vptr vec[], int left, int right, vec_cmp_f f)
{
    vptr pivot = vec[right];
    int  i = left-1;
    int  j = right;
    while (1)
    {
        while (f(vec[++i], pivot) < 0);
        while (f(vec[--j], pivot) > 0);

        if (i < j)
            swap(vec, i, j);
        else
            break;
    }
    swap(vec, i, right);
    return i;
}

static int _median3_partition(vptr vec[], int left, int right, vec_cmp_f f)
{
    int center = (left + right) / 2;

    /* sort <v[l], v[c], v[r]> in place */
    if (f(vec[left], vec[center]) > 0)
        swap(vec, left, center);
    if (f(vec[left], vec[right]) > 0)
        swap(vec, left, right);
    if (f(vec[center], vec[right]) > 0)
        swap(vec, center, right);

    /* v[c] is the median, put it in v[r-1] */
    swap(vec, center, right - 1);

    /* we know v[l] <= v[r-1] <= v[r] so we can shorten our call to partition */
    return _partition(vec, left + 1, right - 1, f);
}

static void _quick_sort(vptr vec[], int left, int right, vec_cmp_f f)
{
    if (right - left + 1 < 20)
        _insertion_sort(vec, left, right, f);
    else
    {
        int i = _median3_partition(vec, left, right, f);
        _quick_sort(vec, left, i - 1, f);
        _quick_sort(vec, i + 1, right, f);
    }
}

static int _is_sorted(vec_ptr vec, vec_cmp_f f)
{
    int i;
    for (i = 0; i < vec->len - 1; i++)
    {
        if (f(vec->objs[i], vec->objs[i+1]) > 0)
            return 0;
    }
    return 1;
}

void vec_sort(vec_ptr vec, vec_cmp_f f)
{
    _quick_sort(vec->objs, 0, vec->len - 1, f);
    assert(_is_sorted(vec, f));
}

