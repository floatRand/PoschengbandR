/* File: mspells1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Monster spells (attack player) */

#include "angband.h"


/*
 * And now for Intelligent monster attacks (including spells).
 *
 * Original idea and code by "DRS" (David Reeves Sward).
 * Major modifications by "BEN" (Ben Harrison).
 *
 * Give monsters more intelligent attack/spell selection based on
 * observations of previous attacks on the player, and/or by allowing
 * the monster to "cheat" and know the player status.
 *
 * Maintain an idea of the player status, and use that information
 * to occasionally eliminate "ineffective" spell attacks. We could
 * also eliminate ineffective normal attacks, but there is no reason
 * for the monster to do this, since he gains no benefit.
 * Note that MINDLESS monsters are not allowed to use this code.
 * And non-INTELLIGENT monsters only use it partially effectively.
 *
 * Actually learn what the player resists, and use that information
 * to remove attacks or spells before using them. This will require
 * much less space, if I am not mistaken. Thus, each monster gets a
 * set of 32 bit flags, "smart", build from the various "SM_*" flags.
 *
 * This has the added advantage that attacks and spells are related.
 * The "smart_learn" option means that the monster "learns" the flags
 * that should be set, and "smart_cheat" means that he "knows" them.
 * So "smart_cheat" means that the "smart" field is always up to date,
 * while "smart_learn" means that the "smart" field is slowly learned.
 * Both of them have the same effect on the "choose spell" routine.
 */

/*
 * Internal probability routine
 */
static bool int_outof(monster_race *r_ptr, int prob)
{
    /* Non-Smart monsters are half as "smart" */
    if (!(r_ptr->flags2 & RF2_SMART)) prob = prob / 2;

    /* Roll the dice */
    return (randint0(100) < prob);
}



/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, u32b *f4p, u32b *f5p, u32b *f6p)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    u32b f4 = (*f4p);
    u32b f5 = (*f5p);
    u32b f6 = (*f6p);

    u32b smart = 0L;


    /* Too stupid to know anything */
    if (r_ptr->flags2 & RF2_STUPID) return;


    /* Must be cheating or learning */
    if (!smart_cheat && !smart_learn) return;


    /* Update acquired knowledge */
    if (smart_learn)
    {
        /* Hack -- Occasionally forget player status */
        /* Only save SM_FRIENDLY, SM_PET or SM_CLONED */
        if (m_ptr->smart && (randint0(100) < 1)) m_ptr->smart &= (SM_FRIENDLY | SM_PET | SM_CLONED);

        /* Use the memorized flags */
        smart = m_ptr->smart;
    }


    /* Cheat if requested */
    if (smart_cheat)
    {
        int pct;
        /* Know basic info ... Unfortunately, the resistance system has changed! */
        pct = res_pct(RES_ACID);
        if (pct >= 50) smart |= (SM_RES_ACID);
        if (pct >= 65) smart |= (SM_OPP_ACID);
        if (pct >= 100) smart |= (SM_IMM_ACID);

        pct = res_pct(RES_ELEC);
        if (pct >= 50) smart |= (SM_RES_ELEC);
        if (pct >= 65) smart |= (SM_OPP_ELEC);
        if (pct >= 100) smart |= (SM_IMM_ELEC);

        pct = res_pct(RES_FIRE);
        if (pct >= 50) smart |= (SM_RES_FIRE);
        if (pct >= 65) smart |= (SM_OPP_FIRE);
        if (pct >= 100) smart |= (SM_IMM_FIRE);

        pct = res_pct(RES_COLD);
        if (pct >= 50) smart |= (SM_RES_COLD);
        if (pct >= 65) smart |= (SM_OPP_COLD);
        if (pct >= 100) smart |= (SM_IMM_COLD);

        pct = res_pct(RES_POIS);
        if (pct >= 50) smart |= (SM_RES_POIS);
        if (pct >= 65) smart |= (SM_OPP_POIS);

        if (res_pct(RES_NETHER) >= 50) smart |= (SM_RES_NETH);
        if (res_pct(RES_LITE) >= 50) smart |= (SM_RES_LITE);
        if (res_pct(RES_DARK) >= 50) smart |= (SM_RES_DARK);
        if (res_pct(RES_FEAR) >= 50) smart |= (SM_RES_FEAR);
        if (res_pct(RES_CONF) >= 50) smart |= (SM_RES_CONF);
        if (res_pct(RES_CHAOS) >= 50) smart |= (SM_RES_CHAOS);
        if (res_pct(RES_DISEN) >= 50) smart |= (SM_RES_DISEN);
        if (res_pct(RES_BLIND) >= 50) smart |= (SM_RES_BLIND);
        if (res_pct(RES_NEXUS) >= 50) smart |= (SM_RES_NEXUS);
        if (res_pct(RES_SOUND) >= 50) smart |= (SM_RES_SOUND);
        if (res_pct(RES_SHARDS) >= 50) smart |= (SM_RES_SHARD);
        if (p_ptr->reflect) smart |= (SM_IMM_REFLECT);

        /* Know bizarre "resistances" */
        if (p_ptr->free_act) smart |= (SM_IMM_FREE);
        if (!p_ptr->msp) smart |= (SM_IMM_MANA);
    }


    /* Nothing known */
    if (!smart) return;


    if (smart & SM_IMM_ACID)
    {
        f4 &= ~(RF4_BR_ACID);
        f5 &= ~(RF5_BA_ACID);
        f5 &= ~(RF5_BO_ACID);
    }
    else if ((smart & (SM_OPP_ACID)) && (smart & (SM_RES_ACID)))
    {
        if (int_outof(r_ptr, 80)) f4 &= ~(RF4_BR_ACID);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BA_ACID);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BO_ACID);
    }
    else if ((smart & (SM_OPP_ACID)) || (smart & (SM_RES_ACID)))
    {
        if (int_outof(r_ptr, 30)) f4 &= ~(RF4_BR_ACID);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BA_ACID);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BO_ACID);
    }


    if (smart & (SM_IMM_ELEC))
    {
        f4 &= ~(RF4_BR_ELEC);
        f5 &= ~(RF5_BA_ELEC);
        f5 &= ~(RF5_BO_ELEC);
    }
    else if ((smart & (SM_OPP_ELEC)) && (smart & (SM_RES_ELEC)))
    {
        if (int_outof(r_ptr, 80)) f4 &= ~(RF4_BR_ELEC);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BA_ELEC);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BO_ELEC);
    }
    else if ((smart & (SM_OPP_ELEC)) || (smart & (SM_RES_ELEC)))
    {
        if (int_outof(r_ptr, 30)) f4 &= ~(RF4_BR_ELEC);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BA_ELEC);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BO_ELEC);
    }


    if (smart & (SM_IMM_FIRE))
    {
        f4 &= ~(RF4_BR_FIRE);
        f5 &= ~(RF5_BA_FIRE);
        f5 &= ~(RF5_BO_FIRE);
    }
    else if ((smart & (SM_OPP_FIRE)) && (smart & (SM_RES_FIRE)))
    {
        if (int_outof(r_ptr, 80)) f4 &= ~(RF4_BR_FIRE);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BA_FIRE);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BO_FIRE);
    }
    else if ((smart & (SM_OPP_FIRE)) || (smart & (SM_RES_FIRE)))
    {
        if (int_outof(r_ptr, 30)) f4 &= ~(RF4_BR_FIRE);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BA_FIRE);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BO_FIRE);
    }


    if (smart & (SM_IMM_COLD))
    {
        f4 &= ~(RF4_BR_COLD);
        f5 &= ~(RF5_BA_COLD);
        f5 &= ~(RF5_BO_COLD);
        f5 &= ~(RF5_BO_ICEE);
    }
    else if ((smart & (SM_OPP_COLD)) && (smart & (SM_RES_COLD)))
    {
        if (int_outof(r_ptr, 80)) f4 &= ~(RF4_BR_COLD);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BA_COLD);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BO_COLD);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BO_ICEE);
    }
    else if ((smart & (SM_OPP_COLD)) || (smart & (SM_RES_COLD)))
    {
        if (int_outof(r_ptr, 30)) f4 &= ~(RF4_BR_COLD);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BA_COLD);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BO_COLD);
        if (int_outof(r_ptr, 20)) f5 &= ~(RF5_BO_ICEE);
    }


    if ((smart & (SM_OPP_POIS)) && (smart & (SM_RES_POIS)))
    {
        if (int_outof(r_ptr, 80)) f4 &= ~(RF4_BR_POIS);
        if (int_outof(r_ptr, 80)) f5 &= ~(RF5_BA_POIS);
        if (int_outof(r_ptr, 60)) f4 &= ~(RF4_BA_NUKE);
        if (int_outof(r_ptr, 60)) f4 &= ~(RF4_BR_NUKE);
    }
    else if ((smart & (SM_OPP_POIS)) || (smart & (SM_RES_POIS)))
    {
        if (int_outof(r_ptr, 30)) f4 &= ~(RF4_BR_POIS);
        if (int_outof(r_ptr, 30)) f5 &= ~(RF5_BA_POIS);
    }


    if (smart & (SM_RES_NETH))
    {
        if (prace_is_(RACE_SPECTRE))
        {
            f4 &= ~(RF4_BR_NETH);
            f5 &= ~(RF5_BA_NETH);
            f5 &= ~(RF5_BO_NETH);
        }
        else
        {
            if (int_outof(r_ptr, 20)) f4 &= ~(RF4_BR_NETH);
            if (int_outof(r_ptr, 50)) f5 &= ~(RF5_BA_NETH);
            if (int_outof(r_ptr, 50)) f5 &= ~(RF5_BO_NETH);
        }
    }

    if (smart & (SM_RES_LITE))
    {
        if (int_outof(r_ptr, 50)) f4 &= ~(RF4_BR_LITE);
        if (int_outof(r_ptr, 50)) f5 &= ~(RF5_BA_LITE);
    }

    if (smart & (SM_RES_DARK))
    {
        if (int_outof(r_ptr, 50)) f4 &= ~(RF4_BR_DARK);
        if (int_outof(r_ptr, 50)) f5 &= ~(RF5_BA_DARK);
    }

    if (smart & (SM_RES_FEAR))
    {
    /*    f5 &= ~(RF5_SCARE); */
    }

    if (smart & (SM_RES_CONF))
    {
        f5 &= ~(RF5_CONF);
        if (int_outof(r_ptr, 50)) f4 &= ~(RF4_BR_CONF);
    }

    if (smart & (SM_RES_CHAOS))
    {
        if (int_outof(r_ptr, 20)) f4 &= ~(RF4_BR_CHAO);
        if (int_outof(r_ptr, 50)) f4 &= ~(RF4_BA_CHAO);
    }

    if (smart & (SM_RES_DISEN))
    {
        if (int_outof(r_ptr, 40)) f4 &= ~(RF4_BR_DISE);
    }

    if (smart & (SM_RES_BLIND))
    {
        f5 &= ~(RF5_BLIND);
    }

    if (smart & (SM_RES_NEXUS))
    {
        if (int_outof(r_ptr, 50)) f4 &= ~(RF4_BR_NEXU);
        f6 &= ~(RF6_TELE_LEVEL);
    }

    if (smart & (SM_RES_SOUND))
    {
        if (int_outof(r_ptr, 50)) f4 &= ~(RF4_BR_SOUN);
    }

    if (smart & (SM_RES_SHARD))
    {
        if (int_outof(r_ptr, 40)) f4 &= ~(RF4_BR_SHAR);
    }

    if (smart & (SM_IMM_REFLECT))
    {
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_COLD);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_FIRE);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_ACID);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_ELEC);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_NETH);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_WATE);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_MANA);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_PLAS);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_BO_ICEE);
        if (int_outof(r_ptr, 150)) f5 &= ~(RF5_MISSILE);
        if (m_ptr->r_idx != MON_ARTEMIS)
        {
            if (int_outof(r_ptr, 150)) f4 &= ~(RF4_SHOOT);
        }
    }

    if (smart & (SM_IMM_FREE))
    {
        f5 &= ~(RF5_HOLD);
        f5 &= ~(RF5_SLOW);
    }

    if (smart & (SM_IMM_MANA))
    {
        f5 &= ~(RF5_DRAIN_MANA);
    }

    /* XXX XXX XXX No spells left? */
    /* if (!f4 && !f5 && !f6) ... */

    (*f4p) = f4;
    (*f5p) = f5;
    (*f6p) = f6;
}


/*
 * Determine if there is a space near the player in which
 * a summoned creature can appear
 */
bool summon_possible(int y1, int x1)
{
    int y, x;

    /* Start at the player's location, and check 2 grids in each dir */
    for (y = y1 - 2; y <= y1 + 2; y++)
    {
        for (x = x1 - 2; x <= x1 + 2; x++)
        {
            /* Ignore illegal locations */
            if (!in_bounds(y, x)) continue;

            /* Only check a circular area */
            if (distance(y1, x1, y, x)>2) continue;

            /* ...nor on the Pattern */
            if (pattern_tile(y, x)) continue;

            /* Require empty floor grid in line of projection */
            if (cave_empty_bold(y, x) && projectable(y1, x1, y, x) && projectable(y, x, y1, x1)) return (TRUE);
        }
    }

    return FALSE;
}


bool raise_possible(monster_type *m_ptr)
{
    int xx, yy;
    int y = m_ptr->fy;
    int x = m_ptr->fx;
    s16b this_o_idx, next_o_idx = 0;
    cave_type *c_ptr;

    for (xx = x - 5; xx <= x + 5; xx++)
    {
        for (yy = y - 5; yy <= y + 5; yy++)
        {
            if (distance(y, x, yy, xx) > 5) continue;
            if (!los(y, x, yy, xx)) continue;
            if (!projectable(y, x, yy, xx)) continue;

            c_ptr = &cave[yy][xx];
            /* Scan the pile of objects */
            for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
            {
                /* Acquire object */
                object_type *o_ptr = &o_list[this_o_idx];

                /* Acquire next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Known to be worthless? */
                if (o_ptr->tval == TV_CORPSE)
                {
                    if (!monster_has_hostile_align(m_ptr, 0, 0, &r_info[o_ptr->pval])) return TRUE;
                }
            }
        }
    }
    return FALSE;
}


/*
 * Originally, it was possible for a friendly to shoot another friendly.
 * Change it so a "clean shot" means no equally friendly monster is
 * between the attacker and target.
 */
/*
 * Determine if a bolt spell will hit the player.
 *
 * This is exactly like "projectable", but it will
 * return FALSE if a monster is in the way.
 * no equally friendly monster is
 * between the attacker and target.
 */
bool clean_shot(int y1, int x1, int y2, int x2, bool friend)
{
    /* Must be the same as projectable() */

    int i, y, x;

    int grid_n = 0;
    u16b grid_g[512];

    /* Check the projection path */
    grid_n = project_path(grid_g, MAX_RANGE, y1, x1, y2, x2, 0);

    /* No grid is ever projectable from itself */
    if (!grid_n) return (FALSE);

    /* Final grid */
    y = GRID_Y(grid_g[grid_n-1]);
    x = GRID_X(grid_g[grid_n-1]);

    /* May not end in an unrequested grid */
    if ((y != y2) || (x != x2)) return (FALSE);

    for (i = 0; i < grid_n; i++)
    {
        y = GRID_Y(grid_g[i]);
        x = GRID_X(grid_g[i]);

        if ((cave[y][x].m_idx > 0) && !((y == y2) && (x == x2)))
        {
            monster_type *m_ptr = &m_list[cave[y][x].m_idx];
            if (friend == is_pet(m_ptr))
            {
                return (FALSE);
            }
        }
        /* Pets may not shoot through the character - TNB */
        if (player_bold(y, x))
        {
            if (friend) return (FALSE);
        }
    }

    return (TRUE);
}

/*
 * Cast a bolt at the player
 * Stop if we hit a monster
 * Affect monsters and the player
 */
static void bolt(int m_idx, int typ, int dam_hp, int monspell, bool learnable)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_PLAYER | PROJECT_REFLECTABLE;

    /* Target the player with a bolt attack */
    (void)project(m_idx, 0, py, px, dam_hp, typ, flg, (learnable ? monspell : -1));
}

static void artemis_bolt(int m_idx, int typ, int dam_hp, int monspell, bool learnable)
{
    int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_PLAYER;

    /* Target the player with a bolt attack */
    (void)project(m_idx, 0, py, px, dam_hp, typ, flg, (learnable ? monspell : -1));
}

static void beam(int m_idx, int typ, int dam_hp, int monspell, bool learnable)
{
    int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_THRU | PROJECT_PLAYER;

    /* Target the player with a bolt attack */
    (void)project(m_idx, 0, py, px, dam_hp, typ, flg, (learnable ? monspell : -1));
}


/*
 * Cast a breath (or ball) attack at the player
 * Pass over any monsters that may be in the way
 * Affect grids, objects, monsters, and the player
 */
static void breath(int y, int x, int m_idx, int typ, int dam_hp, int rad, bool breath, int monspell, bool learnable)
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_PLAYER;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* Determine the radius of the blast */
    if ((rad < 1) && breath) rad = (r_ptr->flags2 & (RF2_POWERFUL)) ? 3 : 2;

    /* Handle breath attacks */
    if (breath) rad = 0 - rad;

    switch (typ)
    {
    case GF_ROCKET:
        flg |= PROJECT_STOP;
        break;
    case GF_DRAIN_MANA:
    case GF_MIND_BLAST:
    case GF_BRAIN_SMASH:
    case GF_CAUSE_1:
    case GF_CAUSE_2:
    case GF_CAUSE_3:
    case GF_CAUSE_4:
    case GF_HAND_DOOM:
        flg |= (PROJECT_HIDE | PROJECT_AIMED);
        break;
    }

    /* Target the player with a ball attack */
    (void)project(m_idx, rad, y, x, dam_hp, typ, flg, (learnable ? monspell : -1));
}


u32b get_curse(int power, object_type *o_ptr)
{
    u32b new_curse;

    while(1)
    {
        new_curse = (1 << (randint0(MAX_CURSE)+4));
        if (power == 2)
        {
            if (!(new_curse & TRC_HEAVY_MASK)) continue;
        }
        else if (power == 1)
        {
            if (new_curse & TRC_SPECIAL_MASK) continue;
        }
        else if (power == 0)
        {
            if (new_curse & TRC_HEAVY_MASK) continue;
        }
        if (new_curse == OFC_LOW_MELEE && !object_is_weapon(o_ptr)) continue;
        if (new_curse == OFC_LOW_AC && !object_is_armour(o_ptr)) continue;
        break;
    }
    return new_curse;
}

static bool _object_is_any(object_type *o_ptr) { return TRUE; }
void curse_equipment(int chance, int heavy_chance)
{
    int slot = equip_random_slot(_object_is_any);
    if (slot)
    {
        bool         changed = FALSE;
        int          curse_power = 0;
        u32b         new_curse;
        u32b         oflgs[OF_ARRAY_SIZE];
        object_type *o_ptr = equip_obj(slot);
        char         o_name[MAX_NLEN];

        if (!o_ptr) return;
        if (randint1(100) > chance) return;

        obj_flags(o_ptr, oflgs);
        object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

        if (have_flag(oflgs, OF_BLESSED) && (randint1(888) > chance))
        {
            msg_format("Your %s resists cursing!", o_name);
            return;
        }

        if ((randint1(100) <= heavy_chance) &&
            (object_is_artifact(o_ptr) || object_is_ego(o_ptr)))
        {
            if (!(o_ptr->curse_flags & OFC_HEAVY_CURSE))
                changed = TRUE;
            o_ptr->curse_flags |= OFC_HEAVY_CURSE;
            o_ptr->curse_flags |= OFC_CURSED;
            curse_power++;
        }
        else
        {
            if (!object_is_cursed(o_ptr))
                changed = TRUE;
            o_ptr->curse_flags |= OFC_CURSED;
        }
        if (heavy_chance >= 50) curse_power++;

        new_curse = get_curse(curse_power, o_ptr);
        if (!(o_ptr->curse_flags & new_curse))
        {
            changed = TRUE;
            o_ptr->curse_flags |= new_curse;
        }

        if (changed)
        {
            msg_format("There is a malignant black aura surrounding %s...", o_name);
            o_ptr->feeling = FEEL_NONE;
        }
        p_ptr->update |= PU_BONUS;
    }
}


/*
 * Return TRUE if a spell is good for hurting the player (directly).
 */
static bool spell_attack(byte spell)
{
    /* All RF4 spells hurt (except for shriek and dispel and anti-magic) */
    if (spell < 128 && spell > 98 && spell != (96 + 5)) return (TRUE);

    /* BA_DISI */
    if (spell == 96 + 1) return (TRUE);

    /* Various "ball" spells */
    if (spell >= 128 && spell <= 128 + 8) return (TRUE);

    /* "Cause wounds" and "bolt" spells */
    if (spell >= 128 + 12 && spell < 128 + 27) return (TRUE);

    /* Hand of Doom */
    if (spell == 160 + 1) return (TRUE);

    /* Psycho-Spear */
    if (spell == 160 + 11) return (TRUE);

    /* Doesn't hurt */
    return (FALSE);
}


/*
 * Return TRUE if a spell is good for escaping.
 */
static bool spell_escape(byte spell)
{
    /* Blink or Teleport */
    if (spell == 160 + 4 || spell == 160 + 5) return (TRUE);

    /* Teleport the player away */
    if (spell == 160 + 9 || spell == 160 + 10) return (TRUE);

    /* Isn't good for escaping */
    return (FALSE);
}

/*
 * Return TRUE if a spell is good for annoying the player.
 */
static bool spell_annoy(byte spell)
{
    /* Shriek */
    if (spell == 96 + 0) return (TRUE);

    /* Brain smash, et al (added curses) */
    if (spell >= 128 + 9 && spell <= 128 + 14) return (TRUE);

    /* Scare, confuse, blind, slow, paralyze */
    if (spell >= 128 + 27 && spell <= 128 + 31) return (TRUE);

    /* Teleport to */
    if (spell == 160 + 8) return (TRUE);

    /* Teleport level */
    if (spell == 160 + 10) return (TRUE);

    /* Darkness, make traps, cause amnesia */
    if (spell >= 160 + 12 && spell <= 160 + 14) return (TRUE);

    /* Doesn't annoy */
    return (FALSE);
}

/*
 * Return TRUE if a spell summons help.
 */
static bool spell_summon(byte spell)
{
    /* All summon spells */
    if (spell >= 160 + 16) return (TRUE);

    /* Doesn't summon */
    return (FALSE);
}


/*
 * Return TRUE if a spell raise-dead.
 */
static bool spell_raise(byte spell)
{
    /* All raise-dead spells */
    if (spell == 160 + 15) return (TRUE);

    /* Doesn't summon */
    return (FALSE);
}


/*
 * Return TRUE if a spell is good in a tactical situation.
 */
static bool spell_tactic(byte spell)
{
    /* Blink */
    if (spell == 160 + 4) return (TRUE);

    /* Not good */
    return (FALSE);
}

/*
 * Return TRUE if a spell makes invulnerable.
 */
static bool spell_invulner(byte spell)
{
    /* Invulnerability */
    if (spell == 160 + 3) return (TRUE);

    /* No invulnerability */
    return (FALSE);
}

/*
 * Return TRUE if a spell hastes.
 */
static bool spell_haste(byte spell)
{
    /* Haste self */
    if (spell == 160 + 0) return (TRUE);

    /* Not a haste spell */
    return (FALSE);
}


/*
 * Return TRUE if a spell world.
 */
static bool spell_world(byte spell)
{
    /* world */
    if (spell == 160 + 6) return (TRUE);

    /* Not a haste spell */
    return (FALSE);
}


/*
 * Return TRUE if a spell special.
 */
static bool spell_special(byte spell)
{
    if (p_ptr->inside_battle) return FALSE;

    /* world */
    if (spell == 160 + 7) return (TRUE);

    /* Not a haste spell */
    return (FALSE);
}


/*
 * Return TRUE if a spell psycho-spear.
 */
static bool spell_psy_spe(byte spell)
{
    /* world */
    if (spell == 160 + 11) return (TRUE);

    /* Not a haste spell */
    return (FALSE);
}


/*
 * Return TRUE if a spell is good for healing.
 */
static bool spell_heal(byte spell)
{
    /* Heal */
    if (spell == 160 + 2) return (TRUE);

    /* No healing */
    return (FALSE);
}


/*
 * Return TRUE if a spell is good for dispel.
 */
static bool spell_dispel(byte spell)
{
    /* Dispel */
    if (spell == 96 + 2) return (TRUE);

    /* No dispel */
    return (FALSE);
}

/*
 * Return TRUE if a spell is good for anti-magic.
 */
static bool spell_anti_magic(byte spell)
{
    if (spell == 96 + 5) return (TRUE);

    /* No dispel */
    return (FALSE);
}

static bool anti_magic_check(void)
{
    if (p_ptr->anti_magic)
        return FALSE;

    if (p_ptr->tim_no_spells)
        return FALSE;

    switch (p_ptr->pclass)
    {
    case CLASS_WARRIOR:
    case CLASS_BERSERKER:
    case CLASS_WEAPONSMITH:
    case CLASS_ARCHER:
    case CLASS_CAVALRY:
        return FALSE;

    case CLASS_TOURIST:
        return one_in_(20);

    case CLASS_DUELIST:
        return one_in_(10);

    case CLASS_ROGUE:
    case CLASS_SCOUT:
    case CLASS_RANGER:
    case CLASS_PALADIN:
    case CLASS_WARRIOR_MAGE:
    case CLASS_CHAOS_WARRIOR:
    case CLASS_MONK:
    case CLASS_MYSTIC:
    case CLASS_BEASTMASTER:
    case CLASS_BLOOD_KNIGHT:
    case CLASS_MAULER:
	case CLASS_ALCHEMIST:
        return one_in_(5);

    case CLASS_MINDCRAFTER:
    case CLASS_IMITATOR:
    case CLASS_FORCETRAINER:
    case CLASS_PSION:
        return one_in_(3);

    case CLASS_MAGIC_EATER:
    case CLASS_RED_MAGE:
    case CLASS_DEVICEMASTER:
        return one_in_(2);
    }

    /* Everybody else always gets it! */
    return TRUE;
}

/*
 * Check should monster cast dispel spell.
 */
bool dispel_check(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* TODO: Monsters should have to learn this! */
    if (psion_mental_fortress() && !one_in_(12)) return FALSE;

    if (p_ptr->tim_slay_sentient) return TRUE;

    /* Invulnabilty (including the song) */
    if (IS_INVULN()) return (TRUE);

    /* Wraith form */
    if (IS_WRAITH()) return (TRUE);

    /* Shield */
    if (p_ptr->shield) return (TRUE);

    /* Magic defence */
    if (p_ptr->magicdef) return (TRUE);

    /* Multi Shadow */
    if (p_ptr->multishadow) return (TRUE);

    /* Robe of dust */
    if (p_ptr->dustrobe) return (TRUE);

    /* Berserk Strength */
    if (IS_SHERO() && (p_ptr->pclass != CLASS_BERSERKER)) return (TRUE);

    /* Powerful Mimickry: Note Colossus and Demon-Lord have insane XP requirements,
       so will always trigger a dispel. */
    if (p_ptr->mimic_form != MIMIC_NONE)
    {
        if (randint1(500) < get_race()->exp) return TRUE;
    }

    if (p_ptr->tim_shrike) return TRUE;
    if (p_ptr->tim_speed_essentia) return TRUE;

    /* Craft Munckin Checks :) */
    if (p_ptr->tim_genji) return TRUE;
    if (p_ptr->tim_force) return TRUE;
    if (p_ptr->tim_enlarge_weapon) return TRUE;
    if (p_ptr->kabenuke) return TRUE;

    /* Elemental resistances */
    if (r_ptr->flags4 & RF4_BR_ACID)
    {
        if (res_pct(RES_ACID) <= 75 && (p_ptr->oppose_acid || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_ACID) return (TRUE);
    }

    if (r_ptr->flags4 & RF4_BR_FIRE)
    {
        if (res_pct(RES_FIRE) <= 75 && (p_ptr->oppose_fire || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_FIRE) return (TRUE);
    }

    if (r_ptr->flags4 & RF4_BR_ELEC)
    {
        if (res_pct(RES_ELEC) <= 75 && (p_ptr->oppose_elec || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_ELEC) return (TRUE);
    }

    if (r_ptr->flags4 & RF4_BR_COLD)
    {
        if (res_pct(RES_COLD) <= 75 && (p_ptr->oppose_cold || music_singing(MUSIC_RESIST))) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_COLD) return (TRUE);
    }

    if (r_ptr->flags4 & (RF4_BR_POIS | RF4_BR_NUKE))
    {
        if (p_ptr->oppose_pois || music_singing(MUSIC_RESIST)) return (TRUE);
        if (p_ptr->special_defense & DEFENSE_POIS) return (TRUE);
    }

    /* Ultimate resistance */
    if (p_ptr->ult_res) return (TRUE);

    /* Potion of Neo Tsuyosi special */
    if (p_ptr->tsuyoshi) return (TRUE);

    /* Elemental Brands */
    if ((p_ptr->special_attack & ATTACK_ACID) && !(r_ptr->flagsr & RFR_EFF_IM_ACID_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_FIRE) && !(r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_ELEC) && !(r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_COLD) && !(r_ptr->flagsr & RFR_EFF_IM_COLD_MASK)) return (TRUE);
    if ((p_ptr->special_attack & ATTACK_POIS) && !(r_ptr->flagsr & RFR_EFF_IM_POIS_MASK)) return (TRUE);

    /* Speed */
    if (p_ptr->pspeed < 145)
    {
        if (IS_FAST()) return (TRUE);
    }

    /* Light speed */
    if (IS_LIGHT_SPEED() && (m_ptr->mspeed < 136)) return (TRUE);

    if (p_ptr->riding && (m_list[p_ptr->riding].mspeed < 135))
    {
        if (MON_FAST(&m_list[p_ptr->riding])) return (TRUE);
    }

    if (psion_check_dispel()) return TRUE;

    if ( p_ptr->prace == RACE_MON_MIMIC
      && p_ptr->current_r_idx != MON_MIMIC )
    {
        int lvl = r_info[p_ptr->current_r_idx].level;
        if (lvl >= 50 && randint1(100) < lvl)
            return TRUE;
    }

    /* No need to cast dispel spell */
    return (FALSE);
}


/*
 * Have a monster choose a spell from a list of "useful" spells.
 *
 * Note that this list does NOT include spells that will just hit
 * other monsters, and the list is restricted when the monster is
 * "desperate". Should that be the job of this function instead?
 *
 * Stupid monsters will just pick a spell randomly. Smart monsters
 * will choose more "intelligently".
 *
 * Use the helper functions above to put spells into categories.
 *
 * This function may well be an efficiency bottleneck.
 */
static int choose_attack_spell(int m_idx, byte spells[], byte num, bool ticked_off)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    byte escape[96], escape_num = 0;
    byte attack[96], attack_num = 0;
    byte summon[96], summon_num = 0;
    byte tactic[96], tactic_num = 0;
    byte annoy[96], annoy_num = 0;
    byte invul[96], invul_num = 0;
    byte haste[96], haste_num = 0;
    byte world[96], world_num = 0;
    byte special[96], special_num = 0;
    byte psy_spe[96], psy_spe_num = 0;
    byte raise[96], raise_num = 0;
    byte heal[96], heal_num = 0;
    byte dispel[96], dispel_num = 0;
    byte anti_magic[96], anti_magic_num = 0;

    int i;

    /* Stupid monsters choose randomly */
    if (r_ptr->flags2 & (RF2_STUPID))
    {
        /* Pick at random */
        return (spells[randint0(num)]);
    }

    /* Categorize spells */
    for (i = 0; i < num; i++)
    {
        /* Escape spell? */
        if (spell_escape(spells[i])) escape[escape_num++] = spells[i];

        /* Attack spell? */
        if (spell_attack(spells[i])) attack[attack_num++] = spells[i];

        /* Summon spell? */
        if (spell_summon(spells[i])) summon[summon_num++] = spells[i];

        /* Tactical spell? */
        if (spell_tactic(spells[i])) tactic[tactic_num++] = spells[i];

        /* Annoyance spell? */
        if (spell_annoy(spells[i])) annoy[annoy_num++] = spells[i];

        /* Invulnerability spell? */
        if (spell_invulner(spells[i])) invul[invul_num++] = spells[i];

        /* Haste spell? */
        if (spell_haste(spells[i])) haste[haste_num++] = spells[i];

        /* World spell? */
        if (spell_world(spells[i])) world[world_num++] = spells[i];

        /* Special spell? */
        if (spell_special(spells[i])) special[special_num++] = spells[i];

        /* Psycho-spear spell? */
        if (spell_psy_spe(spells[i])) psy_spe[psy_spe_num++] = spells[i];

        /* Raise-dead spell? */
        if (spell_raise(spells[i])) raise[raise_num++] = spells[i];

        /* Heal spell? */
        if (spell_heal(spells[i])) heal[heal_num++] = spells[i];

        /* Dispel spell? */
        if (spell_dispel(spells[i])) dispel[dispel_num++] = spells[i];

        if (spell_anti_magic(spells[i])) anti_magic[anti_magic_num++] = spells[i];
    }

    /*** Try to pick an appropriate spell type ***/

    /* world */
    if (world_num && (randint0(100) < 15) && !world_monster)
    {
        /* Choose haste spell */
        return (world[randint0(world_num)]);
    }

    /* special */
    if (special_num)
    {
        bool success = FALSE;
        switch(m_ptr->r_idx)
        {
            case MON_BANOR:
            case MON_LUPART:
                if ((m_ptr->hp < m_ptr->maxhp / 2) && r_info[MON_BANOR].max_num && r_info[MON_LUPART].max_num) success = TRUE;
                break;
            default: break;
        }
        if (success) return (special[randint0(special_num)]);
    }

    /* Still hurt badly, couldn't flee, attempt to heal */
    if (m_ptr->hp < m_ptr->maxhp / 3 && heal_num)
    {
        int odds = 2;
        if (ticked_off)
            odds = 5;

        if (one_in_(odds)) return (heal[randint0(heal_num)]);
    }

    /* Hurt badly or afraid, attempt to flee */
    if (((m_ptr->hp < m_ptr->maxhp / 3) || MON_MONFEAR(m_ptr)) && escape_num)
    {
        int odds = 2;
        if (ticked_off)
            odds = 5;

        if (one_in_(odds)) return (escape[randint0(escape_num)]);
    }

    /* special */
    if (special_num)
    {
        bool success = FALSE;
        switch (m_ptr->r_idx)
        {
            case MON_OHMU:
            case MON_BANOR:
            case MON_LUPART:
                break;
            case MON_BANORLUPART:
                if (randint0(100) < 70) success = TRUE;
                break;
            case MON_ROLENTO:
                if (randint0(100) < 40) success = TRUE;
                break;
            default:
                if (randint0(100) < 50) success = TRUE;
                break;
        }
        if (success) return (special[randint0(special_num)]);
    }

    /* Player is close and we have attack spells, blink away */
    if ((distance(py, px, m_ptr->fy, m_ptr->fx) < 4) && (attack_num || (r_ptr->flags6 & RF6_TRAPS)) && (randint0(100) < 75) && !world_monster)
    {
        /* Choose tactical spell */
        if (tactic_num) return (tactic[randint0(tactic_num)]);
    }

    /* Summon if possible (sometimes) */
    if (summon_num)
    {
        int odds = 20;

        if (ticked_off && attack_num)
            odds = 10;

        if (randint0(100) < odds) return (summon[randint0(summon_num)]);
    }

    /* dispel or anti-magic ... these abilities are evil, so
       only roll the 1 in 2 odds once */
    if ((dispel_num || anti_magic_num) && one_in_(2))
    {
        int n = randint1(10);
        if (n <= 7)
        {
            if (dispel_num && dispel_check(m_idx))
                return (dispel[randint0(dispel_num)]);
        }
        else
        {
            if (dispel_num && anti_magic_check())
                return (anti_magic[randint0(anti_magic_num)]);
        }
    }

    /* Raise-dead if possible (sometimes) */
    if (raise_num && (randint0(100) < 40))
    {
        /* Choose raise-dead spell */
        return (raise[randint0(raise_num)]);
    }

    /* Attack spell (most of the time) */
    if (IS_INVULN())
    {
        if (psy_spe_num && (randint0(100) < 50))
        {
            /* Choose attack spell */
            return (psy_spe[randint0(psy_spe_num)]);
        }
        else if (attack_num && (randint0(100) < 40))
        {
            /* Choose attack spell */
            return (attack[randint0(attack_num)]);
        }
    }
    else if (attack_num && (ticked_off || (randint0(100) < 85)))
    {
        /* Choose attack spell */
        return (attack[randint0(attack_num)]);
    }

    /* Try another tactical spell (sometimes) */
    if (tactic_num && (randint0(100) < 50) && !world_monster)
    {
        /* Choose tactic spell */
        return (tactic[randint0(tactic_num)]);
    }

    /* Cast globe of invulnerability if not already in effect */
    if (invul_num && !m_ptr->mtimed[MTIMED_INVULNER] && (randint0(100) < 50))
    {
        /* Choose Globe of Invulnerability */
        return (invul[randint0(invul_num)]);
    }

    /* We're hurt (not badly), try to heal */
    if ((m_ptr->hp < m_ptr->maxhp * 3 / 4) && (randint0(100) < 25))
    {
        /* Choose heal spell if possible */
        if (heal_num) return (heal[randint0(heal_num)]);
    }

    /* Haste self if we aren't already somewhat hasted (rarely) */
    if (haste_num && (randint0(100) < 20) && !MON_FAST(m_ptr))
    {
        /* Choose haste spell */
        return (haste[randint0(haste_num)]);
    }

    /* Annoy player (most of the time) */
    if (annoy_num && (randint0(100) < 80))
    {
        /* Choose annoyance spell */
        return (annoy[randint0(annoy_num)]);
    }

    /* Choose no spell */
    return (0);
}


/*
 * Return TRUE if a spell is inate spell.
 */
bool spell_is_inate(u16b spell)
{
    if (spell < 32 * 4) /* Set RF4 */
    {
        if ((1L << (spell - 32 * 3)) & RF4_NOMAGIC_MASK) return TRUE;
    }
    else if (spell < 32 * 5) /* Set RF5 */
    {
        if ((1L << (spell - 32 * 4)) & RF5_NOMAGIC_MASK) return TRUE;
    }
    else if (spell < 32 * 6) /* Set RF6 */
    {
        if ((1L << (spell - 32 * 5)) & RF6_NOMAGIC_MASK) return TRUE;
    }

    /* This spell is not "inate" */
    return FALSE;
}


static bool adjacent_grid_check(monster_type *m_ptr, int *yp, int *xp,
    int f_flag, bool (*path_check)(int, int, int, int))
{
    int i;
    int tonari;
    static int tonari_y[4][8] = {{-1, -1, -1,  0,  0,  1,  1,  1},
                                 {-1, -1, -1,  0,  0,  1,  1,  1},
                                 { 1,  1,  1,  0,  0, -1, -1, -1},
                                 { 1,  1,  1,  0,  0, -1, -1, -1}};
    static int tonari_x[4][8] = {{-1,  0,  1, -1,  1, -1,  0,  1},
                                 { 1,  0, -1,  1, -1,  1,  0, -1},
                                 {-1,  0,  1, -1,  1, -1,  0,  1},
                                 { 1,  0, -1,  1, -1,  1,  0, -1}};

    if (m_ptr->fy < py && m_ptr->fx < px) tonari = 0;
    else if (m_ptr->fy < py) tonari = 1;
    else if (m_ptr->fx < px) tonari = 2;
    else tonari = 3;

    for (i = 0; i < 8; i++)
    {
        int next_x = *xp + tonari_x[tonari][i];
        int next_y = *yp + tonari_y[tonari][i];
        cave_type *c_ptr;

        /* Access the next grid */
        c_ptr = &cave[next_y][next_x];

        /* Skip this feature */
        if (!cave_have_flag_grid(c_ptr, f_flag)) continue;

        if (path_check(m_ptr->fy, m_ptr->fx, next_y, next_x))
        {
            *yp = next_y;
            *xp = next_x;
            return TRUE;
        }
    }

    return FALSE;
}

#define DO_SPELL_NONE    0
#define DO_SPELL_BR_LITE 1
#define DO_SPELL_BR_DISI 2
#define DO_SPELL_BA_LITE 3
#define DO_SPELL_TELE_TO 4

/*
 * Creatures can cast spells, shoot missiles, and breathe.
 *
 * Returns "TRUE" if a spell (or whatever) was (successfully) cast.
 *
 * XXX XXX XXX This function could use some work, but remember to
 * keep it as optimized as possible, while retaining generic code.
 *
 * Verify the various "blind-ness" checks in the code.
 *
 * XXX XXX XXX Note that several effects should really not be "seen"
 * if the player is blind. See also "effects.c" for other "mistakes".
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
 *
 * Perhaps smart monsters should decline to use "bolt" spells if
 * there is a monster in the way, unless they wish to kill it.
 *
 * Note that, to allow the use of the "track_target" option at some
 * later time, certain non-optimal things are done in the code below,
 * including explicit checks against the "direct" variable, which is
 * currently always true by the time it is checked, but which should
 * really be set according to an explicit "projectable()" test, and
 * the use of generic "x,y" locations instead of the player location,
 * with those values being initialized with the player location.
 *
 * It will not be possible to "correctly" handle the case in which a
 * monster attempts to attack a location which is thought to contain
 * the player, but which in fact is nowhere near the player, since this
 * might induce all sorts of messages about the attack itself, and about
 * the effects of the attack, which the player might or might not be in
 * a position to observe. Thus, for simplicity, it is probably best to
 * only allow "faulty" attacks by a monster if one of the important grids
 * (probably the initial or final grid) is in fact in view of the player.
 * It may be necessary to actually prevent spell attacks except when the
 * monster actually has line of sight to the player. Note that a monster
 * could be left in a bizarre situation after the player ducked behind a
 * pillar and then teleported away, for example.
 *
 * Note that certain spell attacks do not use the "project()" function
 * but "simulate" it via the "direct" variable, which is always at least
 * as restrictive as the "project()" function. This is necessary to
 * prevent "blindness" attacks and such from bending around walls, etc,
 * and to allow the use of the "track_target" option in the future.
 *
 * Note that this function attempts to optimize the use of spells for the
 * cases in which the monster has no spells, or has spells but cannot use
 * them, or has spells but they will have no "useful" effect. Note that
 * this function has been an efficiency bottleneck in the past.
 *
 * Note the special "MFLAG_NICE" flag, which prevents a monster from using
 * any spell attacks until the player has had a single chance to move.
 */
bool make_attack_spell(int m_idx, bool ticked_off)
{
    int             k, thrown_spell = 0, rlev, failrate;
    byte            spell[96], num = 0;
    u32b            f4, f5, f6;
    monster_type    *m_ptr = &m_list[m_idx];
    monster_race    *r_ptr = &r_info[m_ptr->r_idx];
    char            tmp[MAX_NLEN];
    char            m_name[MAX_NLEN];
    char            m_poss[80];
    bool            no_inate = FALSE;
    bool            do_spell = DO_SPELL_NONE;
    int             dam = 0;
    u32b mode = 0L;
    int s_num_6 = 6;
    int s_num_4 = 3; /* ?! */

    /* Target location */
    int x = px;
    int y = py;

    /* Target location for lite breath */
    int x_br_lite = 0;
    int y_br_lite = 0;

    /* Summon count */
    int count = 0;

    /* Extract the blind-ness */
    bool blind = (p_ptr->blind ? TRUE : FALSE);

    /* Extract the "see-able-ness" */
    bool seen = (!blind && m_ptr->ml);

    bool maneable = player_has_los_bold(m_ptr->fy, m_ptr->fx);
    bool learnable = (seen && maneable && !world_monster);

    /* Check "projectable" */
    bool direct;
    bool wall_scummer = FALSE;

    bool in_no_magic_dungeon = (d_info[dungeon_type].flags1 & DF1_NO_MAGIC) && dun_level
        && (!p_ptr->inside_quest || is_fixed_quest_idx(p_ptr->inside_quest));

    bool can_use_lite_area = FALSE;

    bool can_remember;

    /* Cannot cast spells when confused */
    if (MON_CONFUSED(m_ptr))
    {
        reset_target(m_ptr);
        return FALSE;
    }

    /* Cannot cast spells when nice */
    if (m_ptr->mflag & MFLAG_NICE) return FALSE;
    if (!is_hostile(m_ptr)) return FALSE;
    if (!is_aware(m_ptr)) return FALSE;


    /* Sometimes forbid inate attacks (breaths) */
    if (randint0(100) >= (r_ptr->freq_spell * 2)) no_inate = TRUE;

    /* XXX XXX XXX Handle "track_target" option (?) */


    /* Extract the racial spell flags */
    f4 = r_ptr->flags4;
    f5 = r_ptr->flags5;
    f6 = r_ptr->flags6;

    /* Ticked off monsters favor powerful offense */
    if (ticked_off)
    {
        f4 &= RF4_ATTACK_MASK;
        f5 &= RF5_WORTHY_ATTACK_MASK;

        if (m_ptr->hp < m_ptr->maxhp / 3)
            f6 &= (RF6_WORTHY_ATTACK_MASK | RF6_WORTHY_SUMMON_MASK | RF6_PANIC_MASK);
        else
            f6 &= (RF6_WORTHY_ATTACK_MASK | RF6_WORTHY_SUMMON_MASK);

        /* Restore original spell list if masking removes *all* spells */
        if (!f4 && !f5 && !f6)
        {
            f4 = r_ptr->flags4;
            f5 = r_ptr->flags5;
            f6 = r_ptr->flags6;
            ticked_off = FALSE;
        }
    }

    /*** require projectable player ***/

    /* Check range */
    if ((m_ptr->cdis > MAX_RANGE) && !m_ptr->target_y) return (FALSE);

    /* Check path for lite breath */
    if (f4 & RF4_BR_LITE)
    {
        y_br_lite = y;
        x_br_lite = x;

        if (los(m_ptr->fy, m_ptr->fx, y_br_lite, x_br_lite))
        {
            feature_type *f_ptr = &f_info[cave[y_br_lite][x_br_lite].feat];

            if (!have_flag(f_ptr->flags, FF_LOS))
            {
                if (have_flag(f_ptr->flags, FF_PROJECT) && one_in_(2)) f4 &= ~(RF4_BR_LITE);
            }
        }

        /* Check path to next grid */
        else if (!adjacent_grid_check(m_ptr, &y_br_lite, &x_br_lite, FF_LOS, los)) f4 &= ~(RF4_BR_LITE);

        /* Don't breath lite to the wall if impossible */
        if (!(f4 & RF4_BR_LITE))
        {
            y_br_lite = 0;
            x_br_lite = 0;
        }
    }

    /* Check path */
    if (projectable(m_ptr->fy, m_ptr->fx, y, x))
    {
        feature_type *f_ptr = &f_info[cave[y][x].feat];

        if (!have_flag(f_ptr->flags, FF_PROJECT))
        {
            /* Breath disintegration to the wall if possible */
            if ((f4 & RF4_BR_DISI) && have_flag(f_ptr->flags, FF_HURT_DISI) && one_in_(2)) do_spell = DO_SPELL_BR_DISI;

            /* Breath lite to the transparent wall if possible */
            else if ((f4 & RF4_BR_LITE) && have_flag(f_ptr->flags, FF_LOS) && one_in_(2)) do_spell = DO_SPELL_BR_LITE;
        }
    }

    /* Check path to next grid */
    else
    {
        bool success = FALSE;

        if ((f4 & RF4_BR_DISI) && (m_ptr->cdis < MAX_RANGE/2) &&
            in_disintegration_range(m_ptr->fy, m_ptr->fx, y, x) &&
            (one_in_(10) || (projectable(y, x, m_ptr->fy, m_ptr->fx) && one_in_(2))))
        {
            do_spell = DO_SPELL_BR_DISI;
            success = TRUE;
        }
        else if ((f4 & RF4_BR_LITE) && (m_ptr->cdis < MAX_RANGE/2) &&
            los(m_ptr->fy, m_ptr->fx, y, x) && one_in_(5))
        {
            do_spell = DO_SPELL_BR_LITE;
            success = TRUE;
        }
        else if ((f5 & RF5_BA_LITE) && (m_ptr->cdis <= MAX_RANGE))
        {
            int by = y, bx = x;
            get_project_point(m_ptr->fy, m_ptr->fx, &by, &bx, 0L);
            if ((distance(by, bx, y, x) <= 3) && los(by, bx, y, x) && one_in_(5))
            {
                do_spell = DO_SPELL_BA_LITE;
                success = TRUE;
            }
        }

        if ( !success                      /* <=== Raphael can Breathe Light *and* Teleport To */
          && (f6 & RF6_TELE_TO)
          && m_ptr->cdis <= (MAX_RANGE * 2/3)
          && r_ptr->level >= 40
          && (r_ptr->flags1 & RF1_UNIQUE)
          && !(cave[m_ptr->fy][m_ptr->fx].info & CAVE_ICKY) )
        {
            if (one_in_(15))
            {
                do_spell = DO_SPELL_TELE_TO;
                success = TRUE;
            }
        }

        if (!success) success = adjacent_grid_check(m_ptr, &y, &x, FF_PROJECT, projectable);

        if (!success)
        {
            if (m_ptr->target_y && m_ptr->target_x)
            {
                y = m_ptr->target_y;
                x = m_ptr->target_x;
                f4 &= (RF4_INDIRECT_MASK);
                f5 &= (RF5_INDIRECT_MASK);
                f6 &= (RF6_INDIRECT_MASK);
                success = TRUE;
            }

            if (y_br_lite && x_br_lite && (m_ptr->cdis < MAX_RANGE/2) && one_in_(5))
            {
                if (!success)
                {
                    y = y_br_lite;
                    x = x_br_lite;
                    do_spell = DO_SPELL_BR_LITE;
                    success = TRUE;
                }
                else f4 |= (RF4_BR_LITE);
            }

            /* Hack: Is player hiding in walls? Note MONSTER_FLOW_DEPTH is cranked up
               to 100 but is still might be possible that there exists a viable path
               to the player that is longer. */
            if ( current_flow_depth < 30 /*MONSTER_FLOW_DEPTH*/
              && !(r_ptr->flags2 & RF2_PASS_WALL)
              && !(r_ptr->flags2 & RF2_KILL_WALL)
              && !(r_ptr->flags1 & RF1_NEVER_MOVE)
              && !cave[m_ptr->fy][m_ptr->fx].dist
              && !(cave[m_ptr->fy][m_ptr->fx].info & CAVE_ICKY)
              && !(cave[py][px].info & CAVE_ICKY)
              && !p_ptr->inside_quest
              && dun_level
              && (r_ptr->flags1 & RF1_UNIQUE) )
            {
                y = m_ptr->fy;
                x = m_ptr->fx;

                if (one_in_(20))
                {
                    f4 &= RF4_NO_FLOW_MASK_HARD;
                    f5 &= RF5_NO_FLOW_MASK_HARD;
                    f6 &= RF6_NO_FLOW_MASK_HARD;
                    mode |= PM_WALL_SCUMMER;
                }
                else
                {
                    f4 &= RF4_NO_FLOW_MASK_EASY;
                    f5 &= RF5_NO_FLOW_MASK_EASY;
                    f6 &= RF6_NO_FLOW_MASK_EASY;
                }
                success = TRUE;
                wall_scummer = TRUE;
            }
        }

        /* No spells */
        if (!success) return FALSE;
    }

    reset_target(m_ptr);

    /* Extract the monster level */
    rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

    /* Forbid inate attacks sometimes */
    if (no_inate)
    {
        f4 &= ~(RF4_NOMAGIC_MASK);
        f5 &= ~(RF5_NOMAGIC_MASK);
        f6 &= ~(RF6_NOMAGIC_MASK);
    }

    if (f6 & RF6_DARKNESS)
    {
        if ((p_ptr->pclass == CLASS_NINJA) &&
            !(r_ptr->flags3 & (RF3_UNDEAD | RF3_HURT_LITE)) &&
            !(r_ptr->flags7 & RF7_DARK_MASK))
            can_use_lite_area = TRUE;

        if (!(r_ptr->flags2 & RF2_STUPID))
        {
            if (d_info[dungeon_type].flags1 & DF1_DARKNESS) f6 &= ~(RF6_DARKNESS);
            else if ((p_ptr->pclass == CLASS_NINJA) && !can_use_lite_area) f6 &= ~(RF6_DARKNESS);
        }
    }

    if (in_no_magic_dungeon && !(r_ptr->flags2 & RF2_STUPID))
    {
        f4 &= (RF4_NOMAGIC_MASK);
        f5 &= (RF5_NOMAGIC_MASK);
        f6 &= (RF6_NOMAGIC_MASK);
    }

    if (r_ptr->flags2 & RF2_SMART)
    {
        /* Hack -- allow "desperate" spells */
        if ((m_ptr->hp < m_ptr->maxhp / 10) &&
            (randint0(100) < 50))
        {
            /* Require intelligent spells */
            f4 &= (RF4_INT_MASK);
            f5 &= (RF5_INT_MASK);
            f6 &= (RF6_INT_MASK);
        }

        /* Hack -- decline "teleport level" in some case */
        if ((f6 & RF6_TELE_LEVEL) && TELE_LEVEL_IS_INEFF(0))
        {
            f6 &= ~(RF6_TELE_LEVEL);
        }
    }

    /* No spells left */
    if (!f4 && !f5 && !f6) return (FALSE);

    /* Remove the "ineffective" spells */
    remove_bad_spells(m_idx, &f4, &f5, &f6);

    /* Remove Forgotten Spells */
    f4 &= ~m_ptr->forgot4;
    f5 &= ~m_ptr->forgot5;
    f6 &= ~m_ptr->forgot6;

    if (p_ptr->inside_arena || p_ptr->inside_battle)
    {
        f4 &= ~(RF4_SUMMON_MASK);
        f5 &= ~(RF5_SUMMON_MASK);
        f6 &= ~(RF6_SUMMON_MASK | RF6_TELE_LEVEL);

        if (m_ptr->r_idx == MON_ROLENTO) f6 &= ~(RF6_SPECIAL);
    }

    /* No spells left */
    if (!f4 && !f5 && !f6) return (FALSE);

    if (!(r_ptr->flags2 & RF2_STUPID))
    {
        if (!p_ptr->csp) f5 &= ~(RF5_DRAIN_MANA);

        /* Check for a clean bolt shot */
        if (((f4 & RF4_BOLT_MASK) ||
             (f5 & RF5_BOLT_MASK) ||
             (f6 & RF6_BOLT_MASK)) &&
            !clean_shot(m_ptr->fy, m_ptr->fx, py, px, FALSE))
        {
            /* Remove spells that will only hurt friends */
            f4 &= ~(RF4_BOLT_MASK);
            f5 &= ~(RF5_BOLT_MASK);
            f6 &= ~(RF6_BOLT_MASK);
        }

        /* Check for a possible summon */
        if (((f4 & RF4_SUMMON_MASK) ||
             (f5 & RF5_SUMMON_MASK) ||
             (f6 & RF6_SUMMON_MASK)) &&
            !(summon_possible(y, x)))
        {
            /* Remove summoning spells */
            f4 &= ~(RF4_SUMMON_MASK);
            f5 &= ~(RF5_SUMMON_MASK);
            f6 &= ~(RF6_SUMMON_MASK);
        }

        /* Check for a possible raise dead */
        if ((f6 & RF6_RAISE_DEAD) && !raise_possible(m_ptr))
        {
            /* Remove raise dead spell */
            f6 &= ~(RF6_RAISE_DEAD);
        }

        /* Special moves restriction */
        if (f6 & RF6_SPECIAL)
        {
            if ((m_ptr->r_idx == MON_ROLENTO) && !summon_possible(y, x))
            {
                f6 &= ~(RF6_SPECIAL);
            }
            if (m_ptr->r_idx == MON_ARTEMIS)
            {
                if(!one_in_(7))
                    f6 &= ~(RF6_SPECIAL);
                if (m_ptr->cdis >= 3)
                    f6 &= ~(RF6_BLINK);
            }
            if ((r_ptr->flags3 & RF3_OLYMPIAN) && !summon_possible(y, x))
            {
                f6 &= ~(RF6_SPECIAL);
            }
        }

        /* No spells left */
        if (!f4 && !f5 && !f6) return (FALSE);
    }

    /* Extract the "inate" spells */
    for (k = 0; k < 32; k++)
    {
        if (f4 & (1L << k)) spell[num++] = k + 32 * 3;
    }

    /* Extract the "normal" spells */
    for (k = 0; k < 32; k++)
    {
        if (f5 & (1L << k)) spell[num++] = k + 32 * 4;
    }

    /* Extract the "bizarre" spells */
    for (k = 0; k < 32; k++)
    {
        if (f6 & (1L << k)) spell[num++] = k + 32 * 5;
    }

    /* No spells left */
    if (!num) return (FALSE);

    /* Stop if player is dead or gone */
    if (!p_ptr->playing || p_ptr->is_dead) return (FALSE);

    /* Stop if player is leaving */
    if (p_ptr->leaving) return (FALSE);

    /* Get the monster name (or "it") */
    monster_desc(tmp, m_ptr, 0x00);
    tmp[0] = toupper(tmp[0]);
    sprintf(m_name, "<color:g>%s</color>", tmp);

    /* Get the monster possessive ("his"/"her"/"its") */
    monster_desc(m_poss, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);

    switch (do_spell)
    {
    case DO_SPELL_NONE:
        {
            int attempt = 10;
            while (attempt--)
            {
                thrown_spell = choose_attack_spell(m_idx, spell, num, ticked_off);
                if (thrown_spell) break;
            }
        }
        break;

    case DO_SPELL_BR_LITE:
        thrown_spell = 96+14; /* RF4_BR_LITE */
        break;

    case DO_SPELL_BR_DISI:
        thrown_spell = 96+31; /* RF4_BR_DISI */
        break;

    case DO_SPELL_BA_LITE:
        thrown_spell = 128+20; /* RF5_BA_LITE */
        break;

    case DO_SPELL_TELE_TO:
        thrown_spell = 160+8; /* RF6_TELE_TO */
        break;

    default:
        return FALSE; /* Paranoia */
    }

    /* Abort if no spell was chosen */
    if (!thrown_spell) return (FALSE);

    /* Calculate spell failure rate */
    failrate = 25 - (rlev + 3) / 4;

    /* Hack -- Stupid monsters will never fail (for jellies and such) */
    if (r_ptr->flags2 & RF2_STUPID) failrate = 0;

    /* Check for spell failure (inate attacks never fail) */
    if (!spell_is_inate(thrown_spell)
        && (in_no_magic_dungeon || (MON_STUNNED(m_ptr) && one_in_(2)) || (randint0(100) < failrate)))
    {
        disturb(1, 0);
        mon_lore_aux_spell(r_ptr);
        msg_format("%^s tries to cast a spell, but fails.", m_name);
        return (TRUE);
    }

    /* Hex: Anti Magic Barrier */
    if (!spell_is_inate(thrown_spell) && magic_barrier(m_idx))
    {
        msg_format("Your anti-magic barrier blocks the spell which %^s casts.", m_name);
        return (TRUE);
    }

    if (!spell_is_inate(thrown_spell) && psion_check_disruption(m_idx))
    {
        msg_format("Your psionic disruption blocks the spell which %^s casts.", m_name);
        return TRUE;
    }

    /* Projectable? */
    direct = player_bold(y, x);

    can_remember = is_original_ap_and_seen(m_ptr);

    /* Cast the spell. */
    hack_m_spell = thrown_spell;
    switch (thrown_spell)
    {
        /* RF4_SHRIEK */
        case 96+0:
        {
            if (!wall_scummer) disturb(1, 0);
            msg_format("%^s makes a high pitched shriek.", m_name);
            aggravate_monsters(m_idx);
            break;
        }
        /* RF4_THROW ... Shamelessly snagged from TinyAngband! Thanks guys :) */
        case 96+1:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s shouts, 'Haa!!'.", m_name);
            else msg_format("%^s throws a large rock.", m_name);
             sound(SOUND_MISS); /* (Sound substitute) Throwing a rock isn't a rocket sound anyway */
            dam = rlev * 3;
            breath(y, x, m_idx, GF_ROCK, dam, 1, FALSE, MS_THROW, learnable);
            update_smart_learn(m_idx, DRS_SHARD);
            break;
        }
        /* RF4_DISPEL */
        case 96+2:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles powerfully.", m_name);
            else msg_format("%^s invokes a dispel magic.", m_name);
            if (check_foresight())
            {
            }
            else if (mut_present(MUT_ONE_WITH_MAGIC) && one_in_(2))
                msg_print("You resist the effects!");
            else if (psion_mental_fortress())
                msg_print("Your mental fortress is impenetrable!");
            else
            {
                dispel_player();
                if (p_ptr->riding) dispel_monster_status(p_ptr->riding);
                learn_spell(MS_DISPEL);
            }
            break;
        }
        /* RF4_ROCKET */
        case 96+3:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s shoots something.", m_name);
            else msg_format("%^s fires a rocket.", m_name);
            dam = MIN(m_ptr->hp / 4, 600);
            breath(y, x, m_idx, GF_ROCKET,
                dam, 2, FALSE, MS_ROCKET, learnable);
            update_smart_learn(m_idx, DRS_SHARD);
            break;
        }
        /* RF4_SHOOT */
        case 96+4:
        {
            int ct = 1, i;
            if (!direct) return (FALSE);
            disturb(1, 0);

            if (m_ptr->r_idx == MON_ARTEMIS)
                ct = 4;

            for (i = 0; i < ct; i++)
            {
                if (blind) msg_format("%^s makes a strange noise.", m_name);
                else msg_format("%^s fires an arrow.", m_name);

                dam = damroll(r_ptr->blow[0].d_dice, r_ptr->blow[0].d_side);
                if (m_ptr->r_idx == MON_ARTEMIS)
                    artemis_bolt(m_idx, GF_ARROW, dam, MS_SHOOT, learnable);
                else
                    bolt(m_idx, GF_ARROW, dam, MS_SHOOT, learnable);
                update_smart_learn(m_idx, DRS_REFLECT);
            }
            break;
        }
        /* RF4_ANTI_MAGIC */
        case 96+5:
        {
            if (blind) msg_format("%^s mumbles powerfully.", m_name);
            else msg_format("%^s invokes anti-magic.", m_name);

            if (randint1(100) <= duelist_skill_sav(m_idx) - r_ptr->level/2)
                msg_print("You resist the effects!");
            else if (check_foresight())
            {
            }
            else if (mut_present(MUT_ONE_WITH_MAGIC) && one_in_(2))
                msg_print("You resist the effects!");
            else if (psion_mental_fortress())
                msg_print("Your mental fortress is impenetrable!");
            else
                set_tim_no_spells(p_ptr->tim_no_spells + 3 + randint1(3), FALSE);
            break;
        }
        /* RF4_POLY */
        case 96+6:
        {
            if (blind) msg_format("%^s mumbles powerfully.", m_name);
            else msg_format("%^s invokes polymorph other.", m_name);
            if (prace_is_(RACE_ANDROID) || p_ptr->pclass == CLASS_MONSTER || p_ptr->prace == RACE_DOPPELGANGER)
                msg_print("You are unaffected!");
            else if (mut_present(MUT_DRACONIAN_METAMORPHOSIS))
                msg_print("You are unaffected!");
            else if (randint1(100) <= duelist_skill_sav(m_idx) - r_ptr->level/2)
                msg_print("You resist the effects!");
            else if (check_foresight())
            {
            }
            else
            {
                int which;
                /* TODO: MIMIC_GIANT_TOAD ... */
                switch(randint1(5))
                {
                case 1:
                    if (p_ptr->prace != RACE_SNOTLING)
                    {
                        which = RACE_SNOTLING;
                        break;
                    }
                case 2:
                    if (p_ptr->prace != RACE_YEEK)
                    {
                        which = RACE_YEEK;
                        break;
                    }
                case 3:
                    which = MIMIC_SMALL_KOBOLD;
                    break;
                case 4:
                    which = MIMIC_MANGY_LEPER;
                    break;
                default:
                    for (;;)
                    {
                        which = randint0(MAX_RACES);
                        if ( which != RACE_HUMAN
                          && which != RACE_DEMIGOD
                          && which != RACE_DRACONIAN
                          && which != RACE_ANDROID
                          && p_ptr->prace != which
                          && !(get_race_aux(which, 0)->flags & RACE_IS_MONSTER) )
                        {
                            break;
                        }
                    }
                }
                set_mimic(50 + randint1(50), which, FALSE);
            }
            break;
        }
        /* RF4_BR_STORM */
        case 96+7:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes storm winds.", m_name);
            dam = MIN(m_ptr->hp / 5, 300);
            breath(y, x, m_idx, GF_STORM, dam, 0, TRUE, MS_BR_STORM, learnable);
            break;
        }
        /* RF4_BR_ACID */
        case 96+8:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes acid.", m_name);
            dam = MIN(m_ptr->hp / 4, 900);
            breath(y, x, m_idx, GF_ACID, dam, 0, TRUE, MS_BR_ACID, learnable);
            update_smart_learn(m_idx, DRS_ACID);
            break;
        }
        /* RF4_BR_ELEC */
        case 96+9:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes lightning.", m_name);
            dam = MIN(m_ptr->hp / 4, 900);
            breath(y, x, m_idx, GF_ELEC, dam,0, TRUE, MS_BR_ELEC, learnable);
            update_smart_learn(m_idx, DRS_ELEC);
            break;
        }
        /* RF4_BR_FIRE */
        case 96+10:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes fire.", m_name);
            dam = MIN(m_ptr->hp / 4, 900);
            breath(y, x, m_idx, GF_FIRE, dam,0, TRUE, MS_BR_FIRE, learnable);
            update_smart_learn(m_idx, DRS_FIRE);
            break;
        }
        /* RF4_BR_COLD */
        case 96+11:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes frost.", m_name);
            dam = MIN(m_ptr->hp / 4, 900);
            breath(y, x, m_idx, GF_COLD, dam,0, TRUE, MS_BR_COLD, learnable);
            update_smart_learn(m_idx, DRS_COLD);
            break;
        }
        /* RF4_BR_POIS */
        case 96+12:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes gas.", m_name);
            dam = MIN(m_ptr->hp / 5, 600);
            breath(y, x, m_idx, GF_POIS, dam, 0, TRUE, MS_BR_POIS, learnable);
            update_smart_learn(m_idx, DRS_POIS);
            break;
        }
        /* RF4_BR_NETH */
        case 96+13:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes nether.", m_name);
            dam = MIN(m_ptr->hp / 7, 550);
            breath(y, x, m_idx, GF_NETHER, dam,0, TRUE, MS_BR_NETHER, learnable);
            update_smart_learn(m_idx, DRS_NETH);
            break;
        }
        /* RF4_BR_LITE */
        case 96+14:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes light.", m_name);
            dam = MIN(m_ptr->hp / 6, 400);
            breath(y_br_lite, x_br_lite, m_idx, GF_LITE, dam,0, TRUE, MS_BR_LITE, learnable);
            update_smart_learn(m_idx, DRS_LITE);
            break;
        }
        /* RF4_BR_DARK */
        case 96+15:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes darkness.", m_name);
            dam = MIN(m_ptr->hp / 6, 400);
            breath(y, x, m_idx, GF_DARK, dam,0, TRUE, MS_BR_DARK, learnable);
            update_smart_learn(m_idx, DRS_DARK);
            break;
        }
        /* RF4_BR_CONF */
        case 96+16:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes confusion.", m_name);
            dam = MIN(m_ptr->hp / 6, 400);
            breath(y, x, m_idx, GF_CONFUSION, dam,0, TRUE, MS_BR_CONF, learnable);
            update_smart_learn(m_idx, DRS_CONF);
            break;
        }

        /* RF4_BR_SOUN */
        case 96+17:
        {
            disturb(1, 0);
            if (m_ptr->r_idx == MON_JAIAN) msg_format("'Booooeeeeee'");
            else if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes sound.", m_name);
            dam = MIN(m_ptr->hp / 6, 450);
            breath(y, x, m_idx, GF_SOUND, dam,0, TRUE, MS_BR_SOUND, learnable);
            update_smart_learn(m_idx, DRS_SOUND);
            break;
        }
        /* RF4_BR_CHAO */
        case 96+18:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes chaos.", m_name);
            dam = MIN(m_ptr->hp / 6, 600);
            breath(y, x, m_idx, GF_CHAOS, dam,0, TRUE, MS_BR_CHAOS, learnable);
            update_smart_learn(m_idx, DRS_CHAOS);
            break;
        }
        /* RF4_BR_DISE */
        case 96+19:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes disenchantment.", m_name);
            dam = MIN(m_ptr->hp / 6, 500);
            breath(y, x, m_idx, GF_DISENCHANT, dam,0, TRUE, MS_BR_DISEN, learnable);
            update_smart_learn(m_idx, DRS_DISEN);
            break;
        }
        /* RF4_BR_NEXU */
        case 96+20:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes nexus.", m_name);
            dam = MIN(m_ptr->hp / 3, 250);
            breath(y, x, m_idx, GF_NEXUS, dam,0, TRUE, MS_BR_NEXUS, learnable);
            update_smart_learn(m_idx, DRS_NEXUS);
            break;
        }
        /* RF4_BR_TIME */
        case 96+21:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes time.", m_name);
            dam = MIN(m_ptr->hp / 3, 150);
            breath(y, x, m_idx, GF_TIME, dam,0, TRUE, MS_BR_TIME, learnable);
            break;
        }
        /* RF4_BR_INER */
        case 96+22:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes inertia.", m_name);
            dam = MIN(m_ptr->hp / 6, 200);
            breath(y, x, m_idx, GF_INERT, dam,0, TRUE, MS_BR_INERTIA, learnable);
            break;
        }
        /* RF4_BR_GRAV */
        case 96+23:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes gravity.", m_name);
            dam = MIN(m_ptr->hp / 3, 200);
            breath(y, x, m_idx, GF_GRAVITY, dam,0, TRUE, MS_BR_GRAVITY, learnable);
            break;
        }
        /* RF4_BR_SHAR */
        case 96+24:
        {
            disturb(1, 0);
            if (m_ptr->r_idx == MON_BOTEI) msg_format("'Botei-Build cutter!!!'");
            else if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes shards.", m_name);
            dam = MIN(m_ptr->hp / 6, 500);
            breath(y, x, m_idx, GF_SHARDS, dam,0, TRUE, MS_BR_SHARDS, learnable);
            update_smart_learn(m_idx, DRS_SHARD);
            break;
        }
        /* RF4_BR_PLAS */
        case 96+25:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes plasma.", m_name);
            dam = MIN(m_ptr->hp / 6, 200);
            breath(y, x, m_idx, GF_PLASMA, dam,0, TRUE, MS_BR_PLASMA, learnable);
            break;
        }
        /* RF4_BR_WALL */
        case 96+26:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes force.", m_name);
            dam = MIN(m_ptr->hp / 3, 200);
            breath(y, x, m_idx, GF_FORCE, dam,0, TRUE, MS_BR_FORCE, learnable);
            break;
        }
        /* RF4_BR_MANA */
        case 96+27:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes mana.", m_name);
            dam = MIN(m_ptr->hp / 3, 250);
            breath(y, x, m_idx, GF_MANA, dam,0, TRUE, MS_BR_MANA, learnable);
            break;
        }
        /* RF4_BA_NUKE */
        case 96+28:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);
            else msg_format("%^s casts a ball of radiation.", m_name);
            dam = (rlev + damroll(10, 6)) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            breath(y, x, m_idx, GF_NUKE, dam, 2, FALSE, MS_BALL_NUKE, learnable);
            update_smart_learn(m_idx, DRS_POIS);
            break;
        }
        /* RF4_BR_NUKE */
        case 96+29:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes toxic waste.", m_name);
            dam = MIN(m_ptr->hp / 5, 600);
            breath(y, x, m_idx, GF_NUKE, dam,0, TRUE, MS_BR_NUKE, learnable);
            update_smart_learn(m_idx, DRS_POIS);
            break;
        }
        /* RF4_BA_CHAO */
        case 96+30:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles frighteningly.", m_name);
            else msg_format("%^s invokes a raw Logrus.", m_name);
            dam = ((r_ptr->flags2 & RF2_POWERFUL) ? (rlev * 3) : (rlev * 2))+ damroll(10, 10);
            breath(y, x, m_idx, GF_CHAOS, dam, 4, FALSE, MS_BALL_CHAOS, learnable);
            update_smart_learn(m_idx, DRS_CHAOS);
            break;
        }
        /* RF4_BR_DISI */
        case 96+31:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s breathes.", m_name);
            else msg_format("%^s breathes disintegration.", m_name);
            dam = MIN(m_ptr->hp / 6, 150);
            breath(y, x, m_idx, GF_DISINTEGRATE, dam,0, TRUE, MS_BR_DISI, learnable);
            break;
        }
        /* RF5_BA_ACID */
        case 128+0:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);
            else msg_format("%^s casts an acid ball.", m_name);
            dam = (randint1(rlev * 3) + 15) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            breath(y, x, m_idx, GF_ACID, dam, 2, FALSE, MS_BALL_ACID, learnable);
            update_smart_learn(m_idx, DRS_ACID);
            break;
        }
        /* RF5_BA_ELEC */
        case 128+1:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a lightning ball.", m_name);

            dam = (randint1(rlev * 3 / 2) + 8) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            breath(y, x, m_idx, GF_ELEC, dam, 2, FALSE, MS_BALL_ELEC, learnable);
            update_smart_learn(m_idx, DRS_ELEC);
            break;
        }

        /* RF5_BA_FIRE */
        case 128+2:
        {
            disturb(1, 0);

            if (m_ptr->r_idx == MON_ROLENTO)
            {
                if (blind)
                    msg_format("%^s throws something.", m_name);
                else
                    msg_format("%^s throws a hand grenade.", m_name);
            }
            else
            {
                if (blind) msg_format("%^s mumbles.", m_name);

                else msg_format("%^s casts a fire ball.", m_name);
            }

            dam = (randint1(rlev * 7 / 2) + 10) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            breath(y, x, m_idx, GF_FIRE, dam, 2, FALSE, MS_BALL_FIRE, learnable);
            update_smart_learn(m_idx, DRS_FIRE);
            break;
        }

        /* RF5_BA_COLD */
        case 128+3:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a frost ball.", m_name);

            dam = (randint1(rlev * 3 / 2) + 10) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            breath(y, x, m_idx, GF_COLD, dam, 2, FALSE, MS_BALL_COLD, learnable);
            update_smart_learn(m_idx, DRS_COLD);
            break;
        }

        /* RF5_BA_POIS */
        case 128+4:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a stinking cloud.", m_name);

            dam = damroll(12, 2) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            breath(y, x, m_idx, GF_POIS, dam, 2, FALSE, MS_BALL_POIS, learnable);
            update_smart_learn(m_idx, DRS_POIS);
            break;
        }

        /* RF5_BA_NETH */
        case 128+5:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a nether ball.", m_name);

            dam = 50 + damroll(10, 10) + (rlev * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1));
            breath(y, x, m_idx, GF_NETHER, dam, 2, FALSE, MS_BALL_NETHER, learnable);
            update_smart_learn(m_idx, DRS_NETH);
            break;
        }

        /* RF5_BA_WATE */
        case 128+6:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s gestures fluidly.", m_name);

            msg_print("You are engulfed in a whirlpool.");

            dam = ((r_ptr->flags2 & RF2_POWERFUL) ? randint1(rlev * 3) : randint1(rlev * 2)) + 50;
            breath(y, x, m_idx, GF_WATER, dam, 4, FALSE, MS_BALL_WATER, learnable);
            if (m_ptr->r_idx == MON_POSEIDON)
                fire_ball_hide(GF_WATER_FLOW, 0, 3, 8);
            break;
        }

        /* RF5_BA_MANA */
        case 128+7:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles powerfully.", m_name);

            else msg_format("%^s invokes a mana storm.", m_name);

            dam = (rlev * 4) + 50 + damroll(10, 10);
            breath(y, x, m_idx, GF_MANA, dam, 4, FALSE, MS_BALL_MANA, learnable);
            break;
        }

        /* RF5_BA_DARK */
        case 128+8:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles powerfully.", m_name);

            else msg_format("%^s invokes a darkness storm.", m_name);

            dam = (rlev * 4) + 50 + damroll(10, 10);
            breath(y, x, m_idx, GF_DARK, dam, 4, FALSE, MS_BALL_DARK, learnable);
            update_smart_learn(m_idx, DRS_DARK);
            break;
        }

        /* RF5_DRAIN_MANA */
        case 128+9:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);

            dam = (randint1(rlev) / 2) + 1;
            breath(y, x, m_idx, GF_DRAIN_MANA, dam, 0, FALSE, MS_DRAIN_MANA, learnable);
            update_smart_learn(m_idx, DRS_MANA);
            break;
        }

        /* RF5_MIND_BLAST */
        case 128+10:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (!seen)
            {
                msg_print("You feel something focusing on your mind.");

            }
            else
            {
                msg_format("%^s gazes deep into your eyes.", m_name);

            }

            dam = damroll(7, 7);
            breath(y, x, m_idx, GF_MIND_BLAST, dam, 0, FALSE, MS_MIND_BLAST, learnable);
            break;
        }

        /* RF5_BRAIN_SMASH */
        case 128+11:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (!seen)
            {
                msg_print("You feel something focusing on your mind.");

            }
            else
            {
                msg_format("%^s looks deep into your eyes.", m_name);

            }

            dam = damroll(12, 12);
            breath(y, x, m_idx, GF_BRAIN_SMASH, dam, 0, FALSE, MS_BRAIN_SMASH, learnable);
            break;
        }

        /* RF5_CAUSE_1 */
        case 128+12:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s points at you and curses.", m_name);

            dam = damroll(3, 8);
            breath(y, x, m_idx, GF_CAUSE_1, dam, 0, FALSE, MS_CAUSE_1, learnable);
            break;
        }

        /* RF5_CAUSE_2 */
        case 128+13:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s points at you and curses horribly.", m_name);

            dam = damroll(8, 8);
            breath(y, x, m_idx, GF_CAUSE_2, dam, 0, FALSE, MS_CAUSE_2, learnable);
            break;
        }

        /* RF5_CAUSE_3 */
        case 128+14:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles loudly.", m_name);

            else msg_format("%^s points at you, incanting terribly!", m_name);

            dam = damroll(10, 15);
            breath(y, x, m_idx, GF_CAUSE_3, dam, 0, FALSE, MS_CAUSE_3, learnable);
            break;
        }

        /* RF5_CAUSE_4 */
        case 128+15:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s screams the word 'DIE!'", m_name);

            else msg_format("%^s points at you, screaming the word DIE!", m_name);

            dam = damroll(15, 15);
            breath(y, x, m_idx, GF_CAUSE_4, dam, 0, FALSE, MS_CAUSE_4, learnable);
            break;
        }

        /* RF5_BO_ACID */
        case 128+16:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a acid bolt.", m_name);

            dam = (damroll(7, 8) + (rlev / 3)) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            bolt(m_idx, GF_ACID, dam, MS_BOLT_ACID, learnable);
            update_smart_learn(m_idx, DRS_ACID);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_ELEC */
        case 128+17:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a lightning bolt.", m_name);

            dam = (damroll(4, 8) + (rlev / 3)) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            bolt(m_idx, GF_ELEC, dam, MS_BOLT_ELEC, learnable);
            update_smart_learn(m_idx, DRS_ELEC);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_FIRE */
        case 128+18:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a fire bolt.", m_name);

            dam = (damroll(9, 8) + (rlev / 3)) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            bolt(m_idx, GF_FIRE, dam, MS_BOLT_FIRE, learnable);
            update_smart_learn(m_idx, DRS_FIRE);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_COLD */
        case 128+19:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a frost bolt.", m_name);

            dam = (damroll(6, 8) + (rlev / 3)) * ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 1);
            bolt(m_idx, GF_COLD, dam, MS_BOLT_COLD, learnable);
            update_smart_learn(m_idx, DRS_COLD);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BA_LITE */
        case 128+20:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles powerfully.", m_name);

            else msg_format("%^s invokes a starburst.", m_name);

            dam = (rlev * 4) + 50 + damroll(10, 10);
            breath(y, x, m_idx, GF_LITE, dam, 4, FALSE, MS_STARBURST, learnable);
            update_smart_learn(m_idx, DRS_LITE);
            break;
        }

        /* RF5_BO_NETH */
        case 128+21:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a nether bolt.", m_name);

            dam = 30 + damroll(5, 5) + (rlev * 4) / ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 3);
            bolt(m_idx, GF_NETHER, dam, MS_BOLT_NETHER, learnable);
            update_smart_learn(m_idx, DRS_NETH);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_WATE */
        case 128+22:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a water bolt.", m_name);

            dam = damroll(10, 10) + (rlev * 3 / ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 3));
            bolt(m_idx, GF_WATER, dam, MS_BOLT_WATER, learnable);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_MANA */
        case 128+23:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a mana bolt.", m_name);

            dam = randint1(rlev * 7 / 2) + 50;
            bolt(m_idx, GF_MANA, dam, MS_BOLT_MANA, learnable);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_PLAS */
        case 128+24:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a plasma bolt.", m_name);

            dam = 10 + damroll(8, 7) + (rlev * 3 / ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 3));
            bolt(m_idx, GF_PLASMA, dam, MS_BOLT_PLASMA, learnable);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_BO_ICEE */
        case 128+25:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts an ice bolt.", m_name);

            dam = damroll(6, 6) + (rlev * 3 / ((r_ptr->flags2 & RF2_POWERFUL) ? 2 : 3));
            bolt(m_idx, GF_ICE, dam, MS_BOLT_ICE, learnable);
            update_smart_learn(m_idx, DRS_COLD);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_MISSILE */
        case 128+26:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a magic missile.", m_name);

            dam = damroll(2, 6) + (rlev / 3);
            bolt(m_idx, GF_MISSILE, dam, MS_MAGIC_MISSILE, learnable);
            update_smart_learn(m_idx, DRS_REFLECT);
            break;
        }

        /* RF5_SCARE */
        case 128+27:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles, and you hear scary noises.", m_name);

            else msg_format("%^s casts a fearful illusion.", m_name);

            fear_scare_p(m_ptr);
            learn_spell(MS_SCARE);
            update_smart_learn(m_idx, DRS_FEAR);
            if (p_ptr->tim_spell_reaction && !p_ptr->fast)
            {
                set_fast(4, FALSE);
            }
            break;
        }

        /* RF5_BLIND */
        case 128+28:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);
            else msg_format("%^s casts a spell, burning your eyes!", m_name);

            if (res_save_default(RES_BLIND))
                msg_print("You are unaffected!");
            else if (randint0(100 + rlev/2) < duelist_skill_sav(m_idx))
                msg_print("You resist the effects!");
            else
                (void)set_blind(12 + randint0(4), FALSE);
            learn_spell(MS_BLIND);
            update_smart_learn(m_idx, DRS_BLIND);
            if (p_ptr->tim_spell_reaction && !p_ptr->fast)
            {
                set_fast(4, FALSE);
            }
            break;
        }

        /* RF5_CONF */
        case 128+29:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles, and you hear puzzling noises.", m_name);
            else msg_format("%^s creates a mesmerising illusion.", m_name);

            if (res_save_default(RES_CONF))
                msg_print("You disbelieve the feeble spell.");
            else if (randint0(100 + rlev/2) < duelist_skill_sav(m_idx))
                msg_print("You disbelieve the feeble spell.");
            else
                (void)set_confused(p_ptr->confused + randint0(4) + 4, FALSE);
            learn_spell(MS_CONF);
            update_smart_learn(m_idx, DRS_CONF);
            if (p_ptr->tim_spell_reaction && !p_ptr->fast)
            {
                set_fast(4, FALSE);
            }
            break;
        }

        /* RF5_SLOW */
        case 128+30:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            msg_format("%^s drains power from your muscles!", m_name);

            if (p_ptr->free_act)
            {
                msg_print("You are unaffected!");
                equip_learn_flag(OF_FREE_ACT);
            }
            else if (randint0(100 + rlev/2) < duelist_skill_sav(m_idx))
                msg_print("You resist the effects!");
            else
                (void)set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
            learn_spell(MS_SLOW);
            update_smart_learn(m_idx, DRS_FREE);
            if (p_ptr->tim_spell_reaction && !p_ptr->fast)
            {
                set_fast(4, FALSE);
            }
            break;
        }

        /* RF5_HOLD */
        case 128+31:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);
            else msg_format("%^s stares deep into your eyes!", m_name);

            if (p_ptr->free_act)
            {
                msg_print("You are unaffected!");
                equip_learn_flag(OF_FREE_ACT);
            }
            else if (randint0(100 + rlev/2) < duelist_skill_sav(m_idx))
                msg_format("You resist the effects!");
            else
                set_paralyzed(randint1(3), FALSE);
            learn_spell(MS_SLEEP);
            update_smart_learn(m_idx, DRS_FREE);
            if (p_ptr->tim_spell_reaction && !p_ptr->fast)
            {
                set_fast(4, FALSE);
            }
            break;
        }

        /* RF6_HASTE */
        case 160+0:
        {
            if (!wall_scummer) disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);
            else msg_format("%^s concentrates on %s body.", m_name, m_poss);

            /* Allow quick speed increases to base+10 */
            if (set_monster_fast(m_idx, MON_FAST(m_ptr) + 100))
                msg_format("%^s starts moving faster.", m_name);
            break;
        }

        /* RF6_HAND_DOOM */
        case 160+1:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            msg_format("%^s invokes the Hand of Doom!", m_name);
            dam = (((s32b) ((40 + randint1(20)) * (p_ptr->chp))) / 100);
            breath(y, x, m_idx, GF_HAND_DOOM, dam, 0, FALSE, MS_HAND_DOOM, learnable);
            break;
        }

        /* RF6_HEAL */
        case 160+2:
        {
            if (!wall_scummer) disturb(1, 0);

            /* Message */
            if (blind)
            {
                msg_format("%^s mumbles.", m_name);

            }
            else
            {
                msg_format("%^s concentrates on %s wounds.", m_name, m_poss);

            }

            /* Heal some */
            if (m_ptr->r_idx == MON_DEMETER)
                m_ptr->hp += 1000;
            else
                m_ptr->hp += (rlev * 6);

            /* Fully healed */
            if (m_ptr->hp >= m_ptr->maxhp)
            {
                /* Fully healed */
                m_ptr->hp = m_ptr->maxhp;

                /* Message */
                if (seen)
                {
                    msg_format("%^s looks completely healed!", m_name);

                }
                else
                {
                    msg_format("%^s sounds completely healed!", m_name);

                }
            }

            /* Partially healed */
            else
            {
                /* Message */
                if (seen)
                {
                    msg_format("%^s looks healthier.", m_name);

                }
                else
                {
                    msg_format("%^s sounds healthier.", m_name);

                }
            }

            /* Redraw (later) if needed */
            check_mon_health_redraw(m_idx);

            /* Cancel fear */
            if (MON_MONFEAR(m_ptr))
            {
                /* Cancel fear */
                (void)set_monster_monfear(m_idx, 0);

                /* Message */
                msg_format("%^s recovers %s courage.", m_name, m_poss);
            }
            break;
        }

        /* RF6_INVULNER */
        case 160+3:
        {
            disturb(1, 0);

            /* Message */
            if (!seen)
            {
                msg_format("%^s mumbles powerfully.", m_name);

            }
            else
            {
                msg_format("%^s casts a Globe of Invulnerability.", m_name);

            }

            if (!MON_INVULNER(m_ptr)) (void)set_monster_invulner(m_idx, randint1(4) + 4, FALSE);
            break;
        }

        /* RF6_BLINK */
        case 160+4:
        {
            if (!wall_scummer) disturb(1, 0);
			char tmp_str[80];
            if (teleport_barrier(m_idx, tmp_str))
            {
                msg_format("%^s obstructs teleporting of %^s.", tmp_str,m_name);
            }
            else
            {
                if (seen)
                    msg_format("%^s blinks away.", m_name);
                teleport_away(m_idx, 10, 0L);
                p_ptr->update |= (PU_MONSTERS);
            }
            break;
        }

        /* RF6_TPORT */
        case 160+5:
        {
            if (!wall_scummer) disturb(1, 0);
			char tmp_str[80];
            if (teleport_barrier(m_idx, tmp_str))
            {
                msg_format("%^s obstructs teleporting of %^s.", tmp_str, m_name);
            }
            else
            {
                if (seen)
                    msg_format("%^s teleports away.", m_name);
                teleport_away_followable(m_idx);
            }
            break;
        }

        /* RF6_WORLD */
        case 160+6:
        {
            int who = 0;
            disturb(1, 0);
            if(m_ptr->r_idx == MON_DIO) who = 1;
            else if(m_ptr->r_idx == MON_WONG) who = 3;
            dam = who;
            if (!process_the_world(randint1(2)+2, who, TRUE)) return (FALSE);
            break;
        }

        /* RF6_SPECIAL */
        case 160+7:
        {
            int k;

            disturb(1, 0);
            switch (m_ptr->r_idx)
            {
            case MON_OHMU:
                /* Moved to process_monster(), like multiplication */
                return FALSE;

            case MON_BANORLUPART:
                {
                    int dummy_hp = (m_ptr->hp + 1) / 2;
                    int dummy_maxhp = m_ptr->maxhp/2;
                    int dummy_y = m_ptr->fy;
                    int dummy_x = m_ptr->fx;

                    if (p_ptr->inside_arena || p_ptr->inside_battle || !summon_possible(m_ptr->fy, m_ptr->fx)) return FALSE;
                    delete_monster_idx(cave[m_ptr->fy][m_ptr->fx].m_idx);
                    summon_named_creature(0, dummy_y, dummy_x, MON_BANOR, mode);
                    m_list[hack_m_idx_ii].hp = dummy_hp;
                    m_list[hack_m_idx_ii].maxhp = dummy_maxhp;
                    summon_named_creature(0, dummy_y, dummy_x, MON_LUPART, mode);
                    m_list[hack_m_idx_ii].hp = dummy_hp;
                    m_list[hack_m_idx_ii].maxhp = dummy_maxhp;

                    msg_print("Banor=Rupart splits in two person!");

                    break;
                }

            case MON_BANOR:
            case MON_LUPART:
                {
                    int dummy_hp = 0;
                    int dummy_maxhp = 0;
                    int dummy_y = m_ptr->fy;
                    int dummy_x = m_ptr->fx;

                    if (!r_info[MON_BANOR].cur_num || !r_info[MON_LUPART].cur_num) return (FALSE);
                    for (k = 1; k < m_max; k++)
                    {
                        if (m_list[k].r_idx == MON_BANOR || m_list[k].r_idx == MON_LUPART)
                        {
                            dummy_hp += m_list[k].hp;
                            dummy_maxhp += m_list[k].maxhp;
                            if (m_list[k].r_idx != m_ptr->r_idx)
                            {
                                dummy_y = m_list[k].fy;
                                dummy_x = m_list[k].fx;
                            }
                            delete_monster_idx(k);
                        }
                    }
                    summon_named_creature(0, dummy_y, dummy_x, MON_BANORLUPART, mode);
                    m_list[hack_m_idx_ii].hp = dummy_hp;
                    m_list[hack_m_idx_ii].maxhp = dummy_maxhp;

                    msg_print("Banor and Rupart combine into one!");

                    break;
                }
            case MON_SANTACLAUS:
                {
                    int num = randint1(4);
                    msg_format("%^s says 'Now Dasher! Now Dancer! Now, Prancer and Vixen! On, Comet! On, Cupid! On, Donner and Blitzen!'", m_name);
                    for (k = 0; k < num; k++)
                    {
                        summon_named_creature(m_idx, y, x, MON_REINDEER, mode);
                    }
                    break;
                }
            case MON_ZEUS:
            {
                int num = randint1(4);
                msg_format("%^s summons Shamblers!", m_name);
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_SHAMBLER, mode);
                }
                break;
            }
            case MON_POSEIDON:
            {
                int num = randint1(4);
                fire_ball_hide(GF_WATER_FLOW, 0, 3, 8);
                msg_format("%^s summons Greater Kraken!", m_name);
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_GREATER_KRAKEN, mode);
                }
                break;
            }
            case MON_HADES:
            {
                int num = randint1(2);
                fire_ball_hide(GF_LAVA_FLOW, 0, 3, 8);
                msg_format("%^s summons Death!", m_name);
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_GREATER_BALROG, mode);
                }
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_ARCHLICH, mode);
                }
                break;
            }
            case MON_ATHENA:
            {
                int num = randint1(2);
                msg_format("%^s summons friends!", m_name);
                if (one_in_(3) && r_info[MON_ZEUS].max_num == 1)
                {
                    if (summon_named_creature(m_idx, y, x, MON_ZEUS, mode))
                        break;
                }

                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_ULT_MAGUS, mode);
                }
                break;
            }
            case MON_ARES:
            {
                msg_format("%^s yells 'Mommy! Daddy! Help!!'", m_name);
                if (r_info[MON_ZEUS].max_num == 1)
                {
                    summon_named_creature(m_idx, y, x, MON_ZEUS, mode);
                }
                if (r_info[MON_HERA].max_num == 1)
                {
                    summon_named_creature(m_idx, y, x, MON_HERA, mode);
                }
                break;
            }
            case MON_APOLLO:
            {
                int num = randint1(4);
                msg_format("%^s summons help!", m_name);
                if (one_in_(3) && r_info[MON_ARTEMIS].max_num == 1)
                {
                    if (summon_named_creature(m_idx, y, x, MON_ARTEMIS, mode))
                        break;
                }
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_FENGHUANG, mode);
                }
                break;
            }
            case MON_ARTEMIS:
            {
                msg_format("%^s summons help!", m_name);
                if (r_info[MON_APOLLO].max_num == 1)
                {
                    summon_named_creature(m_idx, y, x, MON_APOLLO, mode);
                }
                break;
            }
            case MON_HEPHAESTUS:
            {
                int num = randint1(4);
                msg_format("%^s summons friends!", m_name);
                if (one_in_(3) && r_info[MON_ZEUS].max_num == 1)
                {
                    if (summon_named_creature(m_idx, y, x, MON_ZEUS, mode))
                        break;
                }

                if (one_in_(3) && r_info[MON_HERA].max_num == 1)
                {
                    summon_named_creature(m_idx, y, x, MON_HERA, mode);
                }
                else
                {
                    for (k = 0; k < num; k++)
                    {
                        summon_named_creature(m_idx, y, x, MON_SPELLWARP, mode);
                    }
                }
                break;
            }
            case MON_HERMES:
            {
                int num = randint1(16);
                msg_format("%^s summons friends!", m_name);
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_MAGIC_MUSHROOM, mode);
                }
                break;
            }
            case MON_HERA:
            {
                int num = randint1(4);
                msg_format("%^s summons aid!'", m_name);
                if (one_in_(3) && r_info[MON_ARES].max_num == 1)
                {
                    summon_named_creature(m_idx, y, x, MON_ARES, mode);
                }
                else if (one_in_(3) && r_info[MON_HEPHAESTUS].max_num == 1)
                {
                    summon_named_creature(m_idx, y, x, MON_HEPHAESTUS, mode);
                }
                else
                {
                    for (k = 0; k < num; k++)
                    {
                        summon_named_creature(m_idx, y, x, MON_DEATH_BEAST, mode);
                    }
                }
                break;
            }
            case MON_DEMETER:
            {
                int num = randint1(4);
                msg_format("%^s summons ents!", m_name);
                for (k = 0; k < num; k++)
                {
                    summon_named_creature(m_idx, y, x, MON_ENT, mode);
                }
                break;
            }
            case MON_ROLENTO:
                if (blind) msg_format("%^s spreads something.", m_name);
                else msg_format("%^s throws some hand grenades.", m_name);

                {
                    int num = 1 + randint1(3);

                    for (k = 0; k < num; k++)
                    {
                        count += summon_named_creature(m_idx, y, x, MON_SHURYUUDAN, mode);
                    }
                }
                if (blind && count) msg_print("You hear many things are scattered nearby.");
                break;

            default:
                if (r_ptr->d_char == 'B')
                {
                    disturb(1, 0);
                    if (one_in_(3) || !direct)
                    {
                        msg_format("%^s suddenly go out of your sight!", m_name);
                        teleport_away(m_idx, 10, TELEPORT_NONMAGICAL);
                        p_ptr->update |= (PU_MONSTERS);
                    }
                    else
                    {
                        int get_damage = 0;
                        bool fear; /* dummy */

                        msg_format("%^s holds you, and drops from the sky.", m_name);
                        dam = damroll(4, 8);
                        teleport_player_to(m_ptr->fy, m_ptr->fx, TELEPORT_NONMAGICAL | TELEPORT_PASSIVE);

                        sound(SOUND_FALL);

                        if (p_ptr->levitation)
                        {
                            msg_print("You float gently down to the ground.");
                        }
                        else
                        {
                            msg_print("You crashed into the ground.");
                            dam += damroll(6, 8);
                        }

                        /* Mega hack -- this special action deals damage to the player. Therefore the code of "eyeeye" is necessary.
                           -- henkma
                         */
                        get_damage = take_hit(DAMAGE_NOESCAPE, dam, m_name, -1);
                        if (get_damage > 0)
                            weaponmaster_do_readied_shot(m_ptr);

                        if (IS_REVENGE() && get_damage > 0 && !p_ptr->is_dead)
                        {
                            char m_name_self[80];

                            /* hisself */
                            monster_desc(m_name_self, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE | MD_OBJECTIVE);

                            msg_format("The attack of %s has wounded %s!", m_name, m_name_self);
                            project(0, 0, m_ptr->fy, m_ptr->fx, psion_backlash_dam(get_damage), GF_MISSILE, PROJECT_KILL, -1);
                            if (p_ptr->tim_eyeeye)
                                set_tim_eyeeye(p_ptr->tim_eyeeye-5, TRUE);
                        }

                        if (p_ptr->riding) mon_take_hit_mon(p_ptr->riding, dam, &fear, extract_note_dies(real_r_ptr(&m_list[p_ptr->riding])), m_idx);
                    }
                    break;
                }

                /* Something is wrong */
                else return FALSE;
            }
            break;
        }

        /* RF6_TELE_TO */
        case 160+8:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            msg_format("%^s commands you to return.", m_name);

            /* Only powerful monsters can choose this spell when the player is not in
               los. In this case, it is nasty enough to warrant a saving throw. */
            if (!projectable(m_ptr->fy, m_ptr->fx, py, px)
              && randint1(100) <= duelist_skill_sav(m_idx) - r_ptr->level/2 )
            {
                msg_print("You resist the effects!");
            }
            else if (res_save_default(RES_TELEPORT))
            {
                msg_print("You resist the effects!");
            }
            else
            {
                teleport_player_to(m_ptr->fy, m_ptr->fx, TELEPORT_PASSIVE);
                learn_spell(MS_TELE_TO);
            }
            break;
        }

        /* RF6_TELE_AWAY */
        case 160+9:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            learn_spell(MS_TELE_AWAY);

            /* Duelist Unending Pursuit */
            if ( p_ptr->pclass == CLASS_DUELIST
              && p_ptr->duelist_target_idx == m_idx
              && p_ptr->lev >= 30 )
            {
                if (get_check(format("%^s is attempting to teleport you. Prevent? ", m_name)))
                {
                    if (one_in_(3))
                        msg_print("Failed!");
                    else
                    {
                        msg_print("You invoke Unending Pursuit ... The duel continues!");
                        break;
                    }
                }
            }

            msg_format("%^s teleports you away.", m_name);
            if (res_save_default(RES_TELEPORT))
                msg_print("You resist the effects!");
            else
                teleport_player_away(m_idx, 100);
            break;
        }

        /* RF6_TELE_LEVEL */
        case 160+10:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles strangely.", m_name);
            else msg_format("%^s gestures at your feet.", m_name);

            if (res_save_default(RES_NEXUS))
                msg_print("You are unaffected!");
            else if (randint0(100 + rlev/2) < duelist_skill_sav(m_idx))
                msg_print("You resist the effects!");
            else
                teleport_level(0);
            learn_spell(MS_TELE_LEVEL);
            update_smart_learn(m_idx, DRS_NEXUS);
            break;
        }

        /* RF6_PSY_SPEAR */
        case 160+11:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s throws a Psycho-Spear.", m_name);

            dam = (r_ptr->flags2 & RF2_POWERFUL) ? (randint1(rlev * 2) + 150) : (randint1(rlev * 3 / 2) + 100);
            beam(m_idx, GF_PSY_SPEAR, dam, MS_PSY_SPEAR, learnable);
            break;
        }

        /* RF6_DARKNESS */
        case 160+12:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else if (can_use_lite_area) msg_format("%^s cast a spell to light up.", m_name);
            else msg_format("%^s gestures in shadow.", m_name);

            if (can_use_lite_area) (void)lite_area(0, 3);
            else
            {
                learn_spell(MS_DARKNESS);
                (void)unlite_area(0, 3);
            }
            break;
        }

        /* RF6_TRAPS */
        case 160+13:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles, and then cackles evilly.", m_name);

            else msg_format("%^s casts a spell and cackles evilly.", m_name);

            learn_spell(MS_MAKE_TRAP);
            (void)trap_creation(y, x);
            break;
        }

        /* RF6_FORGET */
        case 160+14:
        {
            if (!direct) return (FALSE);
            disturb(1, 0);
            msg_format("%^s tries to blank your mind.", m_name);

            if (randint0(100 + rlev/2) < duelist_skill_sav(m_idx))
            {
                msg_print("You resist the effects!");
            }
            else if (lose_all_info())
            {
                msg_print("Your memories fade away.");
            }
            learn_spell(MS_FORGET);
            break;
        }

        /* RF6_RAISE_DEAD */
        case 160+15:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s casts a spell to revive corpses.", m_name);
            animate_dead(m_idx, m_ptr->fy, m_ptr->fx);
            break;
        }

        /* RF6_S_KIN */
        case 160+16:
        {
            if (r_ptr->flags3 & RF3_OLYMPIAN)
            {
                disturb(1, 0);
                if (blind) msg_format("%^s mumbles.", m_name);
                else msg_format("%^s magically summons Olympians!", m_name);

                for (k = 0; k < 2; k++)
                {
                    count += summon_specific(
                        m_idx,
                        y,
                        x,
                        500 /*rlev - Hack: Olympain Summoning should never fail!*/,
                        SUMMON_OLYMPIAN,
                        PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode
                    );
                }
                if (blind && count)
                {
                    msg_print("You hear immortal beings appear nearby.");
                }
                break;
            }
            else if (m_ptr->r_idx == MON_CAMELOT_KNIGHT)
            {
                disturb(1, 0);
                if (blind) msg_format("%^s mumbles.", m_name);
                else msg_format("%^s magically summons Knights!", m_name);

                for (k = 0; k < 2; k++)
                {
                    count += summon_specific(
                        m_idx,
                        y,
                        x,
                        rlev,
                        SUMMON_KNIGHT,
                        PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode
                    );
                }
                if (blind && count)
                {
                    msg_print("You hear brave beings appear nearby.");
                }
                break;
            }
            else if (r_ptr->flags2 & RF2_CAMELOT)
            {
                disturb(1, 0);
                if (blind) msg_format("%^s mumbles.", m_name);
                else msg_format("%^s magically summons Knights of the Round Table!", m_name);

                for (k = 0; k < 2; k++)
                {
                    count += summon_specific(
                        m_idx,
                        y,
                        x,
                        rlev,
                        SUMMON_CAMELOT,
                        PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode
                    );
                }
                if (!count) /* In case they are all dead ... */
                {
                    for (k = 0; k < 2; k++)
                    {
                        count += summon_specific(
                            m_idx,
                            y,
                            x,
                            rlev,
                            SUMMON_KNIGHT,
                            PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode
                        );
                    }
                }
                if (blind && count)
                {
                    msg_print("You hear noble beings appear nearby.");
                }
                break;
            }
            else if (m_ptr->r_idx == MON_GRAND_FEARLORD || m_ptr->r_idx == MON_HYPNOS)
            {
                disturb(1, 0);
                if (blind) msg_format("%^s mumbles.", m_name);
                else msg_format("%^s magically summons your worst nightmares!", m_name);

                for (k = 0; k < s_num_4; k++)
                {
                    count += summon_specific(
                        m_idx,
                        y,
                        x,
                        rlev,
                        SUMMON_NIGHTMARE,
                        PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode
                    );
                }
                if (blind && count)
                {
                    msg_print("You hear terrifying things appear nearby.");
                }
                break;
            }
            disturb(1, 0);
            if (m_ptr->r_idx == MON_SERPENT || m_ptr->r_idx == MON_ZOMBI_SERPENT)
            {
                if (blind)
                    msg_format("%^s mumbles.", m_name);
                else
                    msg_format("%^s magically summons guardians of dungeons.", m_name);
            }
            else if (m_ptr->r_idx != MON_VARIANT_MAINTAINER)
            {
                if (blind)
                    msg_format("%^s mumbles.", m_name);
                else
                    msg_format("%^s magically summons %s %s.",
                    m_name, m_poss,
                    ((r_ptr->flags1) & RF1_UNIQUE ?
                    "minions" : "kin"));
            }

            switch (m_ptr->r_idx)
            {
            case MON_MENELDOR:
            case MON_GWAIHIR:
            case MON_THORONDOR:
                {
                    int num = 4 + randint1(3);
                    for (k = 0; k < num; k++)
                    {
                        count += summon_specific(m_idx, y, x, rlev, SUMMON_EAGLE, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
                    }
                }
                break;

            case MON_BULLGATES:
                {
                    int num = 2 + randint1(3);
                    for (k = 0; k < num; k++)
                    {
                        count += summon_named_creature(m_idx, y, x, MON_IE, mode);
                    }
                }
                break;

            case MON_SERPENT:
            case MON_ZOMBI_SERPENT:
                {
                    int num = 2 + randint1(3);

                    if (r_info[MON_JORMUNGAND].cur_num < r_info[MON_JORMUNGAND].max_num && one_in_(6))
                    {
                        msg_print("Water blew off from the ground!");
                        fire_ball_hide(GF_WATER_FLOW, 0, 3, 8);
                    }

                    for (k = 0; k < num; k++)
                    {
                        count += summon_specific(m_idx, y, x, rlev, SUMMON_GUARDIAN, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
                    }
                }
                break;

            case MON_CALDARM:
                {
                    int num = randint1(3);
                    for (k = 0; k < num; k++)
                    {
                        count += summon_named_creature(m_idx, y, x, MON_LOCKE_CLONE, mode);
                    }
                }
                break;

            case MON_TALOS:
                {
                    int num = randint1(3);
                    for (k = 0; k < num; k++)
                    {
                        count += summon_named_creature(m_idx, y, x, MON_SPELLWARP, mode);
                    }
                }
                break;

            case MON_MASTER_TONBERRY:
                {
                    int num = randint1(3);
                    for (k = 0; k < num; k++)
                    {
                        if (one_in_(3))
                            count += summon_named_creature(m_idx, y, x, MON_NINJA_TONBERRY, mode);
                        else
                            count += summon_named_creature(m_idx, y, x, MON_TONBERRY, mode);
                    }
                }
                break;
            case MON_LOUSY:
                {
                    int num = 2 + randint1(3);
                    for (k = 0; k < num; k++)
                    {
                        count += summon_specific(m_idx, y, x, rlev, SUMMON_LOUSE, PM_ALLOW_GROUP | mode);
                    }
                }
                break;
            case MON_VARIANT_MAINTAINER:
                {
                    int num = 2 + randint1(3);
                    switch (randint1(7))
                    {
                    case 1:
                        msg_format("%^s says, 'I just finished coding up something sweet!'", m_name);
                        break;
                    case 2:
                        msg_format("%^s says, 'It compiles, so it ought to work just fine!'", m_name);
                        break;
                    case 3:
                        msg_format("%^s says, 'Hack him to pieces, my pretties!'", m_name);
                        break;
                    case 4:
                        msg_format("%^s says, 'Just when you thought you fixed the last of 'em ...'", m_name);
                        break;
                    case 5:
                        msg_format("%^s says, 'They're not bugs, they're features!'", m_name);
                        break;
                    case 6:
                        msg_format("%^s says, 'Talk to QA about these guys!'", m_name);
                        break;
                    case 7:
                        msg_format("%^s summons Cyberdemons. %^s says, 'Doh!'", m_name, m_name);
                        break;
                    }


                    for (k = 0; k < num; k++)
                    {
                        count += summon_specific(m_idx, y, x, rlev, SUMMON_SOFTWARE_BUG, PM_ALLOW_GROUP | mode);
                    }
                }
                break;

            default:
                summon_kin_type = r_ptr->d_char; /* Big hack */

                for (k = 0; k < 4; k++)
                {
                    count += summon_specific(m_idx, y, x, rlev, SUMMON_KIN, PM_ALLOW_GROUP | mode);
                }
                break;
            }
            if (blind && count) msg_print("You hear many things appear nearby.");

            break;
        }

        /* RF6_S_CYBER */
        case 160+17:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons Cyberdemons!", m_name);

            if (blind && count) msg_print("You hear heavy steps nearby.");

            summon_cyber(m_idx, y, x);
            break;
        }

        /* RF6_S_MONSTER */
        case 160+18:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons help!", m_name);

            for (k = 0; k < 1; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, 0, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
            }
            if (blind && count) msg_print("You hear something appear nearby.");

            break;
        }

        /* RF6_S_MONSTERS */
        case 160+19:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons monsters!", m_name);

            for (k = 0; k < s_num_6; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, 0, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
            }
            if (blind && count) msg_print("You hear many things appear nearby.");

            break;
        }

        /* RF6_S_ANT */
        case 160+20:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons ants.", m_name);

            for (k = 0; k < s_num_6; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_ANT, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear many things appear nearby.");

            break;
        }

        /* RF6_S_SPIDER */
        case 160+21:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons spiders.", m_name);

            for (k = 0; k < s_num_6; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_SPIDER, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear many things appear nearby.");

            break;
        }

        /* RF6_S_HOUND */
        case 160+22:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons hounds.", m_name);

            for (k = 0; k < s_num_4; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_HOUND, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear many things appear nearby.");

            break;
        }

        /* RF6_S_HYDRA */
        case 160+23:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons hydras.", m_name);

            for (k = 0; k < s_num_4; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_HYDRA, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear many things appear nearby.");

            break;
        }

        /* RF6_S_ANGEL */
        case 160+24:
        {
            int num = 1;

            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons an angel!", m_name);

            if ((r_ptr->flags1 & RF1_UNIQUE))
            {
                num += r_ptr->level/40;
            }

            for (k = 0; k < num; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_ANGEL, PM_ALLOW_GROUP | mode);
            }

            if (count < 2)
            {
                if (blind && count) msg_print("You hear something appear nearby.");
            }
            else
            {
                if (blind) msg_print("You hear many things appear nearby.");
            }

            break;
        }

        /* RF6_S_DEMON */
        case 160+25:
        {
            int type = SUMMON_DEMON;

            if (r_ptr->level >= 70) /* We are completely out of bits in RF6_* */
                type = SUMMON_HI_DEMON;

            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons a demon from the Courts of Chaos!", m_name);

            for (k = 0; k < 1; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, type, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear something appear nearby.");

            break;
        }

        /* RF6_S_UNDEAD */
        case 160+26:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons an undead adversary!", m_name);

            for (k = 0; k < 1; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_UNDEAD, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear something appear nearby.");

            break;
        }

        /* RF6_S_DRAGON */
        case 160+27:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons a dragon!", m_name);

            for (k = 0; k < 1; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_DRAGON, PM_ALLOW_GROUP | mode);
            }
            if (blind && count) msg_print("You hear something appear nearby.");

            break;
        }

        /* RF6_S_HI_UNDEAD */
        case 160+28:
        {
            disturb(1, 0);

            if (((m_ptr->r_idx == MON_MORGOTH) || (m_ptr->r_idx == MON_SAURON) || (m_ptr->r_idx == MON_ANGMAR)) && ((r_info[MON_NAZGUL].cur_num+2) < r_info[MON_NAZGUL].max_num))
            {
                int cy = y;
                int cx = x;

                if (blind) msg_format("%^s mumbles.", m_name);

                else msg_format("%^s magically summons rangers of Nazgul!", m_name);
                msg_print(NULL);

                for (k = 0; k < 30; k++)
                {
                    if (!summon_possible(cy, cx) || !cave_empty_bold(cy, cx))
                    {
                        int j;
                        for (j = 100; j > 0; j--)
                        {
                            scatter(&cy, &cx, y, x, 2, 0);
                            if (cave_empty_bold(cy, cx)) break;
                        }
                        if (!j) break;
                    }
                    if (!cave_empty_bold(cy, cx)) continue;

                    if (summon_named_creature(m_idx, cy, cx, MON_NAZGUL, mode))
                    {
                        y = cy;
                        x = cx;
                        count++;
                        if (count == 1)
                            msg_format("A Nazgul says 'Nazgul-Rangers Number %d, Nazgul-Black!'",count);
                        else
                            msg_format("Another one says 'Number %d, Nazgul-Black!'",count);
                        msg_print(NULL);
                    }
                }
msg_format("They say 'The %d meets! We are the Ring-Ranger!'.", count);
                msg_print(NULL);
            }
            else
            {
                if (blind) msg_format("%^s mumbles.", m_name);

                else msg_format("%^s magically summons greater undead!", m_name);

                for (k = 0; k < s_num_4; k++)
                {
                    count += summon_specific(m_idx, y, x, rlev, SUMMON_HI_UNDEAD, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
                }
            }
            if (blind && count)
            {
                msg_print("You hear many creepy things appear nearby.");

            }
            break;
        }

        /* RF6_S_HI_DRAGON */
        case 160+29:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons ancient dragons!", m_name);

            for (k = 0; k < s_num_4; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_HI_DRAGON, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
            }
            if (blind && count)
            {
                msg_print("You hear many powerful things appear nearby.");

            }
            break;
        }

        /* RF6_S_AMBERITES */
        case 160+30:
        {
            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);

            else msg_format("%^s magically summons Lords of Amber!", m_name);



            for (k = 0; k < s_num_4; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_AMBERITE, PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | mode);
            }
            if (blind && count)
            {
                msg_print("You hear immortal beings appear nearby.");

            }
            break;
        }

        /* RF6_S_UNIQUE */
        case 160+31:
        {
            bool uniques_are_summoned = FALSE;
            int non_unique_type = SUMMON_HI_UNDEAD;
            u32b mode = (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE);

            disturb(1, 0);
            if (blind) msg_format("%^s mumbles.", m_name);
            else msg_format("%^s magically summons special opponents!", m_name);

            for (k = 0; k < s_num_4; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, SUMMON_UNIQUE, mode);
            }

            /* If all uniques are down, occasionally bring them back from the grave! */
            if (!count && r_ptr->level > 98 && one_in_(3))
            {
                mode |= PM_ALLOW_CLONED;
                for (k = 0; k < s_num_4; k++)
                {
                    count += summon_specific(m_idx, y, x, rlev, SUMMON_UNIQUE, mode);
                }
            }

            if (count) uniques_are_summoned = TRUE;

            if ((m_ptr->sub_align & (SUB_ALIGN_GOOD | SUB_ALIGN_EVIL)) == (SUB_ALIGN_GOOD | SUB_ALIGN_EVIL))
                non_unique_type = 0;
            else if (m_ptr->sub_align & SUB_ALIGN_GOOD)
                non_unique_type = SUMMON_ANGEL;

            for (k = count; k < s_num_4; k++)
            {
                count += summon_specific(m_idx, y, x, rlev, non_unique_type, mode);
            }

            if (blind && count)
            {
                msg_format("You hear many %s appear nearby.", uniques_are_summoned ? "powerful things" : "things");
            }
            break;
        }
    }
    hack_m_spell = 0;
    if ((p_ptr->action == ACTION_LEARN) && thrown_spell > 175)
    {
        learn_spell(thrown_spell - 96);
    }

    if (seen && maneable && !world_monster && (p_ptr->pclass == CLASS_IMITATOR))
    {
        if (thrown_spell != 167) /* Not RF6_SPECIAL */
        {
            if (p_ptr->mane_num == MAX_MANE)
            {
                int i;
                p_ptr->mane_num--;
                for (i = 0;i < p_ptr->mane_num;i++)
                {
                    p_ptr->mane_spell[i] = p_ptr->mane_spell[i+1];
                    p_ptr->mane_dam[i] = p_ptr->mane_dam[i+1];
                }
            }
            p_ptr->mane_spell[p_ptr->mane_num] = thrown_spell - 96;
            p_ptr->mane_dam[p_ptr->mane_num] = dam;
            p_ptr->mane_num++;
            new_mane = TRUE;

            p_ptr->redraw |= PR_EFFECTS;
        }
    }

    /* Remember what the monster did to us */
    if (can_remember)
    {
        /* Inate spell */
        if (thrown_spell < 32 * 4)
            mon_lore_aux_4(r_ptr, 1 << (thrown_spell - 32 * 3));

        /* Bolt or Ball */
        else if (thrown_spell < 32 * 5)
            mon_lore_aux_5(r_ptr, 1 << (thrown_spell - 32 * 4));

        /* Special spell */
        else if (thrown_spell < 32 * 6)
            mon_lore_aux_6(r_ptr, 1 << (thrown_spell - 32 * 5));
    }


    /* Always take note of monsters that kill you */
    if (p_ptr->is_dead && (r_ptr->r_deaths < MAX_SHORT) && !p_ptr->inside_arena)
    {
        r_ptr->r_deaths++; /* Ignore appearance difference */
    }

    /* A spell was cast */
    return (TRUE);
}
