#ifndef INCLUDED_PACK_H
#define INCLUDED_PACK_H

#include "inv.h"

#define PACK_MAX 26

extern void    pack_init(void);

extern void    pack_ui(void);
extern void    pack_display(doc_ptr doc, obj_p p, int flags);

/* Adding and removing */
extern bool    pack_get(void);
extern void    pack_get_aux(int o_idx);
extern void    pack_carry(obj_ptr obj);
extern void    pack_remove(slot_t slot);

/* Accessing, Iterating, Searching */
extern obj_ptr pack_obj(slot_t slot);
extern int     pack_max(void); /* for (slot = 1; slot <= pack_max(); slot++) ... */

extern inv_ptr pack_filter(obj_p p);
extern void    pack_for_each(obj_f f);
extern slot_t  pack_find_first(obj_p p);
extern slot_t  pack_find_next(obj_p p, slot_t prev_match);
extern slot_t  pack_find_art(int which);
extern slot_t  pack_find_ego(int which);
extern slot_t  pack_find_obj(int tval, int sval);
extern slot_t  pack_random_slot(obj_p p);

/* Bonuses: A few rare items grant bonuses from the pack. */
extern void    pack_calc_bonuses(void);

/* Overflow
extern void    pack_push_overflow(obj_ptr obj);*/
extern bool    pack_overflow(void);

/* Optimize: Combine, Sort, Cleanup Garbage */
extern bool    pack_optimize(void);

/* Properties of the Entire Inventory */
extern int     pack_weight(obj_p p);
extern int     pack_count(obj_p p);
extern int     pack_count_slots(obj_p p);

/* Savefiles */
extern void    pack_load(savefile_ptr file);
extern void    pack_save(savefile_ptr file);
#endif
