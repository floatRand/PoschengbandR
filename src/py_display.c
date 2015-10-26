#include "angband.h"

#include <stdlib.h>
#include <assert.h>

/* Build & Display the "Character Sheet" */

extern void py_display(void);
extern void py_display_spells(doc_ptr doc, spell_info *table, int ct);
extern void py_display_powers(doc_ptr doc, spell_info *table, int ct);

static void _build_character_sheet(doc_ptr doc);
static void _build_page1(doc_ptr doc);
static void _build_equipment(doc_ptr doc); /* Formerly Pages 2-4 */
static void _build_melee(doc_ptr doc);
static void _build_shooting(doc_ptr doc);
static void _build_powers(doc_ptr doc);
static void _build_spells(doc_ptr doc);

/********************************** Page 1 ************************************/
static void _build_general1(doc_ptr doc)
{
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();

    doc_printf(doc, " Name       : <color:B>%s</color>\n", player_name);
    doc_printf(doc, " Sex        : <color:B>%s</color>\n", sp_ptr->title);
    doc_printf(doc, " Personality: <color:B>%s</color>\n", pers_ptr->name);

    if (race_ptr->mimic)
        doc_printf(doc, " Race       : <color:B>[%s]</color>\n", race_ptr->name);
    else
        doc_printf(doc, " Race       : <color:B>%s</color>\n", race_ptr->name);

    if (race_ptr->subname)
    {
        if (p_ptr->prace == RACE_MON_RING)
            doc_printf(doc, " Controlling: <color:B>%-27.27s</color>\n", race_ptr->subname);
        else if (p_ptr->prace == RACE_MON_MIMIC)
        {
            if (p_ptr->current_r_idx == MON_MIMIC)
                doc_printf(doc, " Mimicking  : <color:B>%-27.27s</color>\n", "Nothing");
            else
                doc_printf(doc, " Mimicking  : <color:B>%-27.27s</color>\n", race_ptr->subname);
        }
        else
            doc_printf(doc, " Subrace    : <color:B>%-27.27s</color>\n", race_ptr->subname);
    }
    else
        doc_printf(doc, " Subrace    : <color:B>%-27.27s</color>\n", "None");

    doc_printf(doc, " Class      : <color:B>%s</color>\n", class_ptr->name);

    /* Assume Subclass and Magic are mutually exclusive ... */
    if (class_ptr->subname)
        doc_printf(doc, " Subclass   : <color:B>%-27.27s</color>\n", class_ptr->subname);
    else if (p_ptr->pclass == CLASS_WARLOCK)
        doc_printf(doc, " Subclass   : <color:B>%-27.27s</color>\n", pact_info[p_ptr->psubclass].title);
    else if (p_ptr->pclass == CLASS_WEAPONMASTER)
        doc_printf(doc, " Subclass   : <color:B>%-27.27s</color>\n", weaponmaster_speciality_name(p_ptr->psubclass));
    else if (p_ptr->prace == RACE_MON_DRAGON)
    {
        dragon_realm_ptr realm = dragon_get_realm(p_ptr->dragon_realm);
        doc_printf(doc, " Realm      : <color:B>%-27.27s</color>\n", realm->name);
    }
    else if (p_ptr->realm1)
    {
        if (p_ptr->realm2)
            doc_printf(doc, " Realm      : <color:B>%s, %s</color>\n", realm_names[p_ptr->realm1], realm_names[p_ptr->realm2]);
        else
            doc_printf(doc, " Realm      : <color:B>%s</color>\n", realm_names[p_ptr->realm1]);
    }
    else
        doc_newline(doc);

    if ((p_ptr->pclass == CLASS_CHAOS_WARRIOR) || mut_present(MUT_CHAOS_GIFT))
        doc_printf(doc, " Patron     : <color:B>%s</color>\n", chaos_patrons[p_ptr->chaos_patron]);
    else
        doc_newline(doc);


    doc_printf(doc, " Level      : <color:G>%8d</color>\n", p_ptr->lev);
    if (p_ptr->prace == RACE_ANDROID)
    {
        doc_printf(doc, " Construct  : <color:%c>%8d</color>\n", p_ptr->exp >= p_ptr->max_exp ? 'G' : 'y', p_ptr->exp);
        doc_newline(doc);
    }
    else
    {
        doc_printf(doc, " Cur Exp    : <color:%c>%8d</color>\n", p_ptr->exp >= p_ptr->max_exp ? 'G' : 'y', p_ptr->exp);
        doc_printf(doc, " Max Exp    : <color:G>%8d</color>\n", p_ptr->max_exp);
    }
    doc_printf(doc, " Adv Exp    : <color:G>%8.8s</color>\n", p_ptr->lev >= PY_MAX_LEVEL ? "*****" : format("%d", exp_requirement(p_ptr->lev)));
    doc_newline(doc);
    doc_newline(doc);

    doc_printf(doc, " Gold       : <color:G>%8d</color>\n", p_ptr->au);
    doc_printf(doc, " Kills      : <color:G>%8d</color>\n", ct_kills());
    doc_printf(doc, " Uniques    : <color:G>%8d</color>\n", ct_uniques());
    doc_printf(doc, " Artifacts  : <color:G>%8.8s</color>\n",
                            no_artifacts ? "N/A" : format("%d+%d" , ct_artifacts(), stats_rand_art_counts.found));
    doc_newline(doc);

    {
        int day, hour, min;
        extract_day_hour_min(&day, &hour, &min);

        doc_printf(doc, " Game Day   : <color:G>%8d</color>\n", day);
        doc_printf(doc, " Game Time  : <color:G>%8.8s</color>\n", format("%d:%02d", hour, min));
    }

    update_playtime();
    doc_printf(doc, " Play Time  : <color:G>%8.8s</color>\n",
                            format("%.2lu:%.2lu", playtime/(60*60), (playtime/60)%60));
}

static void _build_general2(doc_ptr doc)
{
    string_ptr s = string_alloc();
    char       buf[255];
    int        i;

    doc_insert(doc, "   ========== Stats ==========\n");
    for (i = 0; i < MAX_STATS; i++)
    {
        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
            doc_printf(doc, "<tab:9>%3.3s", stat_names_reduced[i]);
        else
            doc_printf(doc, "<tab:9>%3.3s", stat_names[i]);

        if (p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
            doc_insert(doc, "! : ");
        else
            doc_insert(doc, "  : ");

        cnv_stat(p_ptr->stat_use[i], buf);
        doc_printf(doc, "<color:%c>%9.9s</color>\n", p_ptr->stat_use[i] < p_ptr->stat_top[i] ? 'y' : 'G', buf);
    }

    doc_newline(doc);

    string_clear(s);
    string_printf(s, "%d/%d", p_ptr->chp , p_ptr->mhp);
    doc_printf(doc, "<tab:9>HP   : <color:%c>%9.9s</color>\n",
                    p_ptr->chp >= p_ptr->mhp ? 'G' :
                        p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10 ? 'y' : 'r',
                    string_buffer(s));

    string_clear(s);
    string_printf(s, "%d/%d", p_ptr->csp , p_ptr->msp);
    doc_printf(doc, "<tab:9>SP   : <color:%c>%9.9s</color>\n",
                    p_ptr->csp >= p_ptr->msp ? 'G' :
                        p_ptr->csp > (p_ptr->msp * mana_warn) / 10 ? 'y' : 'r',
                    string_buffer(s));

    doc_printf(doc, "<tab:9>AC   : <color:G>%9d</color>\n", p_ptr->dis_ac + p_ptr->dis_to_a);

    /* Dump speed ... What a monster! */
    {
        int  tmp_speed = 0;
        byte attr;
        int  speed = p_ptr->pspeed-110;

        /* Hack -- Visually "undo" the Search Mode Slowdown */
        if (p_ptr->action == ACTION_SEARCH) speed += 10;

        if (speed > 0)
        {
            if (!p_ptr->riding)
                attr = TERM_L_GREEN;
            else
                attr = TERM_GREEN;
        }
        else if (i == 0)
        {
            if (!p_ptr->riding)
                attr = TERM_L_BLUE;
            else
                attr = TERM_GREEN;
        }
        else
        {
            if (!p_ptr->riding)
                attr = TERM_L_UMBER;
            else
                attr = TERM_RED;
        }

        if (!p_ptr->riding)
        {
            if (IS_FAST()) tmp_speed += 10;
            if (p_ptr->slow) tmp_speed -= 10;
            if (IS_LIGHT_SPEED()) tmp_speed = 99;
        }
        else
        {
            if (MON_FAST(&m_list[p_ptr->riding])) tmp_speed += 10;
            if (MON_SLOW(&m_list[p_ptr->riding])) tmp_speed -= 10;
        }

        string_clear(s);
        if (tmp_speed)
        {
            string_printf(s, "%+d%+d", speed-tmp_speed, tmp_speed);
            if (tmp_speed > 0)
                attr = TERM_YELLOW;
            else
                attr = TERM_VIOLET;
        }
        else
        {
            string_printf(s, "%+d", speed);
        }

        doc_printf(doc, "<tab:9>Speed: <color:%c>%9.9s</color>\n", attr_to_attr_char(attr), string_buffer(s));
    }

    doc_newline(doc);
    doc_insert(doc, "   ========== Skills =========\n");

    {
        skills_t        skills = p_ptr->skills;
        int             slot = equip_find_object(TV_BOW, SV_ANY);
        skill_desc_t    desc = {0};

        /* Patch Up Skills a bit */
        skills.thn += p_ptr->to_h_m * BTH_PLUS_ADJ;
        if (slot)
        {
            object_type *bow = equip_obj(slot);
            if (bow)
                skills.thb += (p_ptr->shooter_info.to_h + bow->to_h) * BTH_PLUS_ADJ;
        }
        if (!skills.stl)
            skills.stl = -1; /* Force "Very Bad" */

        /* Display */
        desc = skills_describe(skills.thn, 12);
        doc_printf(doc, "   Melee      : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.thb, 12);
        doc_printf(doc, "   Ranged     : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.sav, 7);
        doc_printf(doc, "   SavingThrow: <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.stl, 1);
        doc_printf(doc, "   Stealth    : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.fos, 6);
        doc_printf(doc, "   Perception : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.srh, 6);
        doc_printf(doc, "   Searching  : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.dis, 8);
        doc_printf(doc, "   Disarming  : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);

        desc = skills_describe(skills.dev, 6);
        doc_printf(doc, "   Device     : <color:%c>%s</color>\n", attr_to_attr_char(desc.color), desc.desc);
    }

    string_free(s);
}

static void _build_page1(doc_ptr doc)
{
    doc_ptr cols[2];

    cols[0] = doc_alloc(40);
    cols[1] = doc_alloc(40);

    _build_general1(cols[0]);
    _build_general2(cols[1]);
    doc_insert_cols(doc, cols, 2, 0);

    doc_free(cols[0]);
    doc_free(cols[1]);
}

/********************************** Equipment *********************************/
static void _equippy_chars(doc_ptr doc, int col)
{
    if (equippy_chars)
    {
        int i;
        doc_printf(doc, "<tab:%d>", col);
        for (i = 0; i < equip_count(); i++)
        {
            int          slot = EQUIP_BEGIN + i;
            object_type *o_ptr = equip_obj(slot);

            if (o_ptr)
            {
                byte a;
                char c;

                a = object_attr(o_ptr);
                c = object_char(o_ptr);

                doc_insert_char(doc, a, c);
            }
            else
                doc_insert_char(doc, TERM_WHITE, ' ');
        }
        doc_newline(doc);
    }
}

static void _equippy_heading_aux(doc_ptr doc, cptr heading, int col)
{
    int i;
    doc_printf(doc, " <color:G>%-11.11s</color><tab:%d>", heading, col);
    for (i = 0; i < equip_count(); i++)
        doc_insert_char(doc, TERM_WHITE, 'a' + i);
    doc_insert_char(doc, TERM_WHITE, '@');
}

static void _equippy_heading(doc_ptr doc, cptr heading, int col)
{
    _equippy_heading_aux(doc, heading, col);
    doc_newline(doc);
}

typedef struct {
    u32b py_flgs[TR_FLAG_SIZE];
    u32b tim_py_flgs[TR_FLAG_SIZE];
    u32b obj_flgs[EQUIP_MAX_SLOTS][TR_FLAG_SIZE];
} _flagzilla_t, *_flagzilla_ptr;

static _flagzilla_ptr _flagzilla_alloc(void)
{
    _flagzilla_ptr flagzilla = malloc(sizeof(_flagzilla_t));
    int            i;

    memset(flagzilla, 0, sizeof(_flagzilla_t));

    player_flags(flagzilla->py_flgs);
    tim_player_flags(flagzilla->tim_py_flgs);
    for (i = 0; i < equip_count(); i++)
    {
        int          slot = EQUIP_BEGIN + i;
        object_type *o_ptr = equip_obj(slot);

        if (o_ptr)
            object_flags_known(o_ptr, flagzilla->obj_flgs[i]);
    }

    return flagzilla;
}

static void _flagzilla_free(_flagzilla_ptr flagzilla)
{
    free(flagzilla);
}

static void _build_res_flags(doc_ptr doc, int which, _flagzilla_ptr flagzilla)
{
    int i;
    int flg = res_get_object_flag(which);
    int im_flg = res_get_object_immune_flag(which);
    int vuln_flg = res_get_object_vuln_flag(which);
    int pct = res_pct_known(which);
    char color = 'w';

    doc_printf(doc, " %-11.11s: ", res_name(which));

    for (i = 0; i < equip_count(); i++)
    {
        if (im_flg != TR_INVALID && have_flag(flagzilla->obj_flgs[i], im_flg))
            doc_insert_char(doc, TERM_VIOLET, '*');
        else if (vuln_flg != TR_INVALID && have_flag(flagzilla->obj_flgs[i], vuln_flg))
            doc_insert_char(doc, TERM_L_RED, '-');
        else if (have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_WHITE, '+');
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }

    if (im_flg != TR_INVALID && have_flag(flagzilla->py_flgs, im_flg))
        doc_insert_char(doc, TERM_VIOLET, '*');
    else if (im_flg != TR_INVALID && have_flag(flagzilla->tim_py_flgs, im_flg))
        doc_insert_char(doc, TERM_YELLOW, '*');
    else if (have_flag(flagzilla->tim_py_flgs, flg))
    {
        if (vuln_flg != TR_INVALID && have_flag(flagzilla->py_flgs, vuln_flg))
            doc_insert_char(doc, TERM_ORANGE, '#');
        else
            doc_insert_char(doc, TERM_YELLOW, '#');
    }
    else if (vuln_flg != TR_INVALID && have_flag(flagzilla->py_flgs, vuln_flg))
        doc_insert_char(doc, TERM_RED, 'v');
    else if (have_flag(flagzilla->py_flgs, flg))
        doc_insert_char(doc, TERM_WHITE, '+');
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    if (pct == 100)
        color = 'v';
    else if (pct < 0)
        color = 'D';
    else if (res_is_low(which))
    {
        if (pct >= 72)
            color =  'r';
        else if (pct >= 65)
            color =  'R';
        else if (pct >= 50)
            color =  'y';
    }
    else
    {
        if (pct >= 45)
            color =  'r';
        else if (pct >= 40)
            color =  'R';
        else if (pct >= 30)
            color =  'y';
    }

    if (which == RES_FEAR)
        doc_printf(doc, " %3dx", res_ct_known(which));
    else
        doc_printf(doc, " <color:%c>%3d%%</color>", color, pct);
    doc_newline(doc);
}

static void _build_curse_flags(doc_ptr doc, cptr name)
{
    int i;
    doc_printf(doc, " %-11.11s: ", name);
    for (i = 0; i < equip_count(); i++)
    {
        int          slot = EQUIP_BEGIN + i;
        object_type *o_ptr = equip_obj(slot);

        if (o_ptr)
        {
            if (o_ptr->curse_flags & TRC_PERMA_CURSE)
                doc_insert_char(doc, TERM_VIOLET, '*');
            else if (o_ptr->curse_flags & TRC_HEAVY_CURSE)
                doc_insert_char(doc, TERM_L_RED, '+');
            else if (o_ptr->curse_flags & TRC_CURSED)
                doc_insert_char(doc, TERM_WHITE, '+');
            else
                doc_insert_char(doc, TERM_L_DARK, '.');
        }
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    doc_insert_char(doc, TERM_L_DARK, '.');
    doc_newline(doc);
}

static void _build_slays_imp(doc_ptr doc, cptr name, int flg, int kill_flg, _flagzilla_ptr flagzilla)
{
    int i;
    doc_printf(doc, " %-11.11s: ", name);
    for (i = 0; i < equip_count(); i++)
    {
        if (kill_flg != TR_INVALID && have_flag(flagzilla->obj_flgs[i], kill_flg))
            doc_insert_char(doc, TERM_RED, '*');
        else if (have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_WHITE, '+');
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    if (kill_flg != TR_INVALID && have_flag(flagzilla->tim_py_flgs, kill_flg))
        doc_insert_char(doc, TERM_YELLOW, '*');
    else if (have_flag(flagzilla->tim_py_flgs, flg))
        doc_insert_char(doc, TERM_YELLOW, '+');
    else if (kill_flg != TR_INVALID && have_flag(flagzilla->py_flgs, kill_flg))
        doc_insert_char(doc, TERM_RED, '*');
    else if (have_flag(flagzilla->py_flgs, flg))
        doc_insert_char(doc, TERM_WHITE, '+');
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    doc_newline(doc);
}

static int _build_flags_imp(doc_ptr doc, cptr name, int flg, int dec_flg, _flagzilla_ptr flagzilla)
{
    int result = 0;
    int i;
    doc_printf(doc, " %-11.11s: ", name);
    for (i = 0; i < equip_count(); i++)
    {
        if (have_flag(flagzilla->obj_flgs[i], flg))
        {
            doc_insert_char(doc, TERM_WHITE, '+');
            result++;
        }
        else if (dec_flg != TR_INVALID && have_flag(flagzilla->obj_flgs[i], dec_flg))
        {
            doc_insert_char(doc, TERM_L_RED, '-');
            result--;
        }
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    if (have_flag(flagzilla->tim_py_flgs, flg))
    {
       doc_insert_char(doc, TERM_YELLOW, '#');
       result++;
    }
    else if (have_flag(flagzilla->py_flgs, flg))
    {
        doc_insert_char(doc, TERM_WHITE, '+');
        result++;
    }
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    return result;
}

static void _build_flags_aura(doc_ptr doc, cptr name, int flg, _flagzilla_ptr flagzilla)
{
    if (_build_flags_imp(doc, name, flg, TR_INVALID, flagzilla))
    {
        doc_printf(doc, " %dd%d+2", 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
    }
    doc_newline(doc);
}

static void _build_flags(doc_ptr doc, cptr name, int flg, int dec_flg, _flagzilla_ptr flagzilla)
{
    _build_flags_imp(doc, name, flg, dec_flg, flagzilla);
    doc_newline(doc);
}

static void _build_flags1(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int i;
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Resistances", 14);

    for (i = RES_BEGIN; i < RES_END; i++)
        _build_res_flags(doc, i, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Auras", 14);
    _build_flags_aura(doc, "Aura Elec", TR_SH_ELEC, flagzilla);
    _build_flags_aura(doc, "Aura Fire", TR_SH_FIRE, flagzilla);
    _build_flags_aura(doc, "Aura Cold", TR_SH_COLD, flagzilla);
    _build_flags_aura(doc, "Aura Shards", TR_SH_SHARDS, flagzilla);
    _build_flags(doc, "Revenge", TR_SH_REVENGE, TR_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Slays", 14);
    _build_slays_imp(doc, "Slay Evil", TR_SLAY_EVIL, TR_KILL_EVIL, flagzilla);
    _build_slays_imp(doc, "Slay Undead", TR_SLAY_UNDEAD, TR_KILL_UNDEAD, flagzilla);
    _build_slays_imp(doc, "Slay Demon", TR_SLAY_DEMON, TR_KILL_DEMON, flagzilla);
    _build_slays_imp(doc, "Slay Dragon", TR_SLAY_DRAGON, TR_KILL_DRAGON, flagzilla);
    _build_slays_imp(doc, "Slay Human", TR_SLAY_HUMAN, TR_KILL_HUMAN, flagzilla);
    _build_slays_imp(doc, "Slay Animal", TR_SLAY_ANIMAL, TR_KILL_ANIMAL, flagzilla);
    _build_slays_imp(doc, "Slay Orc", TR_SLAY_ORC, TR_KILL_ORC, flagzilla);
    _build_slays_imp(doc, "Slay Troll", TR_SLAY_TROLL, TR_KILL_TROLL, flagzilla);
    _build_slays_imp(doc, "Slay Giant", TR_SLAY_GIANT, TR_KILL_GIANT, flagzilla);
    _build_slays_imp(doc, "Slay Good", TR_SLAY_GOOD, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Slay Living", TR_SLAY_LIVING, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Acid Brand", TR_BRAND_ACID, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Elec Brand", TR_BRAND_ELEC, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Fire Brand", TR_BRAND_FIRE, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Cold Brand", TR_BRAND_COLD, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Pois Brand", TR_BRAND_POIS, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Mana Brand", TR_FORCE_WEAPON, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Sharpness", TR_VORPAL, TR_VORPAL2, flagzilla);
    _build_slays_imp(doc, "Quake", TR_IMPACT, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Vampiric", TR_VAMPIRIC, TR_INVALID, flagzilla);
    _build_slays_imp(doc, "Chaotic", TR_CHAOTIC, TR_INVALID, flagzilla);
    _build_flags(doc, "Add Blows", TR_BLOWS, TR_DEC_BLOWS, flagzilla);
    _build_flags(doc, "Blessed", TR_BLESSED, TR_INVALID, flagzilla);
    _build_flags(doc, "Riding", TR_RIDING, TR_INVALID, flagzilla);
    _build_flags(doc, "Tunnel", TR_TUNNEL, TR_INVALID, flagzilla);
    _build_flags(doc, "Throwing", TR_THROW, TR_INVALID, flagzilla);
}

static void _build_flags2(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Abilities", 14);

    _build_flags(doc, "Speed", TR_SPEED, TR_DEC_SPEED, flagzilla);
    _build_flags(doc, "Free Act", TR_FREE_ACT, TR_INVALID, flagzilla);
    _build_flags(doc, "See Invis", TR_SEE_INVIS, TR_INVALID, flagzilla);
    _build_flags(doc, "Warning", TR_WARNING, TR_INVALID, flagzilla);
    _build_flags(doc, "SlowDigest", TR_SLOW_DIGEST, TR_INVALID, flagzilla);
    _build_flags(doc, "Regenerate", TR_REGEN, TR_INVALID, flagzilla);
    _build_flags(doc, "Levitation", TR_LEVITATION, TR_INVALID, flagzilla);
    _build_flags(doc, "Perm Lite", TR_LITE, TR_INVALID, flagzilla);
    _build_flags(doc, "Reflection", TR_REFLECT, TR_INVALID, flagzilla);
    _build_flags(doc, "Hold Life", TR_HOLD_LIFE, TR_INVALID, flagzilla);
    _build_flags(doc, "Dec Mana", TR_DEC_MANA, TR_INVALID, flagzilla);
    _build_flags(doc, "Easy Spell", TR_EASY_SPELL, TR_INVALID, flagzilla);
    _build_flags(doc, "Anti Magic", TR_NO_MAGIC, TR_INVALID, flagzilla);

    _build_flags_imp(doc, "Magic Skill", TR_MAGIC_MASTERY, TR_DEC_MAGIC_MASTERY, flagzilla);
    if (p_ptr->device_power)
    {
        int pow = device_power_aux(100, p_ptr->device_power) - 100;
        doc_printf(doc, " %+3d%%", pow);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "Spell Power", TR_SPELL_POWER, TR_DEC_SPELL_POWER, flagzilla))
    {
        int  pow = spell_power_aux(100, p_ptr->spell_power) - 100;
        doc_printf(doc, " %+3d%%", pow);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "Spell Cap", TR_SPELL_CAP, TR_DEC_SPELL_CAP, flagzilla))
    {
        int cap = spell_cap_aux(100, p_ptr->spell_cap) - 100;
        doc_printf(doc, " %+3d%%", cap);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "Magic Res", TR_MAGIC_RESISTANCE, TR_INVALID, flagzilla))
        doc_printf(doc, " %+3d%%", p_ptr->magic_resistance);
    doc_newline(doc);

    if (_build_flags_imp(doc, "Infravision", TR_INFRA, TR_INVALID, flagzilla))
        doc_printf(doc, " %3d'", p_ptr->see_infra * 10);
    doc_newline(doc);

    _build_flags(doc, "Stealth", TR_STEALTH, TR_DEC_STEALTH, flagzilla);
    _build_flags(doc, "Searching", TR_SEARCH, TR_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Sustains", 14);
    _build_flags(doc, "Sust Str", TR_SUST_STR, TR_INVALID, flagzilla);
    _build_flags(doc, "Sust Int", TR_SUST_INT, TR_INVALID, flagzilla);
    _build_flags(doc, "Sust Wis", TR_SUST_WIS, TR_INVALID, flagzilla);
    _build_flags(doc, "Sust Dex", TR_SUST_DEX, TR_INVALID, flagzilla);
    _build_flags(doc, "Sust Con", TR_SUST_CON, TR_INVALID, flagzilla);
    _build_flags(doc, "Sust Chr", TR_SUST_CHR, TR_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Detection", 14);
    _build_flags(doc, "Telepathy", TR_TELEPATHY, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Evil", TR_ESP_EVIL, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Nonliv", TR_ESP_NONLIVING, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Good", TR_ESP_GOOD, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Undead", TR_ESP_UNDEAD, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Demon", TR_ESP_DEMON, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Dragon", TR_ESP_DRAGON, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Human", TR_ESP_HUMAN, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Animal", TR_ESP_ANIMAL, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Orc", TR_ESP_ORC, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Troll", TR_ESP_TROLL, TR_INVALID, flagzilla);
    _build_flags(doc, "ESP Giant", TR_ESP_GIANT, TR_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "Curses", 14);
    _build_curse_flags(doc, "Cursed");
    _build_flags(doc, "Rnd Tele", TR_TELEPORT, TR_INVALID, flagzilla);
    _build_flags(doc, "No Tele", TR_NO_TELE, TR_INVALID, flagzilla);
    _build_flags(doc, "Drain Exp", TR_DRAIN_EXP, TR_INVALID, flagzilla);
    _build_flags(doc, "Aggravate", TR_AGGRAVATE, TR_INVALID, flagzilla);
    _build_flags(doc, "TY Curse", TR_TY_CURSE, TR_INVALID, flagzilla);
}

static void _build_stats(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int              i, j;
    char             buf[255];
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();
    s16b             stats[MAX_STATS] = {0};
    s16b             tim_stats[MAX_STATS] = {0};

    mut_calc_stats(stats);
    if (race_ptr->calc_stats)
        race_ptr->calc_stats(stats);
    if (class_ptr->calc_stats)
        class_ptr->calc_stats(stats);

    tim_player_stats(tim_stats);

    _equippy_chars(doc, 14);
    _equippy_heading_aux(doc, "", 14);
    doc_insert(doc, "   Base  R  C  P  E  Total\n");

    for (i = 0; i < MAX_STATS; i++)
    {
        int flg = TR_STR + i;
        int dec_flg = TR_DEC_STR + i;
        int sust_flg = TR_SUST_STR + i;
        int e_adj = 0;

        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
            doc_printf(doc, "<tab:7>%3.3s", stat_names_reduced[i]);
        else
            doc_printf(doc, "<tab:7>%3.3s", stat_names[i]);

        if (p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
            doc_insert(doc, "! : ");
        else
            doc_insert(doc, "  : ");

        /* abcdefghijkl */
        for (j = 0; j < equip_count(); j++)
        {
            int          slot = EQUIP_BEGIN + j;
            object_type *o_ptr = equip_obj(slot);

            if (o_ptr)
            {
                int adj = 0;

                if (o_ptr->rune)
                {
                    s16b stats[MAX_STATS] = {0};
                    rune_calc_stats(o_ptr, stats);
                    adj += stats[i];
                }
                if (have_flag(flagzilla->obj_flgs[j], dec_flg))
                    adj = -o_ptr->pval;
                else if (have_flag(flagzilla->obj_flgs[j], flg))
                    adj += o_ptr->pval;

                if (adj)
                {
                    byte a = TERM_WHITE;
                    char c = '*';
                    if (abs(adj) < 10)
                        c = '0' + abs(adj);

                    if (adj < 0)
                        a = TERM_RED;
                    else
                    {
                        a = TERM_L_GREEN;
                        if (have_flag(flagzilla->obj_flgs[j], sust_flg))
                            a = TERM_GREEN;
                    }
                    doc_insert_char(doc, a, c);
                }
                else
                {
                    byte a = TERM_L_DARK;
                    char c = '.';

                    if (have_flag(flagzilla->obj_flgs[j], sust_flg))
                    {
                        a = TERM_GREEN;
                        c = 's';
                    }
                    doc_insert_char(doc, a, c);
                }
                e_adj += adj;
            }
            else
                doc_insert_char(doc, TERM_L_DARK, '.');
        }
        /* @ */
        if (stats[i] + tim_stats[i] != 0)
        {
            byte a = TERM_WHITE;
            char c = '*';
            int  adj = stats[i] + tim_stats[i];

            if (abs(adj) < 10)
                c = '0' + abs(adj);

            if (tim_stats[i])
                a = TERM_YELLOW;
            else if (adj > 0)
            {
                a = TERM_L_GREEN;
                if (have_flag(flagzilla->tim_py_flgs, sust_flg) || have_flag(flagzilla->py_flgs, sust_flg))
                    a = TERM_GREEN;
            }
            else if (adj < 0)
                a = TERM_RED;

            doc_insert_char(doc, a, c);
        }
        else
        {
            byte a = TERM_L_DARK;
            char c = '.';

            if (have_flag(flagzilla->tim_py_flgs, sust_flg))
            {
                a = TERM_YELLOW;
                c = 's';
            }
            else if (have_flag(flagzilla->py_flgs, sust_flg))
            {
                a = TERM_GREEN;
                c = 's';
            }
            doc_insert_char(doc, a, c);
        }

        /* Base R C P E */
        cnv_stat(p_ptr->stat_max[i], buf);
        doc_printf(doc, " <color:B>%6.6s%3d%3d%3d%3d</color>",
                    buf, race_ptr->stats[i], class_ptr->stats[i], pers_ptr->stats[i], e_adj);

        /* Total */
        cnv_stat(p_ptr->stat_top[i], buf);
        doc_printf(doc, " <color:G>%6.6s</color>", buf);

        /* Current */
        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
        {
            cnv_stat(p_ptr->stat_use[i], buf);
            doc_printf(doc, " <color:y>%6.6s</color>", buf);
        }

        doc_newline(doc);
    }
    doc_newline(doc);
}

static void _build_equipment(doc_ptr doc)
{
    bool old_use_graphics = use_graphics;

    /* It always bugged me that equippy characters didn't show in character dumps!*/
    if (equippy_chars && old_use_graphics)
    {
        use_graphics = FALSE;
        reset_visuals();
    }

    if (equip_count_used())
    {
        int slot, i;
        char o_name[MAX_NLEN];
        _flagzilla_ptr flagzilla = 0;

        doc_insert(doc, "<topic:Equipment>============================= Character Equipment =============================\n\n");
        for (slot = EQUIP_BEGIN, i = 0; slot < EQUIP_BEGIN + equip_count(); slot++, i++)
        {
            object_type *o_ptr = equip_obj(slot);
            if (!o_ptr) continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            doc_printf(doc, " %c) <indent><style:indent>%s</style></indent>\n", index_to_label(i), o_name);
        }
        doc_newline(doc);


        /* Flags */
        flagzilla = _flagzilla_alloc();
        {
            doc_ptr cols[2];

            cols[0] = doc_alloc(40);
            cols[1] = doc_alloc(40);
            _build_flags1(cols[0], flagzilla);
            _build_flags2(cols[1], flagzilla);
            doc_insert_cols(doc, cols, 2, 0);
            doc_free(cols[0]);
            doc_free(cols[1]);
        }

        /* Stats */
        _build_stats(doc, flagzilla);

        _flagzilla_free(flagzilla);
    }

    if (equippy_chars && old_use_graphics)
    {
        use_graphics = TRUE;
        reset_visuals();
    }
}

/****************************** Combat ************************************/
static void _build_melee(doc_ptr doc)
{
    if (p_ptr->prace != RACE_MON_RING)
    {
        int i;
        doc_insert(doc, "<topic:Melee>==================================== Melee ====================================\n\n");
        for (i = 0; i < MAX_HANDS; i++)
        {
            if (p_ptr->weapon_info[i].wield_how == WIELD_NONE) continue;
            if (p_ptr->weapon_info[i].bare_hands)
                monk_display_attack_info(doc, i);
            else
                display_weapon_info(doc, i);
        }

        for (i = 0; i < p_ptr->innate_attack_ct; i++)
        {
            display_innate_attack_info(doc, i);
        }
    }
}

static void _build_shooting(doc_ptr doc)
{
    if (equip_find_object(TV_BOW, SV_ANY) && !prace_is_(RACE_MON_JELLY) && p_ptr->shooter_info.tval_ammo != TV_NO_AMMO)
    {
        doc_insert(doc, "<topic:Shooting>=================================== Shooting ==================================\n\n");
        display_shooter_info(doc);
    }
}

/****************************** Magic ************************************/
void py_display_powers(doc_ptr doc, spell_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;
    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    doc_printf(doc, "<topic:Powers>=================================== Powers ====================================\n\n");
    doc_printf(doc, "<color:G>%-20.20s Lvl Cost Fail %-15.15s Cast Fail</color>\n", "", "Desc");
    for (i = 0; i < ct; i++)
    {
        spell_info     *spell = &table[i];
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
        spell->fn(SPELL_COST_EXTRA, &vc);
        spell->fn(SPELL_FAIL_MIN, &vfm);

        doc_printf(doc, "%-20.20s %3d %4d %3d%% %-15.15s %4d %4d %3d%%\n",
            var_get_string(&vn),
            spell->level, calculate_cost(spell->cost + var_get_int(&vc)), MAX(spell->fail, var_get_int(&vfm)),
            var_get_string(&vd),
            stats->ct_cast, stats->ct_fail,
            spell_stats_fail(stats)
        );
    }

    var_clear(&vn);
    var_clear(&vd);
    var_clear(&vc);
    var_clear(&vfm);

    doc_newline(doc);
}

static void _build_powers(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = 0;
    race_t    *race_ptr = get_race();
    class_t   *class_ptr = get_class();

    if (race_ptr->get_powers)
        ct += (race_ptr->get_powers)(spells + ct, MAX_SPELLS - ct);

    if (class_ptr->get_powers)
        ct += (class_ptr->get_powers)(spells + ct, MAX_SPELLS - ct);

    ct += mut_get_powers(spells + ct, MAX_SPELLS - ct);

    py_display_powers(doc, spells, ct);
}

void py_display_spells(doc_ptr doc, spell_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;

    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    doc_printf(doc, "<topic:Spells>=================================== Spells ====================================\n\n");
    doc_printf(doc, "<color:G>%-20.20s Lvl Cost Fail %-15.15s Cast Fail</color>\n", "", "Desc");

    for (i = 0; i < ct; i++)
    {
        spell_info     *spell = &table[i];
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
        spell->fn(SPELL_COST_EXTRA, &vc);
        spell->fn(SPELL_FAIL_MIN, &vfm);

        doc_printf(doc, "%-20.20s %3d %4d %3d%% %-15.15s %4d %4d %3d%%\n",
            var_get_string(&vn),
            spell->level, calculate_cost(spell->cost + var_get_int(&vc)), MAX(spell->fail, var_get_int(&vfm)),
            var_get_string(&vd),
            stats->ct_cast, stats->ct_fail,
            spell_stats_fail(stats)
        );
    }

    var_clear(&vn);
    var_clear(&vd);
    var_clear(&vc);
    var_clear(&vfm);

    doc_newline(doc);
}

static void _build_spells(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = 0;
    race_t    *race_ptr = get_race();
    /*class_t   *class_ptr = get_class_t();*/

    if (race_ptr->get_spells)
        ct += (race_ptr->get_spells)(spells + ct, MAX_SPELLS - ct);

    /* TODO: Some classes prompt the user at this point ...
    if (class_ptr->get_spells)
        ct += (class_ptr->get_spells)(spells + ct, MAX_SPELLS - ct); */

    py_display_spells(doc, spells, ct);
}

/****************************** Character Sheet ************************************/
static void _build_character_sheet(doc_ptr doc)
{
    doc_insert(doc, "<style:wide>  [PosChengband <$:version> Character Dump]\n");
    if (p_ptr->total_winner)
        doc_insert(doc, "              <color:B>***WINNER***</color>\n");
    else if (p_ptr->is_dead)
        doc_insert(doc, "              <color:v>***LOSER***</color>\n");
    else
        doc_newline(doc);
    doc_newline(doc);

    _build_page1(doc);
    _build_equipment(doc);
    _build_melee(doc);
    _build_shooting(doc);
    _build_powers(doc);
    _build_spells(doc);

    {
        class_t *class_ptr = get_class();
        race_t  *race_ptr = get_race();
        if (class_ptr->character_dump)
            (class_ptr->character_dump)(doc);
        if (race_ptr && race_ptr->character_dump)
            race_ptr->character_dump(doc);
    }

    doc_insert(doc, "</style>");
}

void py_display(void)
{
    doc_ptr d = doc_alloc(80);

    _build_character_sheet(d);

    screen_save();
    doc_display(d, "Character Sheet", 0);
    screen_load();
    doc_free(d);
}
