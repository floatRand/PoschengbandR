#ifndef INCLUDED_INV_H
#define INCLUDED_INV_H

/* Helper Module for Managing Object Inventories (inv)
 * Used by equip, pack and quiver modules. Could also
 * be used for shop inventories.
 *
 * Object Ownership: Any objects added to the inventory
 * are copied. Clients own the original while this module
 * owns the copies.
 */

typedef int slot_t; /* Slots are 1..max ('if (slot) ...' is a valid idiom) */
                    /* Slots may be empty (unused) */

typedef object_type *obj_ptr;
typedef void (*obj_f)(obj_ptr obj);
typedef void (*slot_f)(slot_t slot);
typedef bool (*obj_p)(obj_ptr obj);

extern bool obj_exists(obj_ptr obj); /* Useful for counting/iterating occupied slots */

typedef struct inv_s inv_t, *inv_ptr; /* Hidden/Abstract */

/* Creation */
extern inv_ptr inv_alloc(int max); /* max=0 is unbounded */
extern void    inv_free(inv_ptr inv);

/* Adding, Removing and Sorting */
extern slot_t  inv_add(inv_ptr inv, obj_ptr obj); /* Copy obj to next avail slot ... no combining */
extern slot_t  inv_combine(inv_ptr inv, obj_ptr obj); /* Try to combine obj (ie stacking). But fallback on inv_add as needed */
extern int     inv_remove(inv_ptr inv, slot_t slot, int ct); /* returns new ct at this slot */
extern void    inv_sort(inv_ptr inv);
extern void    inv_swap(inv_ptr inv, slot_t left, slot_t right);
extern int     obj_cmp(obj_ptr left, obj_ptr right); /* Doesn't belong here ... */
extern int     inv_cmp(inv_ptr inv, slot_t left, slot_t right);   /* Empty slots sort to bottom */

/* Iterating, Searching and Accessing Objects (Predicates are always optional) */
extern obj_ptr inv_obj(inv_ptr inv, slot_t slot); /* NULL if slot is not occupied */
extern slot_t  inv_first(inv_ptr inv, obj_p p);
extern slot_t  inv_next(inv_ptr inv, obj_p p, slot_t prev_match); /* Begins new search at prev_match + 1 */
extern slot_t  inv_last(inv_ptr inv, obj_p p);
extern slot_t  inv_find_art(inv_ptr inv, int which); /* equip module wants to know if a certain artifact is being worn */
extern slot_t  inv_find_ego(inv_ptr inv, int which);
extern slot_t  inv_find_obj(inv_ptr inv, int tval, int sval);
extern void    inv_for_each(inv_ptr inv, obj_f f); /* apply f to each non-null object */
extern void    inv_for_each_that(inv_ptr inv, obj_f f, obj_p p); /* apply f to each object that p accepts */
extern void    inv_for_each_slot(inv_ptr inv, slot_f f); /* apply f to all slots, empty or not */
extern slot_t  inv_random_slot(inv_ptr inv, obj_p p); /* used for disenchantment, cursing, rusting, inventory damage, etc */

/* Properties of the Entire Inventory */
extern int     inv_weight(inv_ptr inv, obj_p p); /* Pass NULL for total weight */
extern int     inv_count(inv_ptr inv, obj_p p); /* Sum(obj->number) for all objects p accepts */
extern int     inv_count_slots(inv_ptr inv, obj_p p); /* Sum(1) for all objects p accepts */
extern int     inv_max_slots(inv_ptr inv); /* from inv_alloc(max) */

/* Savefiles */
extern void    inv_load(inv_ptr inv, savefile_ptr file);
extern void    inv_save(inv_ptr inv, savefile_ptr file);

#endif
