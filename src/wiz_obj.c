#include "angband.h"
#include <assert.h>

#define _NONE  -1
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
typedef int (*_smith_fn)(object_type *o_ptr);

typedef struct {
    int flag;
    cptr name;
    object_p pred;
} _flag_info_t, *_flag_info_ptr;

typedef struct {
    char choice;
    cptr name;
    _smith_fn smithee;
    object_p pred;
} _command_t, *_command_ptr;

static void _toggle(object_type *o_ptr, int flag)
{
    if (have_flag(o_ptr->flags, flag)) remove_flag(o_ptr->flags, flag);
    else add_flag(o_ptr->flags, flag);
    if (is_pval_flag(flag) && have_flag(o_ptr->flags, flag) && o_ptr->pval == 0)
        o_ptr->pval = 1;
    obj_identify_fully(o_ptr);
}

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
                else copy.dd = 99;
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
                if (copy.dd < 99) copy.dd++;
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
                else copy.ds = 99;
            }
            break;
        case 'Y':
            if (object_is_melee_weapon(&copy))
            {
                if (copy.ds < 99) copy.ds++;
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

static int _smith_stats(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < MAX_STATS; i++)
        {
            doc_printf(_doc, "   <color:y>%c</color>) %s\n", I2A(i), stat_name_true[i]);
        }
        doc_insert(_doc, "      Use SHIFT+choice to toggle decrement flag.\n");
        doc_insert(_doc, "      Use CTRL+choice to toggle sustain flag.\n");
        doc_insert(_doc, "      Use p/P to adust the pval.\n");

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ENTER</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>  ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        
        /* Note: iscntrl('\r') is true ... so we need to check this first*/
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'p':
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
            break;
        case 'P':
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
            break;
        }
       
        /* Toggle inc stat? */
        i = A2I(cmd);
        if (0 <= i && i < MAX_STATS)
        {
            _toggle(&copy, OF_STR + i);
            continue;
        }

        /* Toggle dec stat? */
        if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < MAX_STATS)
            {
                _toggle(&copy, OF_DEC_STR + i);
                continue;
            }
        }

        /* Toggle sustain stat? */
        if (iscntrl(cmd))
        {
            char c = 'a' + cmd - KTRL('A');
            i = A2I(c);
            if (0 <= i && i < MAX_STATS)
            {
                _toggle(&copy, OF_SUST_STR + i);
                continue;
            }
        }
    }
}

static bool _blows_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && o_ptr->tval != TV_BOW;
}

static bool _shots_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && !object_is_melee_weapon(o_ptr);
}

static bool _weaponmastery_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && !object_is_melee_weapon(o_ptr)
        && o_ptr->tval != TV_BOW;
}

static _flag_info_t _bonus_flags[] = {
    { OF_BLOWS, "Attack Speed", _blows_p },
    { OF_MAGIC_MASTERY, "Device Skill" },
    { OF_DEVICE_POWER, "Device Power" },
    { OF_TUNNEL, "Digging" },
    { OF_XTRA_MIGHT, "Extra Might", _shots_p },
    { OF_XTRA_SHOTS, "Extra Shots", _shots_p },
    { OF_INFRA, "Infravision" },
    { OF_LIFE, "Life Rating" },
    { OF_MAGIC_RESISTANCE, "Magic Resistance" },
    { OF_SEARCH, "Searching" },
    { OF_SPEED, "Speed" },
    { OF_SPELL_POWER, "Spell Power" },
    { OF_SPELL_CAP, "Spell Capacity" },
    { OF_STEALTH, "Stealth" },
    { OF_WEAPONMASTERY, "Weaponmastery", _weaponmastery_p },
    { OF_INVALID }
};

static int _smith_bonuses(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    vec_ptr     v = vec_alloc(NULL);
    int         result = _NONE, i;

    /* Build list of applicable flags */
    for (i = 0; ; i++)
    {
        _flag_info_ptr fi = &_bonus_flags[i];
        if (fi->flag == OF_INVALID) break;
        if (fi->pred && !fi->pred(o_ptr)) continue;
        vec_add(v, fi);
    }

    while (result == _NONE)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _flag_info_ptr fi = vec_get(v, i);
            doc_printf(_doc, "   <color:y>%c</color>) %s\n", I2A(i), fi->name);
        }

        doc_insert(_doc, "      Use p/P to adust the pval.\n");
        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ENTER</color>) Accept changes\n");
        doc_insert(_doc, " <color:y>  ESC</color>) Cancel changes\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
        {
            *o_ptr = copy;
            result =  _OK;
        }
        else if (cmd == ESCAPE)
        {
            result = _CANCEL;
        }
        else if (cmd == 'p')
        {
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
        }
        else if (cmd == 'P')
        {
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                _flag_info_ptr fi = vec_get(v, i);
                _toggle(&copy, fi->flag);
            }
        }
    }
    vec_free(v);
    return result;
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
            _toggle(&copy, res_get_object_flag(which));
            continue;
        }

        /* Toggle vulnerability? */
        if (isupper(cmd))
        {
            which = A2I(tolower(cmd)) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flag = res_get_object_vuln_flag(which);
                if (flag != OF_INVALID)
                {
                    _toggle(&copy, flag);
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
                int  flag = res_get_object_immune_flag(which);
                if (flag != OF_INVALID)
                {
                    _toggle(&copy, flag);
                    continue;
                }
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
        case 's': _smith_stats(o_ptr); break;
        case 'b': _smith_bonuses(o_ptr); break;
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

