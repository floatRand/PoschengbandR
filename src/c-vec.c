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

static int _is_valid_partition(vptr vec[], int left, int right, int pivot, vec_cmp_f f)
{
    int i;
    if (left >= right)
        return 0;
    if (left > pivot)
        return 0;
    if (right < pivot)
        return 0;
    for (i = left; i < pivot; i++)
    {
        if (f(vec[i], vec[pivot]) > 0)
            return 0;
    }
    for (i = pivot + 1; i <= right; i++)
    {
        if (f(vec[i], vec[pivot]) < 0)
            return 0;
    }
    return 1;
}

static int _partition(vptr vec[], int left, int right, vec_cmp_f f)
{
#if 0
    /* Here is CLRS's version of partition. It takes 41.5s to execute.
       Switching to the version below takes 0.68s to execute:
       time cat *.c | ./a.out > /dev/null
       where a.out is my version of sort
       I left this in here 'cause I'm puzzled by the performance discrepency ...

       BTW, time cat *.c | sort > /dev/null takes 2.23s*/
    vptr pivot = vec[right];
    int  i = left - 1;
    int  j;
    for (j = left; j < right; j++)
    {
        if (f(vec[j], pivot) <= 0)
        {
            i++;
            swap(vec, i, j);
        }
    }
    swap(vec, i+1, right);
    assert(_is_valid_partition(vec, left, right, i+1, f));
    return i+1;
#else
    vptr pivot = vec[right];
    int  i = left;
    int  j = right - 1;
    while (1)
    {
        while (i < right && f(vec[i], pivot) < 0) i++;
        while (left < j && f(vec[j], pivot) > 0) j--;

        assert(left <= i && i <= right);
        assert(left <= j && j <= right);

        if (i < j)
            swap(vec, i, j);
        else
            break;

        i++;
        j--;
    }
    swap(vec, i, right);
    assert(_is_valid_partition(vec, left, right, i, f));
    return i;
#endif
}

static int _median3_partition(vptr vec[], int left, int right, vec_cmp_f f)
{
    int center = (left + right) / 2;
    int i;

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
    i = _partition(vec, left + 1, right - 1, f);
    assert(_is_valid_partition(vec, left, right, i, f));

    return i;
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

static void _merge(vptr vec[], int p, int q, int r, vec_cmp_f f)
{
    int n1 = q - p + 1;
    int n2 = r - q;
    vptr *ls = malloc(n1 * sizeof(vptr));
    vptr *rs = malloc(n2 * sizeof(vptr));
    int i1 = 0, i2 = 0, i;

    for (i1 = 0; i1 < n1; i1++)
        ls[i1] = vec[p + i1];
    for (i2 = 0; i2 < n2; i2++)
        rs[i2] = vec[q + 1 + i2];

    i1 = 0; i2 = 0;
    for (i = p; i <= r; i++)
    {
        vptr l = i1 < n1 ? ls[i1] : 0;
        vptr r = i2 < n2 ? rs[i2] : 0;
        vptr c;

        if (!l) { c = r; i2++; }
        else if (!r) { c = l; i1++; }
        else if (f(l, r) <= 0) { c = l; i1++; }
        else { c = r; i2++; }

        assert (c);
        vec[i] = c;
    }

    free(ls);
    free(rs);
}

static void _merge_sort(vptr vec[], int left, int right, vec_cmp_f f)
{
    if (left < right)
    {
        int middle = (left + right) / 2;
        _merge_sort(vec, left, middle, f);
        _merge_sort(vec, middle + 1, right, f);
        _merge(vec, left, middle, right, f);
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

void vec_quick_sort(vec_ptr vec, vec_cmp_f f)
{
    _quick_sort(vec->objs, 0, vec->len - 1, f);
    assert(_is_sorted(vec, f));
}

void vec_merge_sort(vec_ptr vec, vec_cmp_f f)
{
    _merge_sort(vec->objs, 0, vec->len - 1, f);
    assert(_is_sorted(vec, f));
}

void vec_sort(vec_ptr vec, vec_cmp_f f)
{
    vec_quick_sort(vec, f);
}

/* Notes on Sorting:
   [1] Median of 3 Partioning is crucial if data is already sorted. A simple test
       of double sorting some files takes 86ms with median of 3, 10,552ms with naive
       partitioning! Note: ang_sort uses naive partitioning.
   [2] Setting the insertion sort cutoff to 20 seems just fine. I took this advice
       from Skiena p238 even though 20 seemed high at first glance.
   [3] Swapping in _partition seems the best tuning opportunity. A naive version
       from CLRS is *much* slower than what ang_sort uses, so I switched, although
       it seems that ang_sort might be reading out of bounds on occasion.
   [4] Quick Sort took me all day to write and debug. Merge Sort took about 10 minutes
       and is almost is fast (w/in 5% or 10%). Double sorting is actually faster with Merge Sort!
*/
