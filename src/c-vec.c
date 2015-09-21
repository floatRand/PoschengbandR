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
