#include "angband.h"

#include <assert.h>

struct inv_s
{
    int     max;
    int     flags;
    vec_ptr objects; /* sparse ... grows as needed (up to max+1 if max is set) */
};

/* Slots: We are assuming slot <= 26 */
char slot_label(slot_t slot)
{
    assert(slot <= 26);
    if (slot) return slot - 1 + 'a';
    return ' ';
}

slot_t label_slot(char label)
{
    assert('a' <= label && label <= 'z');
    return label - 'a' + 1;
}

/* Helpers */
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

static bool _readonly(inv_ptr inv)
{
    if (inv->flags & INV_READ_ONLY) return TRUE;
    return FALSE;
}

/* Creation */
inv_ptr inv_alloc(int max, int flags)
{
    inv_ptr result = malloc(sizeof(inv_t));
    result->max = max;
    result->flags = flags;
    result->objects = vec_alloc(free);
    return result;
}

inv_ptr inv_copy(inv_ptr src)
{
    inv_ptr result = malloc(sizeof(inv_t));
    int     i;

    result->max = src->max;
    result->flags = src->flags;
    result->objects = vec_alloc(free);

    for (i = 0; i < vec_length(src->objects); i++)
    {
        obj_ptr obj = vec_get(src->objects, i);
        if (obj)
            vec_add(result->objects, obj_copy(obj));
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
    result->flags = src->flags | INV_READ_ONLY;
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

/* Adding, Removing and Sorting
 * Note: We separate adding and combining in order to support
 * the autopicker, statistics gathering, marking objects as
 * 'touched' and other things that higher level clients need. */
static void _add_aux(inv_ptr inv, obj_ptr obj, slot_t slot)
{
    obj_ptr   copy;
    obj_loc_t loc = {0};

    assert(!_readonly(inv));
    assert(1 <= slot && slot <= inv->max);

    loc.where = inv->flags & INV_LOC_MASK;
    loc.slot = slot;

    copy = obj_copy(obj);
    copy->loc = loc;
    obj_clear_dun_info(copy);
    if (slot >= vec_length(inv->objects))
        _grow(inv, slot);
    vec_set(inv->objects, slot, copy);
    obj->number = 0;
}

slot_t inv_add(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;
    assert(!_readonly(inv));
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr old = vec_get(inv->objects, slot);
        if (!old)
        {
            _add_aux(inv, obj, slot);
            return slot;
        }
    }
    if (!inv->max || slot <= inv->max)
    {
        assert(slot == vec_length(inv->objects));
        _add_aux(inv, obj, slot);
        return slot;
    }
    return 0;
}

void inv_add_at(inv_ptr inv, obj_ptr obj, slot_t slot)
{
    _add_aux(inv, obj, slot);
}

/* This version of combine will place all of obj->number
 * into a single slot, combining only if there is room. */
slot_t inv_combine(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;

    assert(!_readonly(inv));
    assert(obj->number);

    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if ( obj_can_combine(dest, obj, inv->flags)
          && dest->number + obj->number <= OBJ_STACK_MAX )
        {
            obj_combine(dest, obj, inv->flags);
            return slot;
        }
    }
    assert(obj->number);
    return 0;
}

/* This version will combine the obj pile (if pile it be)
 * into as many slots as possible, splitting the pile in
 * the process. I think stores need this behaviour. We
 * cannot return a single slot, so we return the number
 * of items in the pile that we found room for. */
int inv_combine_ex(inv_ptr inv, obj_ptr obj)
{
    int    ct = 0;
    slot_t slot;

    assert(!_readonly(inv));
    assert(obj->number);

    /* combine obj with as many existing slots as possible */
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if (obj_can_combine(dest, obj, inv->flags))
        {
            ct += obj_combine(dest, obj, inv->flags);
            if (!obj->number) break;
        }
    }
    return ct;
}

bool inv_optimize(inv_ptr inv)
{
    slot_t slot, seek;
    bool result = FALSE;

    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr dest = vec_get(inv->objects, slot);
        if (!dest) continue;
        if (!dest->number)
        {
            vec_set(inv->objects, slot, NULL); /* free */
            dest = NULL;
            result = TRUE;
            continue;
        }
        for (seek = slot + 1; seek < vec_length(inv->objects); seek++)
        {
            obj_ptr src = vec_get(inv->objects, seek);
            if (!src) continue;
            if (obj_combine(dest, src, inv->flags))
            {
                result = TRUE;
                if (!src->number)
                    vec_set(inv->objects, seek, NULL);
            }
        }
    }
    if (inv_sort(inv)) result = TRUE;
    return result;
}

void inv_remove(inv_ptr inv, slot_t slot)
{
    obj_ptr obj;

    assert(!_readonly(inv));
    assert(slot);
    obj = inv_obj(inv, slot);
    if (obj)
        vec_set(inv->objects, slot, NULL); /* free */
}

void inv_clear(inv_ptr inv)
{
    assert(!_readonly(inv));
    vec_clear(inv->objects);
}

bool inv_sort(inv_ptr inv)
{
    vec_for_each(inv->objects, (vec_item_f)obj_clear_scratch);
    if (!vec_is_sorted(inv->objects, (vec_cmp_f)obj_cmp))
    {
        slot_t slot;
        vec_sort(inv->objects, (vec_cmp_f)obj_cmp);
        for (slot = 1; slot < vec_length(inv->objects); slot++)
        {
            obj_ptr obj = vec_get(inv->objects, slot);
            if (obj)
            {
                obj->loc.slot = slot;
                assert(obj->loc.where == (inv->flags & INV_LOC_MASK));
            }
        }
        return TRUE; /* So clients can notify the player ... */
    }
    return FALSE;
}

void inv_swap(inv_ptr inv, slot_t left, slot_t right)
{
    obj_ptr obj;

    _grow(inv, MAX(left, right)); /* force allocation of slots */
    vec_swap(inv->objects, left, right);

    obj = vec_get(inv->objects, left);
    if (obj) obj->loc.slot = left;
    obj = vec_get(inv->objects, right);
    if (obj) obj->loc.slot = right;
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
    assert(!_readonly(inv));
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

