#include "angband.h"

#include <assert.h>

struct inv_s
{
    int     max;
    bool    readonly;
    vec_ptr objects; /* sparse ... grows as needed (up to max+1 if max is set) */
};

bool obj_exists(obj_ptr obj)
{
    return obj ? TRUE : FALSE;
}

void obj_clear_scratch(obj_ptr obj)
{
    if (obj) obj->scratch = 0;
}

/* Creation */
inv_ptr inv_alloc(int max)
{
    inv_ptr result = malloc(sizeof(inv_t));
    result->max = max;
    result->readonly = FALSE;
    result->objects = vec_alloc(free);
    return result;
}

inv_ptr inv_filter(inv_ptr src, obj_p p)
{
    inv_ptr result = malloc(sizeof(inv_t));
    int     i;

    result->max = src->max;
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
static obj_ptr _copy(obj_ptr obj)
{
    obj_ptr copy = malloc(sizeof(object_type));
    object_copy(copy, obj);
    /* *copy = *obj; */
    return copy;
}

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

slot_t inv_combine(inv_ptr inv, obj_ptr obj)
{
    slot_t slot;
    assert(!inv->readonly);
    for (slot = 1; slot < vec_length(inv->objects); slot++)
    {
        obj_ptr old = vec_get(inv->objects, slot);
        if (!old) continue;
        if (object_similar(old, obj))
        {
            object_absorb(old, obj);
            return slot;
        }
    }
    return inv_add(inv, obj);
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

void inv_sort(inv_ptr inv)
{
    vec_for_each(inv->objects, (vec_item_f)obj_clear_scratch);
    vec_sort(inv->objects, (vec_cmp_f)obj_cmp);
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

void inv_swap(inv_ptr inv, slot_t left, slot_t right)
{
    _grow(inv, MAX(left, right)); /* force allocation of slots */
    vec_swap(inv->objects, left, right);
}

/* the next two functions belong in a (currently non-existant) obj module (obj.c) */
static int _obj_cmp_type(obj_ptr obj)
{
    if (!object_is_device(obj))
    {
        if (object_is_fixed_artifact(obj)) return 3;
        else if (obj->art_name) return 2;
        else if (object_is_ego(obj)) return 1;
    }
    return 0;
}

int obj_cmp(obj_ptr left, obj_ptr right)
{
    int left_type, right_type;
    /* Modified from object_sort_comp but the comparison is tri-valued
     * as is standard practice for compare functions. We also memoize
     * computation of obj_value for efficiency. */

    /* Empty slots sort to the end */
    if (!left && !right) return 0;
    if (!left && right) return 1;
    if (left && !right) return -1;

    assert(left && right);

    /* Hack -- readable books always come first (This fails for the Skillmaster) */
    if (left->tval == REALM1_BOOK && right->tval != REALM1_BOOK) return -1;
    if (left->tval != REALM1_BOOK && right->tval == REALM1_BOOK) return 1;

    if (left->tval == REALM2_BOOK && right->tval != REALM2_BOOK) return -1;
    if (left->tval != REALM2_BOOK && right->tval == REALM2_BOOK) return 1;

    /* Objects sort by decreasing type */
    if (left->tval < right->tval) return 1;
    if (left->tval > right->tval) return -1;

    /* Non-aware (flavored) items always come last */
    if (!object_is_aware(left)) return 1;
    if (!object_is_aware(right)) return -1;

    /* Objects sort by increasing sval */
    if (left->sval < right->sval) return -1;
    if (left->sval > right->sval) return 1;

    /* Unidentified objects always come last */
    if (!object_is_known(left)) return 1;
    if (!object_is_known(right)) return -1;

    /* Fixed artifacts, random artifacts and ego items */
    left_type = _obj_cmp_type(left);
    right_type = _obj_cmp_type(right);
    if (left_type < right_type) return -1;
    if (left_type > right_type) return 1;

    switch (left->tval)
    {
    case TV_FIGURINE:
    case TV_STATUE:
    case TV_CORPSE:
    case TV_CAPTURE:
        if (r_info[left->pval].level < r_info[right->pval].level) return -1;
        if (r_info[left->pval].level == r_info[right->pval].level && left->pval < right->pval) return -1;
        return 1;

    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        if (left->to_h + left->to_d < right->to_h + right->to_d) return -1;
        if (left->to_h + left->to_d > right->to_h + right->to_d) return 1;
        break;

    case TV_ROD:
    case TV_WAND:
    case TV_STAFF:
        if (left->activation.type < right->activation.type) return -1;
        if (left->activation.type > right->activation.type) return 1;
        if (device_level(left) < device_level(right)) return -1;
        if (device_level(left) > device_level(right)) return 1;
        break;
    }

    /* Lastly, sort by decreasing value */
    if (!left->scratch) left->scratch = obj_value(left);
    if (!right->scratch) right->scratch = obj_value(right);

    if (left->scratch < right->scratch) return 1;
    if (left->scratch > right->scratch) return -1;

    return 0;
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

/* These were pilfered from load.c and save.c. Ideally, they
 * would go in a (currently non-existent) obj module (obj.c) */
void obj_load(obj_ptr obj, savefile_ptr file)
{
    object_kind *k_ptr;
    char         buf[128];

    object_wipe(obj);

    obj->k_idx = savefile_read_s16b(file);
    k_ptr = &k_info[obj->k_idx];
    obj->tval = k_ptr->tval;
    obj->sval = k_ptr->sval;

    obj->iy = savefile_read_byte(file);
    obj->ix = savefile_read_byte(file);
    obj->weight = savefile_read_s16b(file);

    obj->number = 1;

    for (;;)
    {
        byte code = savefile_read_byte(file);
        if (code == SAVE_ITEM_DONE)
            break;

        switch (code)
        {
        case SAVE_ITEM_PVAL:
            obj->pval = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_DISCOUNT:
            obj->discount = savefile_read_byte(file);
            break;
        case SAVE_ITEM_NUMBER:
            obj->number = savefile_read_byte(file);
            break;
        case SAVE_ITEM_NAME1:
            obj->name1 = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_NAME2:
            obj->name2 = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_NAME3:
            obj->name3 = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_TIMEOUT:
            obj->timeout = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_COMBAT:
            obj->to_h = savefile_read_s16b(file);
            obj->to_d = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_ARMOR:
            obj->to_a = savefile_read_s16b(file);
            obj->ac = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_DAMAGE_DICE:
            obj->dd = savefile_read_byte(file);
            obj->ds = savefile_read_byte(file);
            break;
        case SAVE_ITEM_MULT:
            obj->mult = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_IDENT:
            obj->ident = savefile_read_byte(file);
            break;
        case SAVE_ITEM_MARKED_BYTE:
            obj->marked = savefile_read_byte(file);
            break;
        case SAVE_ITEM_MARKED:
            obj->marked = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_ART_FLAGS_0:
            obj->flags[0] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_ART_FLAGS_1:
            obj->flags[1] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_ART_FLAGS_2:
            obj->flags[2] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_ART_FLAGS_3:
            obj->flags[3] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_ART_FLAGS_4:
            obj->flags[4] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_ART_FLAGS_5:
            obj->flags[5] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_CURSE_FLAGS:
            obj->curse_flags = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_FLAGS_0:
            obj->known_flags[0] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_FLAGS_1:
            obj->known_flags[1] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_FLAGS_2:
            obj->known_flags[2] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_FLAGS_3:
            obj->known_flags[3] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_FLAGS_4:
            obj->known_flags[4] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_FLAGS_5:
            obj->known_flags[5] = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_KNOWN_CURSE_FLAGS:
            obj->known_curse_flags = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_RUNE_FLAGS:
            obj->rune = savefile_read_u32b(file);
            break;
        case SAVE_ITEM_HELD_M_IDX:
            obj->held_m_idx = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_XTRA1:
            obj->xtra1 = savefile_read_byte(file);
            break;
        case SAVE_ITEM_XTRA2:
            obj->xtra2 = savefile_read_byte(file);
            break;
        case SAVE_ITEM_XTRA3:
            obj->xtra3 = savefile_read_byte(file);
            break;
        case SAVE_ITEM_XTRA4:
            obj->xtra4 = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_XTRA5_OLD:
            obj->xtra5 = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_XTRA5:
            obj->xtra5 = savefile_read_s32b(file);
            break;
        case SAVE_ITEM_FEELING:
            obj->feeling = savefile_read_byte(file);
            break;
        case SAVE_ITEM_INSCRIPTION:
            savefile_read_cptr(file, buf, sizeof(buf));
            obj->inscription = quark_add(buf);
            break;
        case SAVE_ITEM_ART_NAME:
            savefile_read_cptr(file, buf, sizeof(buf));
            obj->art_name = quark_add(buf);
            break;
        case SAVE_ITEM_ACTIVATION:
            obj->activation.type = savefile_read_s16b(file);
            obj->activation.power = savefile_read_byte(file);
            obj->activation.difficulty = savefile_read_byte(file);
            obj->activation.cost = savefile_read_s16b(file);
            obj->activation.extra = savefile_read_s16b(file);
            break;
        case SAVE_ITEM_LEVEL:
            obj->level = savefile_read_s16b(file);
            break;
        /* default:
            TODO: Report an error back to the load routine!!*/
        }
    }
    if (object_is_device(obj))
        add_flag(obj->flags, OF_ACTIVATE);
}

void obj_save(obj_ptr obj, savefile_ptr file)
{
    savefile_write_s16b(file, obj->k_idx);
    savefile_write_byte(file, obj->iy);
    savefile_write_byte(file, obj->ix);
    savefile_write_s16b(file, obj->weight);
    if (obj->pval)
    {
        savefile_write_byte(file, SAVE_ITEM_PVAL);
        savefile_write_s16b(file, obj->pval);
    }
    if (obj->discount)
    {
        savefile_write_byte(file, SAVE_ITEM_DISCOUNT);
        savefile_write_byte(file, obj->discount);
    }
    if (obj->number != 1)
    {
        savefile_write_byte(file, SAVE_ITEM_NUMBER);
        savefile_write_byte(file, obj->number);
    }
    if (obj->name1)
    {
        savefile_write_byte(file, SAVE_ITEM_NAME1);
        savefile_write_s16b(file, obj->name1);
    }
    if (obj->name2)
    {
        savefile_write_byte(file, SAVE_ITEM_NAME2);
        savefile_write_s16b(file, obj->name2);
    }
    if (obj->name3)
    {
        savefile_write_byte(file, SAVE_ITEM_NAME3);
        savefile_write_s16b(file, obj->name3);
    }
    if (obj->timeout)
    {
        savefile_write_byte(file, SAVE_ITEM_TIMEOUT);
        savefile_write_s16b(file, obj->timeout);
    }
    if (obj->to_h || obj->to_d)
    {
        savefile_write_byte(file, SAVE_ITEM_COMBAT);
        savefile_write_s16b(file, obj->to_h);
        savefile_write_s16b(file, obj->to_d);
    }
    if (obj->to_a || obj->ac)
    {
        savefile_write_byte(file, SAVE_ITEM_ARMOR);
        savefile_write_s16b(file, obj->to_a);
        savefile_write_s16b(file, obj->ac);
    }
    if (obj->dd || obj->ds)
    {
        savefile_write_byte(file, SAVE_ITEM_DAMAGE_DICE);
        savefile_write_byte(file, obj->dd);
        savefile_write_byte(file, obj->ds);
    }
    if (obj->mult)
    {
        savefile_write_byte(file, SAVE_ITEM_MULT);
        savefile_write_s16b(file, obj->mult);
    }
    if (obj->ident)
    {
        savefile_write_byte(file, SAVE_ITEM_IDENT);
        savefile_write_byte(file, obj->ident);
    }
    if (obj->marked)
    {
        savefile_write_byte(file, SAVE_ITEM_MARKED);
        savefile_write_u32b(file, obj->marked);
    }
    if (obj->flags[0])
    {
        savefile_write_byte(file, SAVE_ITEM_ART_FLAGS_0);
        savefile_write_u32b(file, obj->flags[0]);
    }
    if (obj->flags[1])
    {
        savefile_write_byte(file, SAVE_ITEM_ART_FLAGS_1);
        savefile_write_u32b(file, obj->flags[1]);
    }
    if (obj->flags[2])
    {
        savefile_write_byte(file, SAVE_ITEM_ART_FLAGS_2);
        savefile_write_u32b(file, obj->flags[2]);
    }
    if (obj->flags[3])
    {
        savefile_write_byte(file, SAVE_ITEM_ART_FLAGS_3);
        savefile_write_u32b(file, obj->flags[3]);
    }
    if (obj->flags[4])
    {
        savefile_write_byte(file, SAVE_ITEM_ART_FLAGS_4);
        savefile_write_u32b(file, obj->flags[4]);
    }
    if (obj->flags[5])
    {
        savefile_write_byte(file, SAVE_ITEM_ART_FLAGS_5);
        savefile_write_u32b(file, obj->flags[5]);
    }
    if (obj->curse_flags)
    {
        savefile_write_byte(file, SAVE_ITEM_CURSE_FLAGS);
        savefile_write_u32b(file, obj->curse_flags);
    }
    if (obj->known_flags[0])
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_FLAGS_0);
        savefile_write_u32b(file, obj->known_flags[0]);
    }
    if (obj->known_flags[1])
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_FLAGS_1);
        savefile_write_u32b(file, obj->known_flags[1]);
    }
    if (obj->known_flags[2])
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_FLAGS_2);
        savefile_write_u32b(file, obj->known_flags[2]);
    }
    if (obj->known_flags[3])
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_FLAGS_3);
        savefile_write_u32b(file, obj->known_flags[3]);
    }
    if (obj->known_flags[4])
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_FLAGS_4);
        savefile_write_u32b(file, obj->known_flags[4]);
    }
    if (obj->known_flags[5])
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_FLAGS_5);
        savefile_write_u32b(file, obj->known_flags[5]);
    }
    if (obj->known_curse_flags)
    {
        savefile_write_byte(file, SAVE_ITEM_KNOWN_CURSE_FLAGS);
        savefile_write_u32b(file, obj->known_curse_flags);
    }
    if (obj->rune)
    {
        savefile_write_byte(file, SAVE_ITEM_RUNE_FLAGS);
        savefile_write_u32b(file, obj->rune);
    }
    if (obj->held_m_idx)
    {
        savefile_write_byte(file, SAVE_ITEM_HELD_M_IDX);
        savefile_write_s16b(file, obj->held_m_idx);
    }
    if (obj->xtra1)
    {
        savefile_write_byte(file, SAVE_ITEM_XTRA1);
        savefile_write_byte(file, obj->xtra1);
    }
    if (obj->xtra2)
    {
        savefile_write_byte(file, SAVE_ITEM_XTRA2);
        savefile_write_byte(file, obj->xtra2);
    }
    if (obj->xtra3)
    {
        savefile_write_byte(file, SAVE_ITEM_XTRA3);
        savefile_write_byte(file, obj->xtra3);
    }
    if (obj->xtra4)
    {
        savefile_write_byte(file, SAVE_ITEM_XTRA4);
        savefile_write_s16b(file, obj->xtra4);
    }
    if (obj->xtra5)
    {
        savefile_write_byte(file, SAVE_ITEM_XTRA5);
        savefile_write_s32b(file, obj->xtra5);
    }
    if (obj->feeling)
    {
        savefile_write_byte(file, SAVE_ITEM_FEELING);
        savefile_write_byte(file, obj->feeling);
    }
    if (obj->inscription)
    {
        savefile_write_byte(file, SAVE_ITEM_INSCRIPTION);
        savefile_write_cptr(file, quark_str(obj->inscription));
    }
    if (obj->art_name)
    {
        savefile_write_byte(file, SAVE_ITEM_ART_NAME);
        savefile_write_cptr(file, quark_str(obj->art_name));
    }
    if (obj->activation.type)
    {
        savefile_write_byte(file, SAVE_ITEM_ACTIVATION);
        savefile_write_s16b(file, obj->activation.type);
        savefile_write_byte(file, obj->activation.power);
        savefile_write_byte(file, obj->activation.difficulty);
        savefile_write_s16b(file, obj->activation.cost);
        savefile_write_s16b(file, obj->activation.extra);
    }
    if (obj->level)
    {
        savefile_write_byte(file, SAVE_ITEM_LEVEL);
        savefile_write_s16b(file, obj->level);
    }

    savefile_write_byte(file, SAVE_ITEM_DONE);
}

