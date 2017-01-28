#include "angband.h"

#include <assert.h>

struct inv_s
{
    int     max;
    bool    readonly;
    int     options;
    vec_ptr objects; /* sparse ... grows as needed (up to max+1 if max is set) */
};

/* Helpers */
static obj_ptr _copy(obj_ptr obj)
{
    obj_ptr copy = malloc(sizeof(object_type));
    assert(obj);
    *copy = *obj;
    return copy;
}

static void _grow(inv_ptr inv, slot_t slot)
{
    slot_t i;
    assert(slot);
    assert(!inv->max || slot <= inv->max);
    if (slot >= vec_length(inv->objects))
    {
        for (i = vec_length(inv->objects); i <= slot; i++)
            vec_add(inv->objects, NULL);
        assert(slot == vec_length(inv->objects) - 1);
    }
}

/* Creation */
inv_ptr inv_alloc(int max, int options)
{
    inv_ptr result = malloc(sizeof(inv_t));
    result->max = max;
    result->options = options;
    result->readonly = FALSE;
    result->objects = vec_alloc(free);
    return result;
}

inv_ptr inv_copy(inv_ptr src)
{
    inv_ptr result = malloc(sizeof(inv_t));
    int     i;

    result->max = src->max;
    result->options = src->options;
    result->readonly = FALSE;
    result->objects = vec_alloc(free);

    for (i = 0; i < vec_length(src->objects); i++)
    {
        obj_ptr obj = vec_get(src->objects, i);
        if (obj)
            vec_add(result->objects, _copy(obj));
        else
            vec_add(result->objects, NULL);
    }
    return result;
}

inv_ptr inv_filter(inv_ptr src, obj_p p)
{
    inv_ptr result = malloc(sizeof(inv_t));
    int     i;

    result->max = src->max;
    result->options = src->options;
    result->readonly = TRUE;
    result->objects = vec_alloc(NULL); /* src owns the objects! */

    for (i = 0; i < vec_length(src->objects); i++)
    {
        obj_ptr obj = vec_get(src->objects, i);

        if (!p || p(obj))
            vec_add(result->objects, obj);
        else
            vec_add(result->objects, NULL);
    }
    return result;
}

void inv_free(inv_ptr inv)
{
    vec_free(inv->objects);
    inv->objects = NULL;
    free(inv);
}

/* Adding, Removing and Sorting */
slot_t inv_add(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;
    assert(!inv->readonly);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr old = vec_get(inv->objects, slot);
        if (!old)
        {
            vec_set(inv->objects, slot, _copy(obj));
            return slot;
        }
    }
    if (!inv->max || slot <= inv->max)
    {
        assert(slot == vec_length(inv->objects));
        vec_add(inv->objects, _copy(obj));
        return slot;
    }
    return 0;
}

void inv_add_at(inv_ptr inv, obj_ptr obj, slot_t slot)
{
    assert(!inv->readonly);
    if (slot >= vec_length(inv->objects))
        _grow(inv, slot);
    vec_set(inv->objects, slot, _copy(obj));
}

int inv_combine(inv_ptr inv, obj_ptr obj)
{
    int    ct = 0;
    slot_t slot;

    assert(!inv->readonly);
    assert(obj->number);

    /* combine obj with as many existing slots as possible */
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if (obj_can_combine(dest, obj, inv->options))
        {
            ct += obj_combine(dest, obj, inv->options);
            if (!obj->number) break;
        }
    }
    /* add the leftovers to a new slot */
    if (obj->number)
    {
        if (inv_add(inv, obj))
        {
            ct += obj->number;
            obj->number = 0;
        }
    }
    return ct;
}

int inv_remove(inv_ptr inv, slot_t slot, int ct)
{
    int     remaining = 0;
    obj_ptr obj;

    assert(slot);
    obj = inv_obj(inv, slot);
    assert(obj);
    assert(ct > 0);
    assert(ct <= obj->number);
    assert(!inv->readonly);

    obj->number -= ct;
    remaining = obj->number;
    if (!obj->number)
        vec_set(inv->objects, slot, NULL); /* free */

    return remaining;
}

void inv_clear(inv_ptr inv)
{
    assert(!inv->readonly);
    vec_clear(inv->objects);
}

bool inv_sort(inv_ptr inv)
{
    vec_for_each(inv->objects, (vec_item_f)obj_clear_scratch);
    if (!vec_is_sorted(inv->objects, (vec_cmp_f)obj_cmp))
    {
        vec_sort(inv->objects, (vec_cmp_f)obj_cmp);
        return TRUE; /* So clients can notify the player ... */
    }
    return FALSE;
}

void inv_swap(inv_ptr inv, slot_t left, slot_t right)
{
    _grow(inv, MAX(left, right)); /* force allocation of slots */
    vec_swap(inv->objects, left, right);
}

/* Iterating, Searching and Accessing Objects (Predicates are always optional) */
obj_ptr inv_obj(inv_ptr inv, slot_t slot)
{
    assert(slot);
    if (slot >= vec_length(inv->objects))
    {
        assert(!inv->max || slot <= inv->max);
        return NULL;
    }
    return vec_get(inv->objects, slot);
}

slot_t inv_first(inv_ptr inv, obj_p p)
{
    return inv_next(inv, p, 0);
}

slot_t inv_next(inv_ptr inv, obj_p p, slot_t prev_match)
{
    int slot;
    for (slot = prev_match + 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && (!p || p(obj))) return slot;
    }
    return 0;
}

slot_t inv_last(inv_ptr inv, obj_p p)
{
    int slot;
    for (slot = vec_length(inv->objects) - 1; slot > 0; slot--)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && (!p || p(obj))) return slot;
    }
    return 0;
}

slot_t inv_find_art(inv_ptr inv, int which)
{
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && obj->name1 == which) return slot;
    }
    return 0;
}

slot_t inv_find_ego(inv_ptr inv, int which)
{
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && obj->name2 == which) return slot;
    }
    return 0;
}

slot_t inv_find_obj(inv_ptr inv, int tval, int sval)
{
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (!obj) continue;
        if (obj->tval != tval) continue;
        if (sval != SV_ANY && obj->sval != sval) continue;
        return slot;
    }
    return 0;
}

void inv_for_each(inv_ptr inv, obj_f f)
{
    int slot;
    assert(f);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
            f(obj);
    }
}

void inv_for_each_that(inv_ptr inv, obj_f f, obj_p p)
{
    int slot;
    assert(f);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (!obj) continue;
        if (p && !p(obj)) continue;
        f(obj);
    }
}

void inv_for_each_slot(inv_ptr inv, slot_f f)
{
    int slot;
    int max = inv->max ? inv->max : vec_length(inv->objects) - 1;
    assert(f);
    for (slot = 1; slot <= max; slot++)
        f(slot);
}

slot_t inv_random_slot(inv_ptr inv, obj_p p)
{
    int ct = 0;
    int slot;

    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && (!p || p(obj)))
            ct++;
    }

    if (ct)
    {
        int which = randint0(ct);
        for (slot = 1; slot < vec_length(inv->objects); slot++)
        {
            obj_ptr obj = inv_obj(inv, slot);
            if (obj && (!p || p(obj)))
            {
                if (!which) return slot;
                which--;
            }
        }
    }
    return 0;
}

/* Properties of the Entire Inventory */
int inv_weight(inv_ptr inv, obj_p p)
{
    int wgt = 0;
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && (!p || p(obj)))
            wgt += obj->weight * obj->number;
    }
    return wgt;
}

int inv_count(inv_ptr inv, obj_p p)
{
    int ct = 0;
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && (!p || p(obj)))
            ct += obj->number;
    }
    return ct;
}

int inv_count_slots(inv_ptr inv, obj_p p)
{
    int ct = 0;
    int slot;
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj && (!p || p(obj)))
            ct++;
    }
    return ct;
}

int inv_max_slots(inv_ptr inv)
{
    return inv->max;
}

/* Savefiles */
void inv_load(inv_ptr inv, savefile_ptr file)
{
    int i, ct, slot;
    assert(!inv->readonly);
    vec_clear(inv->objects);
    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        obj_ptr obj = malloc(sizeof(object_type));
        object_wipe(obj);

        slot = savefile_read_s32b(file);
        obj_load(obj, file);

        if (slot > vec_length(inv->objects))
            _grow(inv, slot);
        vec_set(inv->objects, slot, obj);
    }
}

void inv_save(inv_ptr inv, savefile_ptr file)
{
    int ct = inv_count_slots(inv, obj_exists);
    int slot;

    savefile_write_s32b(file, ct);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
        {
            savefile_write_s32b(file, slot);
            obj_save(obj, file);
            ct--;
        }
    }
    assert(ct == 0);
}

