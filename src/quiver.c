#include "angband.h"

#include <assert.h>

static inv_ptr _inv = NULL;
static int     _capacity = 100;

void quiver_init(void)
{
   inv_free(_inv);
   _inv = inv_alloc("Quiver", QUIVER_MAX, INV_QUIVER); 
}

void quiver_display(doc_ptr doc, obj_p p, int flags)
{
    inv_display(_inv, 1, quiver_max(), p, doc, NULL, flags);
}

/* Adding and removing: Quivers allow a large number of slots
 * (QUIVER_MAX) but restrict the number arrows, etc. The capacity 
 * of the quiver may change as the user finds new and better 
 * quivers in the dungeon. */
bool quiver_likes(obj_ptr obj)
{
    if (equip_find_obj(TV_QUIVER, SV_ANY) && obj->tval == p_ptr->shooter_info.tval_ammo)
        return TRUE;
    return FALSE;
}

int quiver_capacity(void)
{
    return _capacity;
}

void quiver_set_capacity(int capacity)
{
    assert(capacity >= quiver_count(NULL));
    _capacity = capacity;
}

void quiver_carry(obj_ptr obj)
{
    /* Helper for pack_carry and equip_wield */
    int ct = quiver_count(NULL);
    int xtra = 0;
    if (ct + obj->number > _capacity)
    {
        xtra = ct + obj->number - _capacity;
        obj->number -= xtra;
    }
    inv_combine_ex(_inv, obj);
    if (obj->number)
    {
        slot_t slot = inv_add(_inv, obj);
        if (slot)
        {
            obj_ptr new_obj = inv_obj(_inv, slot);
            new_obj->marked |= OM_TOUCHED;
            autopick_alter_obj(new_obj, destroy_get);
            p_ptr->notice |= PN_OPTIMIZE_QUIVER;
        }
    }
    obj->number += xtra;
}

void quiver_remove(slot_t slot)
{
    inv_remove(_inv, slot);
}

/* Accessing, Iterating, Searching */
obj_ptr quiver_obj(slot_t slot)
{
    return inv_obj(_inv, slot);
}

int quiver_max(void)
{
    return QUIVER_MAX;
}

inv_ptr quiver_filter(obj_p p)
{
    return inv_filter(_inv, p);
}

void quiver_for_each(obj_f f)
{
    inv_for_each(_inv, f);
}

slot_t quiver_find_first(obj_p p)
{
    return inv_first(_inv, p);
}

slot_t quiver_find_next(obj_p p, slot_t prev_match)
{
    return inv_next(_inv, p, prev_match);
}

slot_t quiver_find_art(int which)
{
    return inv_find_art(_inv, which);
}

slot_t quiver_find_ego(int which)
{
    return inv_find_ego(_inv, which);
}

slot_t quiver_find_obj(int tval, int sval)
{
    return inv_find_obj(_inv, tval, sval);
}

slot_t quiver_random_slot(obj_p p)
{
    return inv_random_slot(_inv, p);
}

/* Optimize */
bool quiver_optimize(void)
{
    if (inv_optimize(_inv))
    {
        msg_print("You reorder your quiver.");
        return TRUE;
    }
    return FALSE;
}

/* Properties of the Entire Inventory */
int quiver_weight(obj_p p)
{
    return inv_weight(_inv, p);
}

int quiver_count(obj_p p)
{
    return inv_count(_inv, p);
}

int quiver_count_slots(obj_p p)
{
    return inv_count_slots(_inv, p);
}

/* Savefiles */
void quiver_load(savefile_ptr file)
{
    inv_load(_inv, file);
}

void quiver_save(savefile_ptr file)
{
    inv_save(_inv, file);
}

