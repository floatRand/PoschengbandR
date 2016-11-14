#include "angband.h"
#include <assert.h>

#define _OK     0
#define _CANCEL 1

static doc_ptr _doc = NULL;

static int _inkey(void)
{
    return inkey_special(TRUE);
}

/***********************************************************************
 * Object Creation
 **********************************************************************/
void wiz_obj_create(void)
{
}

/***********************************************************************
 * Object Modification (Smithing)
 **********************************************************************/
static int _smith_plusses(object_type *o_ptr)
{
    rect_t      r = ui_map_rect();
    object_type copy = *o_ptr;

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        if (object_is_melee_weapon(o_ptr))
        {
            doc_insert(_doc, "      Use x/X to adust the damage dice.\n");
            doc_insert(_doc, "      Use y/Y to adust the damage sides.\n");
        }
        else if (o_ptr->tval == TV_BOW)
            doc_insert(_doc, "      Use x/X to adust the multiplier.\n");
        else
            doc_insert(_doc, "      Use x/X to adust the base AC.\n");
        doc_insert(_doc, "      Use a/A to adust the armor class bonus.\n");
        doc_insert(_doc, "      Use h/H to adust the melee accuracy.\n");
        doc_insert(_doc, "      Use d/D to adust the melee damage.\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ENTER</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>  ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'x':
            if (object_is_melee_weapon(&copy))
            {
                if (copy.dd > 0) copy.dd--;
                else copy.dd = 15;
            }
            else if (copy.tval == TV_BOW)
            {
                if (copy.mult > 0) copy.mult -= 5;
                else copy.mult = 700;
            }
            else
            {
                if (copy.ac > 0) copy.ac--;
                else copy.ac = 50;
            }
            break;
        case 'X':
            if (object_is_melee_weapon(&copy))
            {
                if (copy.dd < 15) copy.dd++;
                else copy.dd = 0;
            }
            else if (copy.tval == TV_BOW)
            {
                if (copy.mult < 696) copy.mult += 5;
                else copy.mult = 0;
            }
            else
            {
                if (copy.ac < 50) copy.ac++;
                else copy.ac = 0;
            }
            break;
        case 'y':
            if (object_is_melee_weapon(&copy))
            {
                if (copy.ds > 0) copy.ds--;
                else copy.ds = 15;
            }
            break;
        case 'Y':
            if (object_is_melee_weapon(&copy))
            {
                if (copy.ds < 15) copy.ds++;
                else copy.ds = 0;
            }
            break;
        case 'h':
            if (copy.to_h > -50) copy.to_h--;
            else copy.to_h = 50;
            break;
        case 'H':
            if (copy.to_h < 50) copy.to_h++;
            else copy.to_h = -50;
            break;
        case 'd':
            if (copy.to_d > -50) copy.to_d--;
            else copy.to_d = 50;
            break;
        case 'D':
            if (copy.to_d < 50) copy.to_d++;
            else copy.to_d = -50;
            break;
        case 'a':
            if (copy.to_a > -50) copy.to_a--;
            else copy.to_a = 50;
            break;
        case 'A':
            if (copy.to_a < 50) copy.to_a++;
            else copy.to_a = -50;
            break;
        }
    }
}

static void _reroll_aux(object_type *o_ptr, int flags)
{
    object_prep(o_ptr, o_ptr->k_idx);
    apply_magic(o_ptr, dun_level, AM_NO_FIXED_ART | flags);
    obj_identify_fully(o_ptr);
}

static int _smith_reroll(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "   <color:y>w</color>) Awful\n");
        doc_insert(_doc, "   <color:y>b</color>) Bad\n");
        doc_insert(_doc, "   <color:y>a</color>) Average\n");
        doc_insert(_doc, "   <color:y>g</color>) Good\n");
        doc_insert(_doc, "   <color:y>e</color>) Excellent\n");
        doc_insert(_doc, "   <color:y>r</color>) Random Artifact\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ENTER</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>  ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'w': _reroll_aux(&copy, AM_GOOD | AM_GREAT | AM_CURSED); break;
        case 'b': _reroll_aux(&copy, AM_GOOD | AM_CURSED); break;
        case 'a': _reroll_aux(&copy, AM_AVERAGE); break;
        case 'g': _reroll_aux(&copy, AM_GOOD); break;
        case 'e': _reroll_aux(&copy, AM_GOOD | AM_GREAT); break;
        case 'r': _reroll_aux(&copy, AM_GOOD | AM_GREAT | AM_SPECIAL); break;
        }
    }
}

static int _smith_resistances(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int  cmd, which;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (which = RES_BEGIN; which < RES_END; which++)
        {
            doc_printf(_doc, "   <color:y>%c</color>) %s%c\n",
                I2A(which - RES_BEGIN), res_name(which),
                res_get_object_immune_flag(which) != OF_INVALID ? '*' : ' ');
        }

        doc_newline(_doc);
        doc_insert(_doc, "   SHIFT+choice toggle vulnerability\n");
        doc_insert(_doc, "(*)CTRL+choice toggle immunity\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ENTER</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>  ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();

        /* Note: iscntrl('\r') is true ... so we need to check this first*/
        if (cmd == '\r')
        {
            *o_ptr = copy;
            return _OK;
        }
        else if (cmd == ESCAPE)
            return _CANCEL;

        /* Toggle resistance? */
        which = A2I(cmd) + RES_BEGIN;
        if (RES_BEGIN <= which && which < RES_END)
        {
            int  flg = res_get_object_flag(which);
            if (have_flag(copy.flags, flg))
                remove_flag(copy.flags, flg);
            else
                add_flag(copy.flags, flg);
            obj_identify_fully(&copy);
            continue;
        }

        /* Toggle vulnerability? */
        if (isupper(cmd))
        {
            which = A2I(tolower(cmd)) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flg = res_get_object_vuln_flag(which);
                if (flg != OF_INVALID)
                {
                    if (have_flag(copy.flags, flg))
                        remove_flag(copy.flags, flg);
                    else
                        add_flag(copy.flags, flg);
                    obj_identify_fully(&copy);
                    continue;
                }
            }
        }

        /* Toggle immunity? */
        if (iscntrl(cmd))
        {
            char c = 'a' + cmd - KTRL('A');
            which = A2I(c) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flg = res_get_object_immune_flag(which);
                if (flg != OF_INVALID)
                {
                    if (have_flag(copy.flags, flg))
                        remove_flag(copy.flags, flg);
                    else
                        add_flag(copy.flags, flg);
                    obj_identify_fully(&copy);
                }
                continue;
            }
        }
    }
}

static int _smith_weapon_armor(object_type *o_ptr)
{
    rect_t r = ui_map_rect();

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        doc_insert(_doc, "   <color:y>p</color>) Plusses\n");
        doc_insert(_doc, "   <color:y>s</color>) Stats\n");
        doc_insert(_doc, "   <color:y>b</color>) Bonuses\n");
        doc_insert(_doc, "   <color:y>r</color>) Resistances\n");
        doc_insert(_doc, "   <color:y>S</color>) Sustains\n");
        doc_insert(_doc, "   <color:y>a</color>) Abilities\n");
        doc_insert(_doc, "   <color:y>t</color>) Telepathies\n");
        if (object_is_melee_weapon(o_ptr))
        {
            doc_insert(_doc, "   <color:y>k</color>) Slays (Kills)\n");
            doc_insert(_doc, "   <color:y>B</color>) Brands\n");
        }
        doc_insert(_doc, "   <color:y>R</color>) Re-roll\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ENTER</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>  ESC</color>) Cancel changes\n");
        doc_newline(_doc);
        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case ESCAPE: return _CANCEL;
        case '\r': return _OK;
        case 'p': _smith_plusses(o_ptr); break;
        case 'r': _smith_resistances(o_ptr); break;
        case 'R': _smith_reroll(o_ptr); break;
        }
    }
}

static int _smith_object(object_type *o_ptr)
{
    int result = _OK;
    assert(!_doc);
    _doc = doc_alloc(72);
    msg_line_clear();
    Term_save();

    if (object_is_weapon(o_ptr) || object_is_armour(o_ptr))
        result = _smith_weapon_armor(o_ptr);

    Term_load();
    doc_free(_doc);
    _doc = NULL;
    return result;
}

static bool _smith_p(object_type *o_ptr)
{
    if (object_is_weapon(o_ptr)) return TRUE;
    if (object_is_armour(o_ptr)) return TRUE;
    return FALSE;
}

void wiz_obj_smith(void)
{
    int          item;
    object_type *o_ptr;
    object_type  copy;

    item_tester_hook = _smith_p;
    item_tester_no_ryoute = TRUE;

    if (!get_item(&item, "Smith which object? ", "You have nothing to work with.", USE_INVEN | USE_EQUIP | USE_FLOOR))
        return;
    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];
    
    copy = *o_ptr;
    obj_identify_fully(&copy);

    if (_smith_object(&copy) == _OK)
    {
        if (item >= 0) p_ptr->total_weight -= o_ptr->weight*o_ptr->number;
        *o_ptr = copy;
        if (item >= 0) p_ptr->total_weight += o_ptr->weight*o_ptr->number;
        p_ptr->update |= PU_BONUS;
        p_ptr->notice |= PN_COMBINE | PN_REORDER;
        p_ptr->window |= PW_INVEN;
    }
}

