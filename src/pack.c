#include "angband.h"

#include <assert.h>

static inv_ptr _inv = NULL;
static vec_ptr _overflow = NULL;

void pack_init(void)
{
    inv_free(_inv);
    _inv = inv_alloc(PACK_MAX, INV_PACK);
    _overflow = vec_alloc(free);
}

void pack_ui(void)
{
    int     wgt = py_total_weight();
    int     pct = wgt * 100 / weight_limit();
    rect_t  r = ui_map_rect();
    doc_ptr doc = doc_alloc(MIN(80, r.cx));

    r = ui_screen_rect();
    doc_insert(doc, "<color:G>Inventory:</color>\n");

    pack_display(doc, obj_exists, 0);
    doc_printf(doc, "\nCarrying %d.%d pounds (<color:%c>%d%%</color> capacity). <color:y>Command:</color> \n",
                    wgt / 10, wgt % 10, pct > 100 ? 'r' : 'G', pct);

    screen_save();
    doc_sync_term(doc, doc_range_top_lines(doc, r.cy), doc_pos_create(r.x, r.y));
    command_new = inkey();
    screen_load();

    if (command_new == ESCAPE)
        command_new = 0;
    else
        command_see = TRUE;

    doc_free(doc);
}

void pack_display(doc_ptr doc, obj_p p, int flags)
{
    inv_display(_inv, 1, pack_max(), p, doc, NULL, flags);
}

/* Adding and removing */
static void pack_push_overflow(obj_ptr obj);
void pack_carry(obj_ptr obj)
{
    /* Carrying an object is rather complex, and the pile,
     * if pile it be, may distribute between various quiver
     * and pack slots. It may consume new slots. We must handle
     * statistics and the autopicker. It's easiest if all this
     * logic is here, in one place. And it simplifies things
     * to omit checking for a full pack in client code. Instead
     * the pack will just overflow as needed. Perhaps this is
     * a bit comical if the player gets a large floor pile? */
    stats_on_pickup(obj);
    if (quiver_likes(obj))
        quiver_carry(obj);
    if (obj->number)
        inv_combine_ex(_inv, obj);
    if (obj->number)
    {
        slot_t slot = inv_add(_inv, obj);
        if (slot)
        {
            obj_ptr new_obj = inv_obj(_inv, slot);
            new_obj->marked |= OM_TOUCHED;
            autopick_alter_obj(new_obj, destroy_get);
            p_ptr->notice |= PN_OPTIMIZE_PACK;
        }
    }
    if (obj->number)
        pack_push_overflow(obj);
    p_ptr->update |= PU_BONUS;
    p_ptr->window |= PW_INVEN;
}

void pack_get_aux(int o_idx)
{
    object_type *o_ptr = &o_list[o_idx];
    char         name[MAX_NLEN];
    class_t     *class_ptr = get_class();
    int          i;

    if (class_ptr->get_object)
        class_ptr->get_object(o_ptr);

    object_desc(name, o_ptr, OD_COLOR_CODED);
    msg_format("You have %s.", name);

    for (i = 0; i < max_quests; i++)
    {
        if ((quest[i].type == QUEST_TYPE_FIND_ARTIFACT) &&
            (quest[i].status == QUEST_STATUS_TAKEN) &&
               (quest[i].k_idx == o_ptr->name1 || quest[i].k_idx == o_ptr->name3))
        {
            quest[i].status = QUEST_STATUS_COMPLETED;
            quest[i].complev = p_ptr->lev;
            cmsg_print(TERM_L_BLUE, "You completed your quest!");
        }
    }
    pack_carry(o_ptr);
    delete_object_idx(o_idx);

    if (o_ptr->tval == TV_RUNE)
        p_ptr->update |= PU_BONUS;

}

bool pack_get(void)
{
    bool       result = FALSE;
    cave_type *c_ptr = &cave[py][px];
    int        this_o_idx, next_o_idx = 0;

    if (autopick_pickup_items(c_ptr))
        result = TRUE;

    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        object_type *o_ptr = &o_list[this_o_idx];

        next_o_idx = o_ptr->next_o_idx;

        if (o_ptr->tval == TV_GOLD)
        {
            char name[MAX_NLEN];
            int  value = o_ptr->pval;

            object_desc(name, o_ptr, OD_COLOR_CODED);
            delete_object_idx(this_o_idx);

            msg_format("You collect %d gold pieces worth of %s.",
                   value, name);

            sound(SOUND_SELL);

            p_ptr->au += value;
            stats_on_gold_find(value);

            p_ptr->redraw |= (PR_GOLD);

            if (prace_is_(RACE_MON_LEPRECHAUN))
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

            result = TRUE;
        }
        else
        {
            pack_get_aux(this_o_idx);
            result = TRUE;
        }
    }
    return result;
}

void pack_remove(slot_t slot)
{
    inv_remove(_inv, slot);
}

/* Accessing, Iterating, Searching */
obj_ptr pack_obj(slot_t slot)
{
    return inv_obj(_inv, slot);
}

int pack_max(void)
{
    return PACK_MAX;
}

void pack_for_each(obj_f f)
{
    inv_for_each(_inv, f);
}

slot_t pack_find_first(obj_p p)
{
    return inv_first(_inv, p);
}

slot_t pack_find_next(obj_p p, slot_t prev_match)
{
    return inv_next(_inv, p, prev_match);
}

slot_t pack_find_art(int which)
{
    return inv_find_art(_inv, which);
}

slot_t pack_find_ego(int which)
{
    return inv_find_ego(_inv, which);
}

slot_t pack_find_obj(int tval, int sval)
{
    return inv_find_obj(_inv, tval, sval);
}

slot_t pack_random_slot(obj_p p)
{
    return inv_random_slot(_inv, p);
}

/* Bonuses: A few rare items grant bonuses from the pack. */
void pack_calc_bonuses(void)
{
    slot_t slot;
    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (!obj) continue;
        if (obj->name1 == ART_MAUL_OF_VICE)
            p_ptr->maul_of_vice = TRUE;
        if (obj->rune == RUNE_ELEMENTAL_PROTECTION)
            p_ptr->rune_elem_prot = TRUE;
        if (obj->rune == RUNE_GOOD_FORTUNE)
            p_ptr->good_luck = TRUE;
    }
}

/* Overflow: We run obj thru the autopicker to inscribe
 * and perhaps auto-id. We also clear dun-info before
 * redropping, just to be safe. */
static void pack_push_overflow(obj_ptr obj)
{
    obj_ptr new_obj = obj_copy(obj);
    obj_clear_dun_info(new_obj);
    new_obj->marked |= OM_TOUCHED;
    autopick_alter_obj(new_obj, FALSE);
    vec_push(_overflow, new_obj);
}

bool pack_overflow(void)
{
    bool result = FALSE;
    char name[MAX_NLEN];

    while (vec_length(_overflow))
    {
        obj_ptr obj = vec_pop(_overflow);
        if (!result)
        {
            disturb(0, 0);
            cmsg_print(TERM_RED, "Your pack overflows!");
            result = TRUE;
        }
        object_desc(name, obj, 0);
        msg_format("You drop %s.", name);
        drop_near(obj, 0, py, px);
        free(obj);
    }
    if (result)
    {
        notice_stuff();
        handle_stuff();
    }
    return result;
}

/* The pack will 'optimize' upon request, combining objects via
 * stacking and resorting. See PN_REORDER and PN_COMBINE, which
 * I've combined into a single method since it is unclear why 
 * they need to be separate. */
bool pack_optimize(void)
{
    if (inv_optimize(_inv))
    {
        msg_print("You reorder your pack.");
        return TRUE;
    }
    return FALSE;
}

/* Properties of the Entire Inventory */
int pack_weight(obj_p p)
{
    return inv_weight(_inv, p);
}

int pack_count(obj_p p)
{
    return inv_count(_inv, p);
}

int pack_count_slots(obj_p p)
{
    return inv_count_slots(_inv, p);
}

/* Savefiles */
void pack_load(savefile_ptr file)
{
    int i, ct;
    inv_load(_inv, file);
    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        obj_ptr obj = obj_alloc();
        obj_load(obj, file);
        vec_add(_overflow, obj);
    }
}

void pack_save(savefile_ptr file)
{
    int i;
    inv_save(_inv, file);
    savefile_write_s16b(file, vec_length(_overflow));
    for (i = 0; i < vec_length(_overflow); i++)
    {
        obj_ptr obj = vec_get(_overflow, i);
        obj_save(obj, file);
    }
}

