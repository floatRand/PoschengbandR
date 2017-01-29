#include "angband.h"

#include <assert.h>

static inv_ptr _inv = NULL;
static int     _capacity = 100;

void quiver_init(void)
{
   assert(!_inv);
   _inv = inv_alloc(QUIVER_MAX, INV_QUIVER); 
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
extern obj_ptr quiver_obj(slot_t slot);
extern int     quiver_max(void); /* for (slot = 1; slot <= quiver_max(); slot++) ... */

extern void    quiver_for_each(obj_f f);
extern slot_t  quiver_find_first(obj_p p);
extern slot_t  quiver_find_next(obj_p p, slot_t prev_match);
extern slot_t  quiver_find_art(int which);
extern slot_t  quiver_find_ego(int which);
extern slot_t  quiver_find_obj(int tval, int sval);
extern slot_t  quiver_random_slot(obj_p p);

/* The quiver will 'optimize' upon request, combining objects via
 * stacking and resorting. See PN_REORDER and PN_COMBINE, which
 * I've combined into a single method since it is unclear why 
 * they need to be separate. */
extern bool    quiver_optimize(void);

/* Properties of the Entire Inventory */
extern int     quiver_weight(obj_p p);
extern int     quiver_count(obj_p p);
extern int     quiver_count_slots(obj_p p);

/* Savefiles */
extern void    quiver_load(savefile_ptr file);
extern void    quiver_save(savefile_ptr file);
