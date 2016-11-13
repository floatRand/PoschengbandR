/* File: cmd5.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Spell/Prayer commands */

#include "angband.h"

cptr spell_category_name(int tval)
{
    switch (tval)
    {
    case TV_HISSATSU_BOOK:
        return "art";
    case TV_LIFE_BOOK:
        return "prayer";
    case TV_MUSIC_BOOK:
        return "song";
    default:
        return "spell";
    }
}

/*
 * Allow user to choose a spell/prayer from the given book.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 *
 * The "prompt" should be "cast", "recite", or "study"
 * The "known" should be TRUE for cast/pray, FALSE for study
 */

bool select_the_force = FALSE;

static int get_spell(int *sn, cptr prompt, int sval, bool learned, int use_realm)
{
    int         i;
    int         spell = -1;
    int         num = 0;
    int         ask = TRUE;
    int         need_mana;
    byte        spells[64];
    bool        flag, redraw, okay;
    char        choice;
    magic_type  *s_ptr;
    char        out_val[160];
    cptr        p;
    rect_t      display = ui_menu_rect();
    int menu_line = (use_menu ? 1 : 0);

#ifdef ALLOW_REPEAT /* TNB */

    /* Get the spell, if available */
    if (repeat_pull(sn))
    {
        /* Verify the spell */
        if (spell_okay(*sn, learned, FALSE, use_realm))
        {
            /* Success */
            return (TRUE);
        }
    }

#endif /* ALLOW_REPEAT -- TNB */

    p = spell_category_name(mp_ptr->spell_book);

    /* Extract spells */
    for (spell = 0; spell < 32; spell++)
    {
        /* Check for this spell */
        if ((fake_spell_flags[sval] & (1L << spell)))
        {
            /* Collect this spell */
            spells[num++] = spell;
        }
    }

    /* Assume no usable spells */
    okay = FALSE;

    /* Assume no spells available */
    (*sn) = -2;

    /* Check for "okay" spells */
    for (i = 0; i < num; i++)
    {
        /* Look for "okay" spells */
        if (spell_okay(spells[i], learned, FALSE, use_realm)) okay = TRUE;
    }

    /* No "okay" spells */
    if (!okay) return (FALSE);
    if (((use_realm) != p_ptr->realm1) && ((use_realm) != p_ptr->realm2) && (p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE)) return FALSE;
    if (((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE)) && !is_magic(use_realm)) return FALSE;
    if ((p_ptr->pclass == CLASS_RED_MAGE) && ((use_realm) != REALM_ARCANE) && (sval > 1)) return FALSE;

    /* Assume cancelled */
    *sn = (-1);

    /* Nothing chosen yet */
    flag = FALSE;

    /* No redraw yet */
    redraw = FALSE;

    /* Show choices */
    p_ptr->window |= (PW_SPELL);

    /* Window stuff */
    window_stuff();

    /* Build a prompt (accept all spells) */
    (void)strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit) %^s which %s? ",
        p, I2A(0), I2A(num - 1), prompt, p);

    /* Get a spell from the user */

    choice = (always_show_list || use_menu) ? ESCAPE : 1;
    while (!flag)
    {
        if (choice == ESCAPE) choice = ' '; 
        else if (!get_com(out_val, &choice, TRUE))break;

        if (use_menu && choice != ' ')
        {
            switch (choice)
            {
                case '0':
                {
                    screen_load();
                    return FALSE;
                }

                case '8':
                case 'k':
                case 'K':
                {
                    menu_line += (num - 1);
                    break;
                }

                case '2':
                case 'j':
                case 'J':
                {
                    menu_line++;
                    break;
                }

                case 'x':
                case 'X':
                case '\r':
                case '\n':
                {
                    i = menu_line - 1;
                    ask = FALSE;
                    break;
                }
            }
            if (menu_line > num) menu_line -= num;
            /* Display a list of spells */
            print_spells(menu_line, spells, num, display, use_realm);
            if (ask) continue;
        }
        else
        {
            /* Request redraw */
            if ((choice == ' ') || (choice == '*') || (choice == '?'))
            {
                /* Show the list */
                if (!redraw)
                {
                    /* Show list */
                    redraw = TRUE;

                    /* Save the screen */
                    screen_save();

                    /* Display a list of spells */
                    print_spells(menu_line, spells, num, display, use_realm);
                }

                /* Hide the list */
                else
                {
                    if (use_menu) continue;

                    /* Hide list */
                    redraw = FALSE;

                    /* Restore the screen */
                    screen_load();
                }

                /* Redo asking */
                continue;
            }


            /* Note verify */
            ask = (isupper(choice));

            /* Lowercase */
            if (ask) choice = tolower(choice);

            /* Extract request */
            i = (islower(choice) ? A2I(choice) : -1);
        }

        /* Totally Illegal */
        if ((i < 0) || (i >= num))
        {
            bell();
            continue;
        }

        /* Save the spell index */
        spell = spells[i];

        /* Require "okay" spells */
        if (!spell_okay(spell, learned, FALSE, use_realm))
        {
            bell();
            msg_format("You may not %s that %s.", prompt, p);
            continue;
        }

        /* Verify it */
        if (ask)
        {
            char tmp_val[160];

            /* Access the spell */
            if (!is_magic(use_realm))
            {
                s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
            }
            else
            {
                s_ptr = &mp_ptr->info[use_realm - 1][spell];
            }

            /* Extract mana consumption rate */
            if (use_realm == REALM_HISSATSU)
            {
                need_mana = s_ptr->smana;
            }
            else
            {
                need_mana = mod_need_mana(s_ptr->smana, spell, use_realm);
            }

            /* Prompt */
            (void)strnfmt(tmp_val, 78, "%^s %s (%d mana, %d%% fail)? ",
                prompt, do_spell(use_realm, spell, SPELL_NAME), need_mana,
                spell_chance(spell, use_realm));

            /* Belay that order */
            if (!get_check(tmp_val)) continue;
        }

        /* Stop the loop */
        flag = TRUE;
    }


    /* Restore the screen */
    if (redraw) screen_load();


    /* Show choices */
    p_ptr->window |= (PW_SPELL);

    /* Window stuff */
    window_stuff();


    /* Abort if needed */
    if (!flag) return FALSE;

    /* Save the choice */
    (*sn) = spell;

#ifdef ALLOW_REPEAT /* TNB */

    repeat_push(*sn);

#endif /* ALLOW_REPEAT -- TNB */

    /* Success */
    return TRUE;
}


static bool item_tester_learn_spell(object_type *o_ptr)
{
    s32b choices = realm_choices2[p_ptr->pclass];

    if (p_ptr->pclass == CLASS_PRIEST)
    {
        if (is_good_realm(p_ptr->realm1))
        {
            choices &= ~(CH_DEATH | CH_DAEMON);
        }
        else
        {
            choices &= ~(CH_LIFE | CH_CRUSADE);
        }
    }

    if ((o_ptr->tval < TV_LIFE_BOOK) || (o_ptr->tval > (TV_LIFE_BOOK + MAX_REALM - 1))) return (FALSE);
    if ((o_ptr->tval == TV_MUSIC_BOOK) && (p_ptr->pclass == CLASS_BARD)) return (TRUE);
    else if (!is_magic(tval2realm(o_ptr->tval))) return FALSE;
    if ((REALM1_BOOK == o_ptr->tval) || (REALM2_BOOK == o_ptr->tval)) return (TRUE);
    if (choices & (0x0001 << (tval2realm(o_ptr->tval) - 1))) return (TRUE);
    return (FALSE);
}


static bool player_has_no_spellbooks(void)
{
    int         i;
    object_type *o_ptr;

    for (i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];
        if (o_ptr->k_idx && check_book_realm(o_ptr->tval, o_ptr->sval)) return FALSE;
    }

    for (i = cave[py][px].o_idx; i; i = o_ptr->next_o_idx)
    {
        o_ptr = &o_list[i];
        if (o_ptr->k_idx && (o_ptr->marked & OM_FOUND) && check_book_realm(o_ptr->tval, o_ptr->sval)) return FALSE;
    }

    return TRUE;
}


static void confirm_use_force(bool browse_only)
{
    int  item;
    char which;

#ifdef ALLOW_REPEAT

    /* Get the item index */
    if (repeat_pull(&item) && (item == INVEN_FORCE))
    {
        browse_only ? do_cmd_spell_browse() : do_cmd_spell();
        return;
    }

#endif /* ALLOW_REPEAT */

    /* Show the prompt */
    prt("(w for the Force, ESC) Hit 'w' or ESC. ", 0, 0);

    while (1)
    {
        /* Get a key */
        which = inkey();

        if (which == ESCAPE) break;
        else if (which == 'w')
        {

#ifdef ALLOW_REPEAT

            repeat_push(INVEN_FORCE);

#endif /* ALLOW_REPEAT */

            break;
        }
    }

    /* Clear the prompt line */
    prt("", 0, 0);

    if (which == 'w')
    {
        browse_only ? do_cmd_spell_browse() : do_cmd_spell();
    }
}


/*
 * Peruse the spells/prayers in a book
 *
 * Note that *all* spells in the book are listed
 *
 * Note that browsing is allowed while confused or blind,
 * and in the dark, primarily to allow browsing in stores.
 */
void do_cmd_browse(void)
{
    int        item, sval, use_realm = 0, j, line;
    int        spell = -1;
    int        num = 0;
    rect_t     display = ui_menu_rect();
    byte       spells[64];
    char       temp[62*4];

    object_type    *o_ptr;

    cptr q, s;

    /* Warriors are illiterate */
    if (!(p_ptr->realm1 || p_ptr->realm2) && (p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE))
    {
        msg_print("You cannot read books!");

        return;
    }

    if (p_ptr->special_defense & KATA_MUSOU)
    {
        set_action(ACTION_NONE);
    }

    if (p_ptr->pclass == CLASS_FORCETRAINER)
    {
        if (player_has_no_spellbooks())
        {
            confirm_use_force(TRUE);
            return;
        }
        select_the_force = TRUE;
    }

    /* Restrict choices to "useful" books */
    if (p_ptr->realm2 == REALM_NONE) item_tester_tval = mp_ptr->spell_book;
    else item_tester_hook = item_tester_learn_spell;

    /* Get an item */
    q = "Browse which book? ";

    s = "You have no books that you can read.";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR)))
    {
        select_the_force = FALSE;
        return;
    }
    select_the_force = FALSE;

    if (item == INVEN_FORCE) /* the_force */
    {
        do_cmd_spell_browse();
        return;
    }

    /* Get the item (in the pack) */
    else if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Access the item's sval */
    sval = o_ptr->sval;

    use_realm = tval2realm(o_ptr->tval);

    /* Track the object kind */
    object_kind_track(o_ptr->k_idx);

    /* Hack -- Handle stuff */
    handle_stuff();


    /* Extract spells */
    for (spell = 0; spell < 32; spell++)
    {
        /* Check for this spell */
        if ((fake_spell_flags[sval] & (1L << spell)))
        {
            /* Collect this spell */
            spells[num++] = spell;
        }
    }


    /* Save the screen */
    screen_save();

    /* Clear the top line */
    prt("", 0, 0);

    /* Keep browsing spells. Exit browsing on cancel. */
    while(TRUE)
    {
        /* Ask for a spell, allow cancel */
        if (!get_spell(&spell, "browse", o_ptr->sval, TRUE, use_realm))
        {
            /* If cancelled, leave immediately. */
            if (spell == -1) break;

            /* Display a list of spells */
            print_spells(0, spells, num, display, use_realm);

            /* Notify that there's nothing to see, and wait. */
            if (use_realm == REALM_HISSATSU)
                prt("No techniques to browse.", 0, 0);
            else
                prt("No spells to browse.", 0, 0);
            (void)inkey();


            /* Restore the screen */
            screen_load();

            return;
        }

        /* Clear lines, position cursor  (really should use strlen here) */
        line = display.y + num + 1;
        Term_erase(display.x, line, display.cx);
        Term_erase(display.x, line + 1, display.cx);
        Term_erase(display.x, line + 2, display.cx);
        Term_erase(display.x, line + 3, display.cx);

        roff_to_buf(do_spell(use_realm, spell, SPELL_DESC), 62, temp, sizeof(temp));

        for (j = 0; temp[j]; j += 1 + strlen(&temp[j]))
        {
            put_str(&temp[j], ++line, display.x);
        }
    }

    /* Restore the screen */
    screen_load();
}


static void change_realm2(int next_realm)
{
    int i, j = 0;
    for (i = 0; i < 64; i++)
    {
        p_ptr->spell_order[j] = p_ptr->spell_order[i];
        if (p_ptr->spell_order[i] < 32) j++;
    }
    for (; j < 64; j++)
        p_ptr->spell_order[j] = 99;

    for (i = 32; i < 64; i++)
    {
        p_ptr->spell_exp[i] = SPELL_EXP_UNSKILLED;
    }
    p_ptr->spell_learned2 = 0L;
    p_ptr->spell_worked2 = 0L;
    p_ptr->spell_forgotten2 = 0L;

    p_ptr->old_realm |= 1 << (p_ptr->realm2-1);
    p_ptr->realm2 = next_realm;

    p_ptr->notice |= (PN_REORDER);
    p_ptr->update |= (PU_SPELLS);
    handle_stuff();

    /* Load an autopick preference file */
    autopick_load_pref(FALSE);
}


/*
 * Study a book to gain a new spell/prayer
 */
void do_cmd_study(void)
{
    int    i, item, sval;
    int    increment = 0;
    bool    learned = FALSE;

    /* Spells of realm2 will have an increment of +32 */
    int    spell = -1;

    cptr p = spell_category_name(mp_ptr->spell_book);

    object_type *o_ptr;

    cptr q, s;

    if (!p_ptr->realm1)
    {
        msg_print("You cannot read books!");

        return;
    }

    if (p_ptr->blind || no_lite())
    {
        msg_print("You cannot see!");

        return;
    }

    if (p_ptr->confused)
    {
        msg_print("You are too confused!");

        return;
    }

    if (!(p_ptr->new_spells))
    {
        msg_format("You cannot learn any new %ss!", p);

        return;
    }

    if (p_ptr->special_defense & KATA_MUSOU)
    {
        set_action(ACTION_NONE);
    }

/*  Please, no more -more-!
    msg_format("You can learn %d new %s%s.", p_ptr->new_spells, p,
        (p_ptr->new_spells == 1?"":"s"));

    msg_print(NULL);*/


    /* Restrict choices to "useful" books */
    if (p_ptr->realm2 == REALM_NONE) item_tester_tval = mp_ptr->spell_book;
    else item_tester_hook = item_tester_learn_spell;

    /* Get an item */
    q = "Study which book? ";

    s = "You have no books that you can read.";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Access the item's sval */
    sval = o_ptr->sval;

    if (o_ptr->tval == REALM2_BOOK) increment = 32;
    else if (o_ptr->tval != REALM1_BOOK)
    {
        if (!get_check("Really, change magic realm? ")) return;
        change_realm2(tval2realm(o_ptr->tval));
        increment = 32;
    }

    /* Track the object kind */
    object_kind_track(o_ptr->k_idx);

    /* Hack -- Handle stuff */
    handle_stuff();

    /* Mage -- Learn a selected spell */
    if (mp_ptr->spell_book != TV_LIFE_BOOK)
    {
        /* Ask for a spell, allow cancel */
        if (!get_spell(&spell, "study", sval, FALSE, o_ptr->tval - TV_LIFE_BOOK + 1)
            && (spell == -1)) return;

    }

    /* Priest -- Learn a random prayer */
    else
    {
        int k = 0;

        int gift = -1;

        /* Extract spells */
        for (spell = 0; spell < 32; spell++)
        {
            /* Check spells in the book */
            if ((fake_spell_flags[sval] & (1L << spell)))
            {
                /* Skip non "okay" prayers */
                if (!spell_okay(spell, FALSE, TRUE,
                    (increment ? p_ptr->realm2 : p_ptr->realm1))) continue;

                /* Hack -- Prepare the randomizer */
                k++;

                /* Hack -- Apply the randomizer */
                if (one_in_(k)) gift = spell;
            }
        }

        /* Accept gift */
        spell = gift;
    }

    /* Nothing to study */
    if (spell < 0)
    {
        /* Message */
        msg_format("You cannot learn any %ss in that book.", p);


        /* Abort */
        return;
    }


    if (increment) spell += increment;

    /* Learn the spell */
    if (spell < 32)
    {
        if (p_ptr->spell_learned1 & (1L << spell)) learned = TRUE;
        else p_ptr->spell_learned1 |= (1L << spell);
    }
    else
    {
        if (p_ptr->spell_learned2 & (1L << (spell - 32))) learned = TRUE;
        else p_ptr->spell_learned2 |= (1L << (spell - 32));
    }

    if (learned)
    {
        int max_exp = (spell < 32) ? SPELL_EXP_MASTER : SPELL_EXP_EXPERT;
        int old_exp = p_ptr->spell_exp[spell];
        int new_rank = EXP_LEVEL_UNSKILLED;
        cptr name = do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell%32, SPELL_NAME);

        if (old_exp >= max_exp)
        {
            msg_format("You don't need to study this %s anymore.", p);
            return;
        }
        if (!get_check(format("You will study a %s of %s again. Are you sure? ", p, name)))
        {
            return;
        }
        else if (old_exp >= SPELL_EXP_EXPERT)
        {
            p_ptr->spell_exp[spell] = SPELL_EXP_MASTER;
            new_rank = EXP_LEVEL_MASTER;
        }
        else if (old_exp >= SPELL_EXP_SKILLED)
        {
            if (spell >= 32) p_ptr->spell_exp[spell] = SPELL_EXP_EXPERT;
            else p_ptr->spell_exp[spell] += SPELL_EXP_EXPERT - SPELL_EXP_SKILLED;
            new_rank = EXP_LEVEL_EXPERT;
        }
        else if (old_exp >= SPELL_EXP_BEGINNER)
        {
            p_ptr->spell_exp[spell] = SPELL_EXP_SKILLED + (old_exp - SPELL_EXP_BEGINNER) * 2 / 3;
            new_rank = EXP_LEVEL_SKILLED;
        }
        else
        {
            p_ptr->spell_exp[spell] = SPELL_EXP_BEGINNER + old_exp / 3;
            new_rank = EXP_LEVEL_BEGINNER;
        }
        msg_format("Your proficiency of %s is now %s rank.", name, exp_level_str[new_rank]);
    }
    else
    {
        /* Find the next open entry in "p_ptr->spell_order[]" */
        for (i = 0; i < 64; i++)
        {
            /* Stop at the first empty space */
            if (p_ptr->spell_order[i] == 99) break;
        }

        /* Add the spell to the known list */
        p_ptr->spell_order[i++] = spell;

        /* Mention the result */
        msg_format("You have learned the %s of %s.",
            p, do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell % 32, SPELL_NAME));
    }

    /* Take a turn */
    energy_use = 100;

    switch (mp_ptr->spell_book)
    {
    case TV_LIFE_BOOK:
        virtue_add(VIRTUE_FAITH, 1);
        break;
    case TV_DEATH_BOOK:
    case TV_NECROMANCY_BOOK:
        virtue_add(VIRTUE_UNLIFE, 1);
        break;
    case TV_NATURE_BOOK:
        virtue_add(VIRTUE_NATURE, 1);
        break;
    default:
        virtue_add(VIRTUE_KNOWLEDGE, 1);
        break;
    }

    /* Sound */
    sound(SOUND_STUDY);

    /* One less spell available */
    p_ptr->learned_spells++;
#if 0
    /* Message if needed */
    if (p_ptr->new_spells)
    {
        /* Message */
        msg_format("You can learn %d more %s%s.", p_ptr->new_spells, p,
                   (p_ptr->new_spells != 1) ? "s" : "");
    }
#endif

    p_ptr->update |= PU_SPELLS;
    p_ptr->redraw |= PR_EFFECTS;
    p_ptr->window |= PW_OBJECT;
}


static void wild_magic(int spell)
{
    int counter = 0;
    int type = SUMMON_BIZARRE1 + randint0(6);

    if (type < SUMMON_BIZARRE1) type = SUMMON_BIZARRE1;
    else if (type > SUMMON_BIZARRE6) type = SUMMON_BIZARRE6;

    switch (randint1(spell) + randint1(8) + 1)
    {
    case 1:
    case 2:
    case 3:
        teleport_player(10, TELEPORT_PASSIVE);
        break;
    case 4:
    case 5:
    case 6:
        teleport_player(100, TELEPORT_PASSIVE);
        break;
    case 7:
    case 8:
        teleport_player(200, TELEPORT_PASSIVE);
        break;
    case 9:
    case 10:
    case 11:
        unlite_area(10, 3);
        break;
    case 12:
    case 13:
    case 14:
        lite_area(damroll(2, 3), 2);
        break;
    case 15:
        destroy_doors_touch();
        break;
    case 16: case 17:
        wall_breaker();
    case 18:
        sleep_monsters_touch();
        break;
    case 19:
    case 20:
        trap_creation(py, px);
        break;
    case 21:
    case 22:
        door_creation();
        break;
    case 23:
    case 24:
    case 25:
        aggravate_monsters(0);
        break;
    case 26:
        earthquake(py, px, 5);
        break;
    case 27:
    case 28:
        mut_gain_random(NULL);
        break;
    case 29:
    case 30:
        apply_disenchant(1);
        break;
    case 31:
        lose_all_info();
        break;
    case 32:
        fire_ball(GF_CHAOS, 0, spell + 5, 1 + (spell / 10));
        break;
    case 33:
        wall_stone();
        break;
    case 34:
    case 35:
        while (counter++ < 8)
        {
            (void)summon_specific(0, py, px, (dun_level * 3) / 2, type, (PM_ALLOW_GROUP | PM_NO_PET));
        }
        break;
    case 36:
    case 37:
        activate_hi_summon(py, px, FALSE);
        break;
    case 38:
        (void)summon_cyber(-1, py, px);
        break;
    default:
        {
            int count = 0;
            (void)activate_ty_curse(FALSE, &count);
            break;
        }
    }

    return;
}


/*
 * Cast a spell
 */
void do_cmd_cast(void)
{
    int    item, sval, spell, realm;
    int    chance;
    int    increment = 0;
    int    use_realm;
    int    need_mana;
    int take_mana;

    cptr prayer;

    object_type    *o_ptr;

    magic_type    *s_ptr;

    cptr q, s;

    caster_info    *caster_ptr = get_caster_info();

    /* Require spell ability */
    if (!p_ptr->realm1 && (p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE))
    {
        msg_print("You cannot cast spells!");

        return;
    }

    /* Require lite */
    if (p_ptr->blind || no_lite())
    {
        if (p_ptr->pclass == CLASS_FORCETRAINER) confirm_use_force(FALSE);
        else
        {
            msg_print("You cannot see!");
            flush();
        }
        return;
    }

    /* Not when confused */
    if (p_ptr->confused)
    {
        msg_print("You are too confused!");
        flush();
        return;
    }

    /* Hex */
    if (p_ptr->realm1 == REALM_HEX)
    {
        if (hex_spell_fully())
        {
            bool flag = FALSE;
            msg_print("Can not spell new spells more.");
            flush();
            if (p_ptr->lev >= 35) flag = stop_hex_spell();
            if (!flag) return;
        }
    }

    if (p_ptr->pclass == CLASS_FORCETRAINER)
    {
        if (player_has_no_spellbooks())
        {
            confirm_use_force(FALSE);
            return;
        }
        select_the_force = TRUE;
    }

    prayer = spell_category_name(mp_ptr->spell_book);

    /* Restrict choices to spell books */
    item_tester_tval = mp_ptr->spell_book;

    /* Get an item */
    q = "Use which book? ";

    s = "You have no spell books!";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR)))
    {
        select_the_force = FALSE;
        return;
    }
    select_the_force = FALSE;

    if (item == INVEN_FORCE) /* the_force */
    {
        do_cmd_spell(); /* hack */
        return;
    }

    /* Get the item (in the pack) */
    else if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Access the item's sval */
    sval = o_ptr->sval;

    if ((p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE) && (o_ptr->tval == REALM2_BOOK)) increment = 32;


    /* Track the object kind */
    object_kind_track(o_ptr->k_idx);

    /* Hack -- Handle stuff */
    handle_stuff();

    if ((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE))
        realm = o_ptr->tval - TV_LIFE_BOOK + 1;
    else if (increment) realm = p_ptr->realm2;
    else realm = p_ptr->realm1;

    /* Ask for a spell */
    if (!get_spell(&spell, ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "recite" : "cast"),
        sval, TRUE, realm))
    {
        if (spell == -2)
            msg_format("You don't know any %ss in that book.", prayer);
        return;
    }


    use_realm = tval2realm(o_ptr->tval);

    /* Hex */
    if (use_realm == REALM_HEX)
    {
        if (hex_spelling(spell))
        {
            msg_print("You are already casting it.");
            return;
        }
    }

    if (!is_magic(use_realm))
    {
        s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
    }
    else
    {
        s_ptr = &mp_ptr->info[realm - 1][spell];
    }

    /* Extract mana consumption rate */
    need_mana = mod_need_mana(s_ptr->smana, spell, realm);

    /* Verify "dangerous" spells */
    if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
    {
        if (need_mana > p_ptr->chp)
        {
            msg_print("You do not have enough hp to use this spell.");
            if (flush_failure) flush();
            return;
        }
    }
    else if (need_mana > p_ptr->csp)
    {
        if (flush_failure) flush();

        /* Warning */
        msg_format("You do not have enough mana to %s this %s.",
            ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "recite" : "cast"),
            prayer);


        if (!over_exert) return;

        /* Verify */
        if (!get_check_strict("Attempt it anyway? ", CHECK_OKAY_CANCEL)) return;

    }

    /* Spell failure chance */
    chance = spell_chance(spell, use_realm);

    /* Take spell cost eagerly unless we are exerting ourselves.
       This is to prevent death from using a force weapon with a spell
       that also attacks, like Cyclone.
    */
    if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
    {
        take_mana = 0;
    }
    else
    {
        take_mana = 0;
        if (need_mana <= p_ptr->csp)
        {
            p_ptr->csp -= need_mana;
            take_mana = need_mana;
        }
    }

    /* Take a turn ... Note some spells might have variable
        energy costs, so we allow them to override the default
        value of 100 when handling SPELL_CAST.
    */
    energy_use = 100;

    /* Failed spell */
    if (randint0(100) < chance)
    {
        if (flush_failure) flush();

        msg_format("You failed to cast %s!", do_spell(increment ? p_ptr->realm2 : p_ptr->realm1, spell % 32, SPELL_NAME));

        if (take_mana && prace_is_(RACE_DEMIGOD) && p_ptr->psubrace == DEMIGOD_ATHENA) 
            p_ptr->csp += take_mana/2;

        spell_stats_on_fail_old(realm, spell);
        sound(SOUND_FAIL);

        if (caster_ptr && caster_ptr->on_fail != NULL)
        {
            spell_info hack = {0};
            hack.level = s_ptr->slevel;
            hack.cost = need_mana;
            hack.fail = chance;
            (caster_ptr->on_fail)(&hack);
        }
        if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
            take_hit(DAMAGE_USELIFE, need_mana, "concentrating too hard", -1);

        switch (realm)
        {
        case REALM_LIFE:
            if (randint1(100) < chance) virtue_add(VIRTUE_VITALITY, -1);
            break;
        case REALM_DEATH:
        case REALM_NECROMANCY:
            if (randint1(100) < chance) virtue_add(VIRTUE_UNLIFE, -1);
            break;
        case REALM_NATURE:
            if (randint1(100) < chance) virtue_add(VIRTUE_NATURE, -1);
            break;
        case REALM_DAEMON:
            if (randint1(100) < chance) virtue_add(VIRTUE_JUSTICE, 1);
            break;
        case REALM_CRUSADE:
            if (randint1(100) < chance) virtue_add(VIRTUE_JUSTICE, -1);
            break;
        case REALM_HEX:
            if (randint1(100) < chance) virtue_add(VIRTUE_COMPASSION, -1);
            break;
        default:
            if (randint1(100) < chance) virtue_add(VIRTUE_KNOWLEDGE, -1);
            break;
        }

        /* Failure casting may activate some side effect */
        do_spell(realm, spell, SPELL_FAIL);


        if ((o_ptr->tval == TV_CHAOS_BOOK) && (randint1(100) < spell))
        {
            msg_print("You produce a chaotic effect!");

            wild_magic(spell);
        }
        else if ((o_ptr->tval == TV_DEATH_BOOK) && (randint1(100) < spell))
        {
            if ((sval == 3) && one_in_(2))
            {
                sanity_blast(0, TRUE);
            }
            else
            {
                msg_print("It hurts!");

                take_hit(DAMAGE_LOSELIFE, damroll(o_ptr->sval + 1, 6), "a miscast Death spell", -1);

                if ((spell > 15) && one_in_(6) && !p_ptr->hold_life)
                    lose_exp(spell * 250);
            }
        }
        else if ((o_ptr->tval == TV_MUSIC_BOOK) && (randint1(200) < spell))
        {
msg_print("An infernal sound echoed.");

            aggravate_monsters(0);
        }
        if (randint1(100) >= chance)
            virtue_add(VIRTUE_CHANCE,-1);
    }

    /* Process spell */
    else
    {
        /* Canceled spells cost neither a turn nor mana */
        if (!do_spell(realm, spell, SPELL_CAST))
        {
            /* If we eagerly took mana for this spell, then put it back! */
            if (take_mana > 0)
                p_ptr->csp += take_mana;
            energy_use = 0;
            return;
        }

        spell_stats_on_cast_old(realm, spell);

        if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
            take_hit(DAMAGE_USELIFE, need_mana, "concentrating too hard", -1);

        if (caster_ptr && caster_ptr->on_cast != NULL)
        {
            spell_info hack = {0};
            hack.level = s_ptr->slevel;
            hack.cost = need_mana;
            hack.fail = chance;
            (caster_ptr->on_cast)(&hack);
        }

        if (randint1(100) < chance)
            virtue_add(VIRTUE_CHANCE,1);

        /* A spell was cast */
        if (!(increment ?
            (p_ptr->spell_worked2 & (1L << spell)) :
            (p_ptr->spell_worked1 & (1L << spell)))
            && (p_ptr->pclass != CLASS_SORCERER)
            && (p_ptr->pclass != CLASS_RED_MAGE))
        {
            int e = s_ptr->sexp;

            /* The spell worked */
            if (realm == p_ptr->realm1)
            {
                p_ptr->spell_worked1 |= (1L << spell);
            }
            else
            {
                p_ptr->spell_worked2 |= (1L << spell);
            }

            /* Gain experience */
            gain_exp(e * s_ptr->slevel);

            /* Redraw object recall */
            p_ptr->window |= (PW_OBJECT);

            switch (realm)
            {
            case REALM_LIFE:
                virtue_add(VIRTUE_TEMPERANCE, 1);
                virtue_add(VIRTUE_COMPASSION, 1);
                virtue_add(VIRTUE_VITALITY, 1);
                virtue_add(VIRTUE_DILIGENCE, 1);
                break;
            case REALM_DEATH:
            case REALM_NECROMANCY:
                virtue_add(VIRTUE_UNLIFE, 1);
                virtue_add(VIRTUE_JUSTICE, -1);
                virtue_add(VIRTUE_FAITH, -1);
                virtue_add(VIRTUE_VITALITY, -1);
                break;
            case REALM_DAEMON:
                virtue_add(VIRTUE_JUSTICE, -1);
                virtue_add(VIRTUE_FAITH, -1);
                virtue_add(VIRTUE_HONOUR, -1);
                virtue_add(VIRTUE_TEMPERANCE, -1);
                break;
            case REALM_CRUSADE:
                virtue_add(VIRTUE_FAITH, 1);
                virtue_add(VIRTUE_JUSTICE, 1);
                virtue_add(VIRTUE_SACRIFICE, 1);
                virtue_add(VIRTUE_HONOUR, 1);
                break;
            case REALM_NATURE:
                virtue_add(VIRTUE_NATURE, 1);
                virtue_add(VIRTUE_HARMONY, 1);
                break;
            case REALM_HEX:
                virtue_add(VIRTUE_JUSTICE, -1);
                virtue_add(VIRTUE_FAITH, -1);
                virtue_add(VIRTUE_HONOUR, -1);
                virtue_add(VIRTUE_COMPASSION, -1);
                break;
            default:
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                break;
            }
        }
        switch (realm)
        {
        case REALM_LIFE:
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_TEMPERANCE, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_COMPASSION, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_VITALITY, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_DILIGENCE, 1);
            break;
        case REALM_DEATH:
        case REALM_NECROMANCY:
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_UNLIFE, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_JUSTICE, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_FAITH, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_VITALITY, -1);
            break;
        case REALM_DAEMON:
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_JUSTICE, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_FAITH, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_HONOUR, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_TEMPERANCE, -1);
            break;
        case REALM_CRUSADE:
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_FAITH, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_JUSTICE, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_SACRIFICE, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_HONOUR, 1);
            break;
        case REALM_NATURE:
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_NATURE, 1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_HARMONY, 1);
            break;
        case REALM_HEX:
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_JUSTICE, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_FAITH, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_HONOUR, -1);
            if (randint1(100 + p_ptr->lev) < need_mana) virtue_add(VIRTUE_COMPASSION, -1);
            break;
        }
        if (mp_ptr->spell_xtra & MAGIC_GAIN_EXP)
        {
            s16b cur_exp = p_ptr->spell_exp[(increment ? 32 : 0)+spell];
            s16b exp_gain = 0;
            static s32b last_pexp = 0;
            s32b last_turn = p_ptr->spell_turn[(increment ? 32 : 0)+spell];

            /* Hack: Try to eliminate spell spamming for experience.
               The experience check is for blasting monsters with consecutive Mana Bursts
               which should be granting spell experience, provided one is damaging monsters.
               
               The turn check is for utility spells (Teleport & Detect tactics) since one
               might be doing a bunch of legitimate casting without fighting monsters.

               Note: Androids will probably always fail to pass the xp check! Hmm ...
               Note: One still might be able to macro up a cast followed by a rest command.
            */
            if ( last_pexp != p_ptr->exp 
              || (!p_ptr->inside_quest && game_turn > last_turn + 50 + randint1(50)) )
            {
                if (cur_exp < SPELL_EXP_BEGINNER)
                    exp_gain += 60;
                else if (cur_exp < SPELL_EXP_SKILLED)
                {
                    if ((MAX(base_level, dun_level) > 4) && ((MAX(base_level, dun_level) + 10) > p_ptr->lev))
                        exp_gain = 8;
                }
                else if (cur_exp < SPELL_EXP_EXPERT)
                {
                    if (((MAX(base_level, dun_level) + 5) > p_ptr->lev) && ((MAX(base_level, dun_level) + 5) > s_ptr->slevel))
                        exp_gain = 2;
                }
                else if ((cur_exp < SPELL_EXP_MASTER) && !increment)
                {
                    if (((MAX(base_level, dun_level) + 5) > p_ptr->lev) && (MAX(base_level, dun_level) > s_ptr->slevel))
                        exp_gain = 1;
                }
                p_ptr->spell_exp[(increment ? 32 : 0) + spell] += exp_gain;
            }
            last_pexp = p_ptr->exp;
            p_ptr->spell_turn[(increment ? 32 : 0)+spell] = game_turn;
        }
    }

    /* In general, we already charged the players sp. However, in the event the 
       player knowingly exceeded their csp, then, well, they get what they deserve!
    */
    if (take_mana == 0)
    {
        /* Sufficient mana ... this should no longer fire ... unless we add a spell
           to gain spellpoints, but that seems unlikely ;)  Actually, there is a mincrafter
           spell that might do just that, but I don't think that spell comes thru this fn.
           So it is prudent to double check for overexertion ...
        */
        if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
        {
        }
        else if (need_mana <= p_ptr->csp)
        {
            p_ptr->csp -= need_mana;
        }

        /* Over-exert the player */
        else
        {
            int oops = need_mana;

            /* No mana left */
            p_ptr->csp = 0;
            p_ptr->csp_frac = 0;

            /* Message */
            msg_print("You faint from the effort!");


            /* Hack -- Bypass free action */
            (void)set_paralyzed(randint1(5 * oops + 1), FALSE);

            switch (realm)
            {
            case REALM_LIFE:
                virtue_add(VIRTUE_VITALITY, -10);
                break;
            case REALM_DEATH:
            case REALM_NECROMANCY:
                virtue_add(VIRTUE_UNLIFE, -10);
                break;
            case REALM_DAEMON:
                virtue_add(VIRTUE_JUSTICE, 10);
                break;
            case REALM_NATURE:
                virtue_add(VIRTUE_NATURE, -10);
                break;
            case REALM_CRUSADE:
                virtue_add(VIRTUE_JUSTICE, -10);
                break;
            case REALM_HEX:
                virtue_add(VIRTUE_COMPASSION, 10);
                break;
            default:
                virtue_add(VIRTUE_KNOWLEDGE, -10);
                break;
            }

            /* Damage CON (possibly permanently) */
            if (randint0(100) < 50)
            {
                bool perm = (randint0(100) < 25);

                /* Message */
                msg_print("You have damaged your health!");


                /* Reduce constitution */
                (void)dec_stat(A_CON, 15 + randint1(10), perm);
            }
        }
    }

    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);

    /* Window stuff */
    p_ptr->window |= (PW_SPELL);
}


static bool ang_sort_comp_pet_dismiss(vptr u, vptr v, int a, int b)
{
    u16b *who = (u16b*)(u);

    int w1 = who[a];
    int w2 = who[b];

    monster_type *m_ptr1 = &m_list[w1];
    monster_type *m_ptr2 = &m_list[w2];
    monster_race *r_ptr1 = &r_info[m_ptr1->r_idx];
    monster_race *r_ptr2 = &r_info[m_ptr2->r_idx];

    /* Unused */
    (void)v;

    if (w1 == p_ptr->riding) return TRUE;
    if (w2 == p_ptr->riding) return FALSE;

    if (m_ptr1->nickname && !m_ptr2->nickname) return TRUE;
    if (m_ptr2->nickname && !m_ptr1->nickname) return FALSE;

    if (!m_ptr1->parent_m_idx && m_ptr2->parent_m_idx) return TRUE;
    if (!m_ptr2->parent_m_idx && m_ptr1->parent_m_idx) return FALSE;

    if ((r_ptr1->flags1 & RF1_UNIQUE) && !(r_ptr2->flags1 & RF1_UNIQUE)) return TRUE;
    if ((r_ptr2->flags1 & RF1_UNIQUE) && !(r_ptr1->flags1 & RF1_UNIQUE)) return FALSE;

    if (r_ptr1->level > r_ptr2->level) return TRUE;
    if (r_ptr2->level > r_ptr1->level) return FALSE;

    if (m_ptr1->hp > m_ptr2->hp) return TRUE;
    if (m_ptr2->hp > m_ptr1->hp) return FALSE;
    
    return w1 <= w2;
}

void check_pets_num_and_align(monster_type *m_ptr, bool inc)
{
    s32b old_friend_align = friend_align;
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    if (inc)
    {
        total_friends++;
        if (r_ptr->flags3 & RF3_GOOD) friend_align += r_ptr->level;
        if (r_ptr->flags3 & RF3_EVIL) friend_align -= r_ptr->level;
    }
    else
    {
        total_friends--;
        if (r_ptr->flags3 & RF3_GOOD) friend_align -= r_ptr->level;
        if (r_ptr->flags3 & RF3_EVIL) friend_align += r_ptr->level;
    }

    if (old_friend_align != friend_align) p_ptr->update |= (PU_BONUS);
}

int calculate_upkeep(void)
{
    s32b old_friend_align = friend_align;
    int m_idx;
    bool have_a_unique = FALSE;
    s32b total_friend_levels = 0;

    total_friends = 0;
    friend_align = 0;

    for (m_idx = m_max - 1; m_idx >=1; m_idx--)
    {
        monster_type *m_ptr;
        monster_race *r_ptr;
        
        m_ptr = &m_list[m_idx];
        if (!m_ptr->r_idx) continue;
        r_ptr = &r_info[m_ptr->r_idx];

        if (is_pet(m_ptr))
        {
            total_friends++;
            if (warlock_is_pact_monster(r_ptr))
            {
                total_friend_levels += r_ptr->level/2;
                if (r_ptr->flags1 & RF1_UNIQUE)
                    total_friend_levels += r_ptr->level/2;
            }
            else if (r_ptr->flags1 & RF1_UNIQUE)
            {
                if (p_ptr->pclass == CLASS_CAVALRY || p_ptr->prace == RACE_MON_RING)
                {
                    if (p_ptr->riding == m_idx)
                        total_friend_levels += (r_ptr->level+5)*2;
                    else if (!have_a_unique && (r_info[m_ptr->r_idx].flags7 & RF7_RIDING))
                        total_friend_levels += (r_ptr->level+5)*7/2;
                    else
                        total_friend_levels += (r_ptr->level+5)*10;
                    have_a_unique = TRUE;
                }
                else
                    total_friend_levels += (r_ptr->level+5)*10;
            }
            else
                total_friend_levels += r_ptr->level;

            /* Determine pet alignment */
            if (r_ptr->flags3 & RF3_GOOD) friend_align += r_ptr->level;
            if (r_ptr->flags3 & RF3_EVIL) friend_align -= r_ptr->level;
        }
    }
    if (old_friend_align != friend_align) p_ptr->update |= (PU_BONUS);
    if (total_friends)
    {
        int upkeep_factor;
        int div = get_class()->pets;

        /* Lower divs are better ... I think. */
        if (prace_is_(RACE_DEMIGOD) && p_ptr->psubrace == DEMIGOD_APHRODITE)
            div /= 2;

        if (prace_is_(RACE_MON_QUYLTHULG))
            div = 7;

        if (p_ptr->dragon_realm == DRAGON_REALM_DOMINATION)
            div = 9;

        if (prace_is_(RACE_MON_VAMPIRE))
            div = 10;

        upkeep_factor = (total_friend_levels - (p_ptr->lev * 80 / div));

        if (upkeep_factor < 0) upkeep_factor = 0;
        if (upkeep_factor > 1000) upkeep_factor = 1000;
        return upkeep_factor;
    }
    else
        return 0;
}

void do_cmd_pet_dismiss(void)
{
    monster_type    *m_ptr;
    bool        all_pets = FALSE;
    int pet_ctr, i;
    int Dismissed = 0;

    u16b *who;
    u16b dummy_why;
    int max_pet = 0;
    int cu, cv;

    cu = Term->scr->cu;
    cv = Term->scr->cv;
    Term->scr->cu = 0;
    Term->scr->cv = 1;

    /* Allocate the "who" array */
    C_MAKE(who, max_m_idx, u16b);

    /* Process the monsters (backwards) */
    for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
    {
        if (is_pet(&m_list[pet_ctr]))
            who[max_pet++] = pet_ctr;
    }

    /* Select the sort method */
    ang_sort_comp = ang_sort_comp_pet_dismiss;
    ang_sort_swap = ang_sort_swap_hook;

    ang_sort(who, &dummy_why, max_pet);

    /* Process the monsters (backwards) */
    for (i = 0; i < max_pet; i++)
    {
        bool delete_this;
        char friend_name[MAX_NLEN];
        char buf[512];
        bool kakunin;

        /* Access the monster */
        pet_ctr = who[i];
        m_ptr = &m_list[pet_ctr];

        delete_this = FALSE;
        kakunin = ((pet_ctr == p_ptr->riding) || (m_ptr->nickname));
        monster_desc(friend_name, m_ptr, MD_ASSUME_VISIBLE);

        if (!all_pets)
        {
            /* Hack -- health bar for this monster */
            health_track(pet_ctr);

            /* Hack -- handle stuff */
            handle_stuff();

            sprintf(buf, "Dismiss %s? [Yes/No/Unnamed (%d remain)]", friend_name, max_pet - i);
            prt(buf, 0, 0);

            if (m_ptr->ml)
                move_cursor_relative(m_ptr->fy, m_ptr->fx);

            while (TRUE)
            {
                char ch = inkey();

                if (ch == 'Y' || ch == 'y')
                {
                    delete_this = TRUE;

                    if (kakunin)
                    {
                        sprintf(buf, "Are you sure? (%s) ", friend_name);
                        if (!get_check(buf))
                            delete_this = FALSE;
                    }
                    break;
                }

                if (ch == 'U' || ch == 'u')
                {
                    all_pets = TRUE;
                    break;
                }

                if (ch == ESCAPE || ch == 'N' || ch == 'n')
                    break;

                bell();
            }
        }

        if ((all_pets && !kakunin) || (!all_pets && delete_this))
        {
            if (pet_ctr == p_ptr->riding)
            {
                msg_format("You have got off %s. ", friend_name);
                p_ptr->riding = 0;
                p_ptr->update |= (PU_BONUS | PU_MONSTERS);
                p_ptr->redraw |= (PR_EXTRA | PR_HEALTH_BARS);
            }

            sprintf(buf, "Dismissed %s.", friend_name);

            msg_add(buf);
            p_ptr->window |= (PW_MESSAGE);
            window_stuff();

            delete_monster_idx(pet_ctr);
            Dismissed++;
        }
    }

    Term->scr->cu = cu;
    Term->scr->cv = cv;
    Term_fresh();

    C_KILL(who, max_m_idx, u16b);

    msg_format("You have dismissed %d pet%s.", Dismissed,
           (Dismissed == 1 ? "" : "s"));
    if (Dismissed == 0 && all_pets)
        msg_print("'U'nnamed means all your pets except named pets and your mount.");
}

bool player_can_ride_aux(cave_type *c_ptr, bool now_riding)
{
    bool p_can_enter;
    bool old_character_xtra = character_xtra;
    int  old_riding = p_ptr->riding;
    bool old_riding_ryoute = p_ptr->riding_ryoute;
    bool old_old_riding_ryoute = p_ptr->old_riding_ryoute;
    bool old_pf_ryoute = (p_ptr->pet_extra_flags & PF_RYOUTE) ? TRUE : FALSE;

    /* Hack -- prevent "icky" message */
    character_xtra = TRUE;

    if (now_riding) p_ptr->riding = c_ptr->m_idx;
    else
    {
        p_ptr->riding = 0;
        p_ptr->pet_extra_flags &= ~(PF_RYOUTE);
        p_ptr->riding_ryoute = p_ptr->old_riding_ryoute = FALSE;
    }

    calc_bonuses();

    p_can_enter = player_can_enter(c_ptr->feat, CEM_P_CAN_ENTER_PATTERN);

    p_ptr->riding = old_riding;
    if (old_pf_ryoute) p_ptr->pet_extra_flags |= (PF_RYOUTE);
    else p_ptr->pet_extra_flags &= ~(PF_RYOUTE);
    p_ptr->riding_ryoute = old_riding_ryoute;
    p_ptr->old_riding_ryoute = old_old_riding_ryoute;

    calc_bonuses();

    character_xtra = old_character_xtra;

    return p_can_enter;
}

bool rakuba(int dam, bool force)
{
    int i, y, x, oy, ox;
    int sn = 0, sy = 0, sx = 0;
    char m_name[80];
    monster_type *m_ptr = &m_list[p_ptr->riding];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    bool fall_dam = FALSE;

    if (!p_ptr->riding) return FALSE;
    if (p_ptr->prace == RACE_MON_RING) return FALSE; /* cf ring_process_m instead ... */
    if (p_ptr->wild_mode) return FALSE;

    if (dam >= 0 || force)
    {
        if (!force)
        {
            int cur = skills_riding_current();
            int max = skills_riding_max();
            int ridinglevel = r_ptr->level;
            int rakubalevel = r_ptr->level;
            if (p_ptr->riding_ryoute) rakubalevel += 20;

            if ((cur < max) && (max > 1000) &&
                (dam / 2 + ridinglevel) > (cur / 30 + 10))
            {
                int inc = 0;

                if (ridinglevel > (cur / 100 + 15))
                    inc += 1 + (ridinglevel - cur / 100 - 15);
                else
                    inc += 1;

                p_ptr->skill_exp[SKILL_RIDING] = MIN(max, cur + inc);
            }

            /* see design/riding.ods */
            if (randint0(dam / 2 + rakubalevel * 2) < cur / 30 + 10)
            {
                if (max == RIDING_EXP_MASTER && !p_ptr->riding_ryoute)
                    return FALSE;
                if (!one_in_(p_ptr->lev*(p_ptr->riding_ryoute ? 2 : 3) + 30))
                    return FALSE;
            }
        }

        /* Check around the player */
        for (i = 0; i < 8; i++)
        {
            cave_type *c_ptr;

            /* Access the location */
            y = py + ddy_ddd[i];
            x = px + ddx_ddd[i];

            c_ptr = &cave[y][x];

            if (c_ptr->m_idx) continue;

            /* Skip non-empty grids */
            if (!cave_have_flag_grid(c_ptr, FF_MOVE) && !cave_have_flag_grid(c_ptr, FF_CAN_FLY))
            {
                if (!player_can_ride_aux(c_ptr, FALSE)) continue;
            }

            if (cave_have_flag_grid(c_ptr, FF_PATTERN)) continue;

            /* Count "safe" grids */
            sn++;

            /* Randomize choice */
            if (randint0(sn) > 0) continue;

            /* Save the safe location */
            sy = y; sx = x;
        }
        if (!sn)
        {
            monster_desc(m_name, m_ptr, 0);
            msg_format("You have nearly fallen from %s, but bumped into wall.",m_name);
            take_hit(DAMAGE_NOESCAPE, r_ptr->level+3, "bumping into wall", -1);
            return FALSE;
        }

        oy = py;
        ox = px;

        py = sy;
        px = sx;

        /* Redraw the old spot */
        lite_spot(oy, ox);

        /* Redraw the new spot */
        lite_spot(py, px);

        /* Check for new panel */
        viewport_verify();
    }

    p_ptr->riding = 0;
    p_ptr->pet_extra_flags &= ~(PF_RYOUTE);
    p_ptr->riding_ryoute = p_ptr->old_riding_ryoute = FALSE;

    calc_bonuses();

    p_ptr->update |= (PU_BONUS);

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_MONSTERS);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

    p_ptr->redraw |= (PR_EXTRA);

    /* Update health track of mount */
    p_ptr->redraw |= PR_HEALTH_BARS;

    if (p_ptr->levitation && !force)
    {
        monster_desc(m_name, m_ptr, 0);
        msg_format("You are thrown from %s, but make a good landing.",m_name);
    }
    else
    {
        take_hit(DAMAGE_NOESCAPE, r_ptr->level+3, "Falling from riding", -1);
        fall_dam = TRUE;
    }

    /* Move the player */
    if (sy && !p_ptr->is_dead)
        (void)move_player_effect(py, px, MPE_DONT_PICKUP | MPE_DONT_SWAP_MON);

    return fall_dam;
}

bool do_riding(bool force)
{
    int x, y, dir = 0;
    cave_type *c_ptr;
    monster_type *m_ptr;

    if (!get_rep_dir2(&dir)) return FALSE;
    y = py + ddy[dir];
    x = px + ddx[dir];
    c_ptr = &cave[y][x];

    if (p_ptr->special_defense & KATA_MUSOU) set_action(ACTION_NONE);

    if (p_ptr->riding)
    {
        /* Skip non-empty grids */
        if (!player_can_ride_aux(c_ptr, FALSE))
        {
            msg_print("You cannot go to that direction.");
            return FALSE;
        }

        if (!pattern_seq(py, px, y, x)) return FALSE;

        if (c_ptr->m_idx)
        {
            /* Take a turn */
            energy_use = 100;

            /* Message */
            msg_print("There is a monster in the way!");

            py_attack(y, x, 0);
            return FALSE;
        }

        p_ptr->riding = 0;
        p_ptr->pet_extra_flags &= ~(PF_RYOUTE);
        p_ptr->riding_ryoute = p_ptr->old_riding_ryoute = FALSE;
    }
    else
    {
        if (p_ptr->confused)
        {
            msg_print("You are too confused!");
            return FALSE;
        }

        m_ptr = &m_list[c_ptr->m_idx];

        if (!c_ptr->m_idx || !m_ptr->ml)
        {
            msg_print("Here is no monster.");

            return FALSE;
        }
        if (!is_pet(m_ptr) && !force)
        {
            msg_print("That monster is not a pet.");

            return FALSE;
        }
        if (p_ptr->prace == RACE_MON_RING)
        {
            if (!mon_is_type(m_ptr->r_idx, SUMMON_RING_BEARER))
            {
                msg_print("This monster is not a suitable ring bearer.");
                return FALSE;
            }
        }
        else
        {
            if (!(r_info[m_ptr->r_idx].flags7 & RF7_RIDING))
            {
                msg_print("This monster doesn't seem suitable for riding.");

                return FALSE;
            }
            if (warlock_is_(WARLOCK_DRAGONS) && !(r_info[m_ptr->r_idx].flags3 & RF3_DRAGON))
            {
                msg_print("You are a dragon rider!");
                return FALSE;
            }
        }

        if (!pattern_seq(py, px, y, x)) return FALSE;

        if (!player_can_ride_aux(c_ptr, TRUE))
        {
            /* Feature code (applying "mimic" field) */
            feature_type *f_ptr = &f_info[get_feat_mimic(c_ptr)];
            msg_format("This monster is %s the %s.",
                       ((!have_flag(f_ptr->flags, FF_MOVE) && !have_flag(f_ptr->flags, FF_CAN_FLY)) ||
                        (!have_flag(f_ptr->flags, FF_LOS) && !have_flag(f_ptr->flags, FF_TREE))) ?
                       "in" : "on", f_name + f_ptr->name);

            return FALSE;
        }
        if ( p_ptr->prace != RACE_MON_RING 
          && r_info[m_ptr->r_idx].level > randint1((skills_riding_current() / 50 + p_ptr->lev / 2 + 20)))
        {
            msg_print("You failed to ride.");

            energy_use = 100;

            return FALSE;
        }

        if (MON_CSLEEP(m_ptr))
        {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);
            (void)set_monster_csleep(c_ptr->m_idx, 0);
            msg_format("You have waked %s up.", m_name);
        }

        if (p_ptr->action == ACTION_KAMAE) set_action(ACTION_NONE);
        if (p_ptr->action == ACTION_GLITTER) set_action(ACTION_NONE);

        p_ptr->riding = c_ptr->m_idx;

        /* Hack -- remove tracked monster */
        if (p_ptr->riding == p_ptr->health_who) health_track(0);
    }

    energy_use = 100;

    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update the monsters */
    p_ptr->update |= (PU_BONUS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_EXTRA);

    p_ptr->redraw |= PR_HEALTH_BARS;

    /* Move the player */
    (void)move_player_effect(y, x, MPE_HANDLE_STUFF | MPE_ENERGY_USE | MPE_DONT_PICKUP | MPE_DONT_SWAP_MON);

    return TRUE;
}

static void do_name_pet(void)
{
    monster_type *m_ptr;
    char out_val[20];
    char m_name[80];
    bool old_target_pet = target_pet;

    target_pet = TRUE;
    if (!target_set(TARGET_KILL))
    {
        target_pet = old_target_pet;
        return;
    }
    target_pet = old_target_pet;

    if (cave[target_row][target_col].m_idx)
    {
        m_ptr = &m_list[cave[target_row][target_col].m_idx];

        if (!is_pet(m_ptr))
        {
            /* Message */
            msg_format("This monster is not a pet.");
            return;
        }
        if (r_info[m_ptr->r_idx].flags1 & RF1_UNIQUE)
        {
            msg_format("You cannot change name of this monster!");
            return;
        }
        monster_desc(m_name, m_ptr, 0);

        /* Message */
        msg_format("Name %s.", m_name);

        msg_print(NULL);

        /* Start with nothing */
        strcpy(out_val, "");

        /* Use old inscription */
        if (m_ptr->nickname)
        {
            /* Start with the old inscription */
            strcpy(out_val, quark_str(m_ptr->nickname));
        }

        /* Get a new inscription (possibly empty) */
        if (get_string("Name: ", out_val, 15))
        {
            if (out_val[0])
            {
                m_ptr->nickname = quark_add(out_val);
            }
            else
            {
                m_ptr->nickname = 0;
            }
        }
    }
}

/*
 * Issue a pet command
 */
void do_cmd_pet(void)
{
    int            i = 0;
    int            num;
    int            powers[36];
    cptr            power_desc[36];
    bool            flag, redraw;
    int            ask = FALSE;
    char            choice;
    char            out_val[160];
    int            pet_ctr;
    monster_type    *m_ptr;

    int mode = 0;

    char buf[160];
    char target_buf[160];

    int menu_line = use_menu ? 1 : 0;

    num = 0;

    power_desc[num] = "dismiss pets";

    powers[num++] = PET_DISMISS;

    sprintf(target_buf, "specify a target of pet (now:%s)",
        (pet_t_m_idx ? (p_ptr->image ? "something strange" : (r_name + r_info[m_list[pet_t_m_idx].ap_r_idx].name)) : "nothing"));
    power_desc[num] = target_buf;

    powers[num++] = PET_TARGET;

    power_desc[num] = "stay close";

    if (p_ptr->pet_follow_distance == PET_CLOSE_DIST) mode = num;
    powers[num++] = PET_STAY_CLOSE;

    power_desc[num] = "follow me";

    if (p_ptr->pet_follow_distance == PET_FOLLOW_DIST) mode = num;
    powers[num++] = PET_FOLLOW_ME;

    power_desc[num] = "seek and destroy";

    if (p_ptr->pet_follow_distance == PET_DESTROY_DIST) mode = num;
    powers[num++] = PET_SEEK_AND_DESTROY;

    power_desc[num] = "give me space";

    if (p_ptr->pet_follow_distance == PET_SPACE_DIST) mode = num;
    powers[num++] = PET_ALLOW_SPACE;

    power_desc[num] = "stay away";

    if (p_ptr->pet_follow_distance == PET_AWAY_DIST) mode = num;
    powers[num++] = PET_STAY_AWAY;

    if (p_ptr->pet_extra_flags & PF_OPEN_DOORS)
    {
        power_desc[num] = "pets open doors (now On)";
    }
    else
    {
        power_desc[num] = "pets open doors (now Off)";
    }
    powers[num++] = PET_OPEN_DOORS;

    if (p_ptr->pet_extra_flags & PF_PICKUP_ITEMS)
    {
        power_desc[num] = "pets pick up items (now On)";
    }
    else
    {
        power_desc[num] = "pets pick up items (now Off)";
    }
    powers[num++] = PET_TAKE_ITEMS;

    if (p_ptr->pet_extra_flags & PF_TELEPORT)
    {
        power_desc[num] = "allow teleport (now On)";
    }
    else
    {
        power_desc[num] = "allow teleport (now Off)";
    }
    powers[num++] = PET_TELEPORT;

    if (p_ptr->pet_extra_flags & PF_ATTACK_SPELL)
    {
        power_desc[num] = "allow cast attack spell (now On)";
    }
    else
    {
        power_desc[num] = "allow cast attack spell (now Off)";
    }
    powers[num++] = PET_ATTACK_SPELL;

    if (p_ptr->pet_extra_flags & PF_SUMMON_SPELL)
    {
        power_desc[num] = "allow cast summon spell (now On)";
    }
    else
    {
        power_desc[num] = "allow cast summon spell (now Off)";
    }
    powers[num++] = PET_SUMMON_SPELL;

    if (p_ptr->pet_extra_flags & PF_BALL_SPELL)
    {
        power_desc[num] = "allow involve player in area spell (now On)";
    }
    else
    {
        power_desc[num] = "allow involve player in area spell (now Off)";
    }
    powers[num++] = PET_BALL_SPELL;

    if (p_ptr->riding)
    {
        power_desc[num] = "get off a pet";
    }
    else
    {
        power_desc[num] = "ride a pet";
    }
    powers[num++] = PET_RIDING;

    power_desc[num] = "name pets";

    powers[num++] = PET_NAME;

    if (p_ptr->riding && p_ptr->prace != RACE_MON_RING)
    {
        /* TODO: We used to check weapons to see if 2-handed was an option ... */
        if (p_ptr->pet_extra_flags & PF_RYOUTE)
            power_desc[num] = "use one hand to control a riding pet";
        else
            power_desc[num] = "use both hands for a weapon";

        powers[num++] = PET_RYOUTE;
    }

    if (p_ptr->pet_extra_flags & PF_NO_BREEDING)
    {
        power_desc[num] = "no breeding (now On)";
    }
    else
    {
        power_desc[num] = "no breeding (now Off)";
    }
    powers[num++] = PET_NO_BREEDING;

    if (!use_graphics)
    {
        if (p_ptr->pet_extra_flags & PF_HILITE)
        {
            power_desc[num] = "highlight pets (now On)";
        }
        else
        {
            power_desc[num] = "highlight pets (now Off)";
        }
        powers[num++] = PET_HILITE;
    }

#ifdef ALLOW_REPEAT
    if (!(repeat_pull(&i) && (i >= 0) && (i < num)))
    {
#endif /* ALLOW_REPEAT */

    /* Nothing chosen yet */
    flag = FALSE;

    /* No redraw yet */
    redraw = FALSE;

    if (use_menu)
    {
        /* Save the screen */
        screen_save();

        /* Build a prompt */
        strnfmt(out_val, 78, "(Command, ESC=exit) Choose command from menu.");
    }
    else
    {
        /* Build a prompt */
        strnfmt(out_val, 78,
                "(Command %c-%c, *=List, ESC=exit) Select a command: ",
                I2A(0), I2A(num - 1));
    }

    choice = (always_show_list || use_menu) ? ESCAPE : 1;

    /* Get a command from the user */
    while (!flag)
    {
        if (choice == ESCAPE) choice = ' ';
        else if (!get_com(out_val, &choice, TRUE)) break;

        if (use_menu && (choice != ' '))
        {
            switch (choice)
            {
            case '0':
                screen_load();
                return;

            case '8':
            case 'k':
            case 'K':
                menu_line += (num - 1);
                break;

            case '2':
            case 'j':
            case 'J':
                menu_line++;
                break;

            case '4':
            case 'h':
            case 'H':
                menu_line = 1;
                break;

            case '6':
            case 'l':
            case 'L':
                menu_line = num;
                break;

            case 'x':
            case 'X':
            case '\r':
            case '\n':
                i = menu_line - 1;
                ask = FALSE;
                break;
            }
            if (menu_line > num) menu_line -= num;
        }

        /* Request redraw */
        if ((choice == ' ') || (choice == '*') || (choice == '?') || (use_menu && ask))
        {
            /* Show the list */
            if (!redraw || use_menu)
            {
                byte y = 1, x = 0;
                int ctr = 0;

                /* Show list */
                redraw = TRUE;

                /* Save the screen */
                if (!use_menu) screen_save();

                prt("", y++, x);

                /* Print list */
                for (ctr = 0; ctr < num; ctr++)
                {
                    /* Letter/number for power selection */
                    if (use_menu) sprintf(buf, "%c%s ", (ctr == mode) ? '*' : ' ', (ctr == (menu_line - 1)) ? "> " : "  ");
                    else sprintf(buf, "%c%c) ", (ctr == mode) ? '*' : ' ', I2A(ctr));

                    strcat(buf, power_desc[ctr]);

                    prt(buf, y + ctr, x);
                }

                prt("", y + MIN(ctr, 17), x);
            }

            /* Hide the list */
            else
            {
                /* Hide list */
                redraw = FALSE;

                /* Restore the screen */
                screen_load();
            }

            /* Redo asking */
            continue;
        }

        if (!use_menu)
        {
            /* Note verify */
            ask = (isupper(choice));

            /* Lowercase */
            if (ask) choice = tolower(choice);

            /* Extract request */
            i = (islower(choice) ? A2I(choice) : -1);
        }

        /* Totally Illegal */
        if ((i < 0) || (i >= num))
        {
            bell();
            continue;
        }

        /* Verify it */
        if (ask)
        {
            /* Prompt */
            strnfmt(buf, 78, "Use %s? ", power_desc[i]);

            /* Belay that order */
            if (!get_check(buf)) continue;
        }

        /* Stop the loop */
        flag = TRUE;
    }

    /* Restore the screen */
    if (redraw) screen_load();

    /* Abort if needed */
    if (!flag)
    {
        energy_use = 0;
        return;
    }

#ifdef ALLOW_REPEAT
    repeat_push(i);
    }
#endif /* ALLOW_REPEAT */

    switch (powers[i])
    {
        case PET_DISMISS: /* Dismiss pets */
        {
            /* Check pets (backwards) */
            for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
            {
                /* Player has pet */
                if (is_pet(&m_list[pet_ctr])) break;
            }

            if (!pet_ctr)
            {
                msg_print("You have no pets!");
                break;
            }
            do_cmd_pet_dismiss();
            (void)calculate_upkeep();
            break;
        }
        case PET_TARGET:
        {
            project_length = -1;
            target_pet = FALSE;
            if (!target_set(TARGET_MARK)) 
                pet_t_m_idx = 0;
            else
            {
                if (target_who > 0)
                    pet_t_m_idx = target_who;
                else
                    pet_t_m_idx = cave[target_row][target_col].m_idx;
            }
            project_length = 0;

            break;
        }
        /* Call pets */
        case PET_STAY_CLOSE:
        {
            p_ptr->pet_follow_distance = PET_CLOSE_DIST;
            pet_t_m_idx = 0;
            break;
        }        
        /* "Follow Me" */
        case PET_FOLLOW_ME:
        {
            p_ptr->pet_follow_distance = PET_FOLLOW_DIST;
            pet_t_m_idx = 0;
            break;
        }
        /* "Seek and destoy" */
        case PET_SEEK_AND_DESTROY:
        {
            p_ptr->pet_follow_distance = PET_DESTROY_DIST;
            break;
        }
        /* "Give me space" */
        case PET_ALLOW_SPACE:
        {
            p_ptr->pet_follow_distance = PET_SPACE_DIST;
            break;
        }
        /* "Stay away" */
        case PET_STAY_AWAY:
        {
            p_ptr->pet_follow_distance = PET_AWAY_DIST;
            break;
        }
        /* flag - allow pets to open doors */
        case PET_OPEN_DOORS:
        {
            if (p_ptr->pet_extra_flags & PF_OPEN_DOORS) p_ptr->pet_extra_flags &= ~(PF_OPEN_DOORS);
            else p_ptr->pet_extra_flags |= (PF_OPEN_DOORS);
            break;
        }
        /* flag - allow pets to pickup items */
        case PET_TAKE_ITEMS:
        {
            if (p_ptr->pet_extra_flags & PF_PICKUP_ITEMS)
            {
                p_ptr->pet_extra_flags &= ~(PF_PICKUP_ITEMS);
                for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
                {
                    /* Access the monster */
                    m_ptr = &m_list[pet_ctr];

                    if (is_pet(m_ptr))
                    {
                        monster_drop_carried_objects(m_ptr);
                    }
                }
            }
            else p_ptr->pet_extra_flags |= (PF_PICKUP_ITEMS);

            break;
        }
        /* flag - allow pets to teleport */
        case PET_TELEPORT:
        {
            if (p_ptr->pet_extra_flags & PF_TELEPORT) p_ptr->pet_extra_flags &= ~(PF_TELEPORT);
            else p_ptr->pet_extra_flags |= (PF_TELEPORT);
            break;
        }
        /* flag - allow pets to cast attack spell */
        case PET_ATTACK_SPELL:
        {
            if (p_ptr->pet_extra_flags & PF_ATTACK_SPELL) p_ptr->pet_extra_flags &= ~(PF_ATTACK_SPELL);
            else p_ptr->pet_extra_flags |= (PF_ATTACK_SPELL);
            break;
        }
        /* flag - allow pets to cast attack spell */
        case PET_SUMMON_SPELL:
        {
            if (p_ptr->pet_extra_flags & PF_SUMMON_SPELL) p_ptr->pet_extra_flags &= ~(PF_SUMMON_SPELL);
            else p_ptr->pet_extra_flags |= (PF_SUMMON_SPELL);
            break;
        }
        /* flag - allow pets to cast attack spell */
        case PET_BALL_SPELL:
        {
            if (p_ptr->pet_extra_flags & PF_BALL_SPELL) p_ptr->pet_extra_flags &= ~(PF_BALL_SPELL);
            else p_ptr->pet_extra_flags |= (PF_BALL_SPELL);
            break;
        }

        case PET_RIDING:
        {
            (void)do_riding(FALSE);
            break;
        }

        case PET_NAME:
        {
            do_name_pet();
            break;
        }

        case PET_RYOUTE:
        {
            if (p_ptr->pet_extra_flags & PF_RYOUTE) p_ptr->pet_extra_flags &= ~(PF_RYOUTE);
            else p_ptr->pet_extra_flags |= (PF_RYOUTE);
            p_ptr->update |= (PU_BONUS);
            handle_stuff();
            break;
        }
        case PET_NO_BREEDING:
        {
            if (p_ptr->pet_extra_flags & PF_NO_BREEDING) p_ptr->pet_extra_flags &= ~(PF_NO_BREEDING);
            else p_ptr->pet_extra_flags |= PF_NO_BREEDING;
            break;
        }
        case PET_HILITE:
        {
            if (p_ptr->pet_extra_flags & PF_HILITE) p_ptr->pet_extra_flags &= ~(PF_HILITE);
            else p_ptr->pet_extra_flags |= PF_HILITE;
            p_ptr->redraw |= PR_MAP;
            break;
        }
    }
}
