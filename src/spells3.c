/* File: spells3.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Spell code (part 3) */

#include "angband.h"
#include <assert.h>

/* Maximum number of tries for teleporting */
#define MAX_TRIES 100

/* 1/x chance of reducing stats (for elemental attacks) */
#define HURT_CHANCE 16


static bool cave_monster_teleportable_bold(int m_idx, int y, int x, u32b mode)
{
    monster_type *m_ptr = &m_list[m_idx];
    cave_type    *c_ptr = &cave[y][x];
    feature_type *f_ptr = &f_info[c_ptr->feat];

    /* Require "teleportable" space */
    if (!have_flag(f_ptr->flags, FF_TELEPORTABLE)) return FALSE;

    if (c_ptr->m_idx && (c_ptr->m_idx != m_idx)) return FALSE;
    if (player_bold(y, x)) return FALSE;

    /* Hack -- no teleport onto glyph of warding */
    if (is_glyph_grid(c_ptr)) return FALSE;
    if (is_mon_trap_grid(c_ptr)) return FALSE;

    if (!(mode & TELEPORT_PASSIVE))
    {
        if (!monster_can_cross_terrain(c_ptr->feat, &r_info[m_ptr->r_idx], 0)) return FALSE;
    }

    return TRUE;
}


/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
bool teleport_away(int m_idx, int dis, u32b mode)
{
    int oy, ox, d, i, min;
    int tries = 0;
    int ny = 0, nx = 0;

    bool look = TRUE;

    monster_type *m_ptr = &m_list[m_idx];

    /* Paranoia */
    if (!m_ptr->r_idx) return (FALSE);

    /* Save the old location */
    oy = m_ptr->fy;
    ox = m_ptr->fx;

    /* Minimum distance */
    min = dis / 2;

    if ((mode & TELEPORT_DEC_VALOUR) &&
        (((p_ptr->chp * 10) / p_ptr->mhp) > 5) &&
        (4+randint1(5) < ((p_ptr->chp * 10) / p_ptr->mhp)))
    {
        virtue_add(VIRTUE_VALOUR, -1);
    }

    /* Look until done */
    while (look)
    {
        tries++;

        /* Verify max distance */
        if (dis > 200) dis = 200;

        /* Try several locations */
        for (i = 0; i < 500; i++)
        {
            /* Pick a (possibly illegal) location */
            while (1)
            {
                ny = rand_spread(oy, dis);
                nx = rand_spread(ox, dis);
                d = distance(oy, ox, ny, nx);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if (!in_bounds(ny, nx)) continue;

            if (!cave_monster_teleportable_bold(m_idx, ny, nx, mode)) continue;

            /* No teleporting into vaults and such */
            if (!(p_ptr->inside_quest || p_ptr->inside_arena))
                if (cave[ny][nx].info & CAVE_ICKY) continue;

            /* This grid looks good */
            look = FALSE;

            /* Stop looking */
            break;
        }

        /* Increase the maximum distance */
        dis = dis * 2;

        /* Decrease the minimum distance */
        min = min / 2;

        /* Stop after MAX_TRIES tries */
        if (tries > MAX_TRIES) return (FALSE);
    }

    /* Sound */
    sound(SOUND_TPOTHER);

    /* Update the old location */
    cave[oy][ox].m_idx = 0;

    /* Update the new location */
    cave[ny][nx].m_idx = m_idx;

    /* Move the monster */
    m_ptr->fy = ny;
    m_ptr->fx = nx;

    /* Forget the counter target */
    reset_target(m_ptr);

    /* Update the monster (new location) */
    update_mon(m_idx, TRUE);

    /* Redraw the old grid */
    lite_spot(oy, ox);

    /* Redraw the new grid */
    lite_spot(ny, nx);

    if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
        p_ptr->update |= (PU_MON_LITE);

    return (TRUE);
}



/*
 * Teleport monster next to a grid near the given location
 */
void teleport_monster_to(int m_idx, int ty, int tx, int power, u32b mode)
{
    int ny, nx, oy, ox, d, i, min;
    int attempts = 500;
    int dis = 2;
    bool look = TRUE;
    monster_type *m_ptr = &m_list[m_idx];

    /* Paranoia */
    if (!m_ptr->r_idx) return;

    /* "Skill" test */
    if (randint1(100) > power) return;

    /* Initialize */
    ny = m_ptr->fy;
    nx = m_ptr->fx;

    /* Save the old location */
    oy = m_ptr->fy;
    ox = m_ptr->fx;

    /* Minimum distance */
    min = dis / 2;

    /* Look until done */
    while (look && --attempts)
    {
        /* Verify max distance */
        if (dis > 200) dis = 200;

        /* Try several locations */
        for (i = 0; i < 500; i++)
        {
            /* Pick a (possibly illegal) location */
            while (1)
            {
                ny = rand_spread(ty, dis);
                nx = rand_spread(tx, dis);
                d = distance(ty, tx, ny, nx);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if (!in_bounds(ny, nx)) continue;

            if (!cave_monster_teleportable_bold(m_idx, ny, nx, mode)) continue;

            /* No teleporting into vaults and such */
            /* if (cave[ny][nx].info & (CAVE_ICKY)) continue; */

            /* This grid looks good */
            look = FALSE;

            /* Stop looking */
            break;
        }

        /* Increase the maximum distance */
        dis = dis * 2;

        /* Decrease the minimum distance */
        min = min / 2;
    }

    if (attempts < 1) return;

    /* Sound */
    sound(SOUND_TPOTHER);

    /* Update the old location */
    cave[oy][ox].m_idx = 0;

    /* Update the new location */
    cave[ny][nx].m_idx = m_idx;

    /* Move the monster */
    m_ptr->fy = ny;
    m_ptr->fx = nx;

    /* Update the monster (new location) */
    update_mon(m_idx, TRUE);

    /* Redraw the old grid */
    lite_spot(oy, ox);

    /* Redraw the new grid */
    lite_spot(ny, nx);

    if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
        p_ptr->update |= (PU_MON_LITE);
}


bool cave_player_teleportable_bold(int y, int x, u32b mode)
{
    cave_type    *c_ptr = &cave[y][x];
    feature_type *f_ptr = &f_info[c_ptr->feat];

    /* Require "teleportable" space */
    if (!have_flag(f_ptr->flags, FF_TELEPORTABLE)) return FALSE;

    /* No magical teleporting into vaults and such */
    if (!(mode & TELEPORT_NONMAGICAL) && (c_ptr->info & CAVE_ICKY)) return FALSE;

    if (c_ptr->m_idx && (c_ptr->m_idx != p_ptr->riding)) return FALSE;

    /* don't teleport on a trap. */
    if (have_flag(f_ptr->flags, FF_HIT_TRAP)) return FALSE;

    if (!(mode & TELEPORT_PASSIVE))
    {
        if (!player_can_enter(c_ptr->feat, 0)) return FALSE;

        if (have_flag(f_ptr->flags, FF_WATER) && have_flag(f_ptr->flags, FF_DEEP))
        {
            if (!p_ptr->levitation && !p_ptr->can_swim) return FALSE;
        }

        if (have_flag(f_ptr->flags, FF_LAVA) && res_pct(RES_FIRE) < 100 && !IS_INVULN())
        {
            /* (Almost) always forbid deep lava */
            if (have_flag(f_ptr->flags, FF_DEEP) && !elemental_is_(ELEMENTAL_FIRE)) return FALSE;

            /* Forbid shallow lava when the player don't have levitation */
            if (!p_ptr->levitation) return FALSE;
        }

    }

    return TRUE;
}


/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 *
 * There was a nasty tendency for a long time; which was causing the
 * player to "bounce" between two or three different spots because
 * these are the only spots that are "far enough" way to satisfy the
 * algorithm.
 *
 * But this tendency is now removed; in the new algorithm, a list of
 * candidates is selected first, which includes at least 50% of all
 * floor grids within the distance, and any single grid in this list
 * of candidates has equal possibility to be choosen as a destination.
 */

#define MAX_TELEPORT_DISTANCE 200

bool teleport_player_aux(int dis, u32b mode)
{
    int candidates_at[MAX_TELEPORT_DISTANCE + 1];
    int total_candidates, cur_candidates;
    int y = 0, x = 0, min, pick, i;

    int left = MAX(1, px - dis);
    int right = MIN(cur_wid - 2, px + dis);
    int top = MAX(1, py - dis);
    int bottom = MIN(cur_hgt - 2, py + dis);

    if (p_ptr->wild_mode) return FALSE;

    if (p_ptr->anti_tele && !(mode & TELEPORT_NONMAGICAL))
    {
        msg_print("A mysterious force prevents you from teleporting!");
        equip_learn_flag(OF_NO_TELE);
        return FALSE;
    }

    /* Initialize counters */
    total_candidates = 0;
    for (i = 0; i <= MAX_TELEPORT_DISTANCE; i++)
        candidates_at[i] = 0;

    /* Limit the distance */
    if (dis > MAX_TELEPORT_DISTANCE) dis = MAX_TELEPORT_DISTANCE;

    /* Search valid locations */
    for (y = top; y <= bottom; y++)
    {
        for (x = left; x <= right; x++)
        {
            int d;

            /* Skip illegal locations */
            if (!cave_player_teleportable_bold(y, x, mode)) continue;

            /* Calculate distance */
            d = distance(py, px, y, x);

            /* Skip too far locations */
            if (d > dis) continue;

            /* Strafing ... Maintains LoS with old location */
            if ((mode & TELEPORT_LINE_OF_SIGHT) && !los(y, x, py, px)) continue;

            /* Count the total number of candidates */
            total_candidates++;

            /* Count the number of candidates in this circumference */
            candidates_at[d]++;
        }
    }

    /* No valid location! */
    if (0 == total_candidates) return FALSE;

    /* Fix the minimum distance */
    for (cur_candidates = 0, min = dis; min >= 0; min--)
    {
        cur_candidates += candidates_at[min];

        /* 50% of all candidates will have an equal chance to be choosen. */
        if (cur_candidates && (cur_candidates >= total_candidates / 2)) break;
    }

    /* Pick up a single location randomly */
    pick = randint1(cur_candidates);

    /* Search again the choosen location */
    for (y = top; y <= bottom; y++)
    {
        for (x = left; x <= right; x++)
        {
            int d;

            /* Skip illegal locations */
            if (!cave_player_teleportable_bold(y, x, mode)) continue;

            /* Calculate distance */
            d = distance(py, px, y, x);

            /* Skip too far locations */
            if (d > dis) continue;

            /* Skip too close locations */
            if (d < min) continue;

            /* Strafing ... Maintains LoS with old location */
            if ((mode & TELEPORT_LINE_OF_SIGHT) && !los(y, x, py, px)) continue;

            /* This grid was picked up? */
            pick--;
            if (!pick) break;
        }

        /* Exit the loop */
        if (!pick) break;
    }

    if (player_bold(y, x)) return FALSE;

    /* Sound */
    sound(SOUND_TELEPORT);

    /* Move the player */
    (void)move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);

    return TRUE;
}

void teleport_player(int dis, u32b mode)
{
    int yy, xx;

    /* Save the old location */
    int oy = py;
    int ox = px;

    if (!teleport_player_aux(dis, mode)) return;
    if (!dun_level) return; /* Wilderness scrolling ... */

    /* Monsters with teleport ability may follow the player */
    for (xx = -2; xx < 3; xx++)
    {
        for (yy = -2; yy < 3; yy++)
        {
            int tmp_m_idx;
            if (!in_bounds(oy+yy, ox+xx)) continue;
            tmp_m_idx = cave[oy+yy][ox+xx].m_idx;

            /* A monster except your mount may follow */
            if (tmp_m_idx && (p_ptr->riding != tmp_m_idx))
            {
                monster_type *m_ptr = &m_list[tmp_m_idx];
                monster_race *r_ptr = &r_info[m_ptr->r_idx];

                /* Hack: Duelist Disengage. Marked foe can never follow */
                if ((mode & TELEPORT_DISENGAGE) && tmp_m_idx == p_ptr->duelist_target_idx)
                    continue;

                if (!projectable(oy+yy, ox+xx, oy, ox)) /* Careful: Player has already left!!! */
                    continue;

                /*
                 * The latter limitation is to avoid
                 * totally unkillable suckers...
                 */
                if ((r_ptr->flags6 & RF6_TPORT) &&
                    !(r_ptr->flagsr & RFR_RES_TELE))
                {
                    if (!MON_CSLEEP(m_ptr)) teleport_monster_to(tmp_m_idx, py, px, r_ptr->level, 0L);
                }

                if (m_ptr->r_idx == MON_MONKEY_CLONE)
                    teleport_monster_to(tmp_m_idx, py, px, 10000, 0L);
            }
        }
    }
}


void teleport_player_away(int m_idx, int dis)
{
    int yy, xx;

    /* Save the old location */
    int oy = py;
    int ox = px;

    if (!teleport_player_aux(dis, TELEPORT_PASSIVE)) return;

    /* Monsters with teleport ability may follow the player */
    for (xx = -1; xx < 2; xx++)
    {
        for (yy = -1; yy < 2; yy++)
        {
            int tmp_m_idx = cave[oy+yy][ox+xx].m_idx;

            /* A monster except your mount or caster may follow */
            if (tmp_m_idx && (p_ptr->riding != tmp_m_idx) && (m_idx != tmp_m_idx))
            {
                monster_type *m_ptr = &m_list[tmp_m_idx];
                monster_race *r_ptr = &r_info[m_ptr->r_idx];

                /*
                 * The latter limitation is to avoid
                 * totally unkillable suckers...
                 */
                if ((r_ptr->flags6 & RF6_TPORT) &&
                    !(r_ptr->flagsr & RFR_RES_TELE))
                {
                    if (!MON_CSLEEP(m_ptr)) teleport_monster_to(tmp_m_idx, py, px, r_ptr->level, 0L);
                }
            }
        }
    }
}


/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx, u32b mode)
{
    int y, x, dis = 0, ctr = 0;
    int attempt = 0;
    const int max_attempts = 10 * 1000;

    if (p_ptr->anti_tele && !(mode & TELEPORT_NONMAGICAL))
    {
        msg_print("A mysterious force prevents you from teleporting!");
        equip_learn_flag(OF_NO_TELE);
        return;
    }

    /* Find a usable location */
    while (1)
    {
        /* Pick a nearby legal location */
        while (1)
        {
            y = rand_spread(ny, dis);
            x = rand_spread(nx, dis);
            if (in_bounds(y, x)) break;
        }

        ++attempt;
        if (attempt >= max_attempts)
        {
            msg_print("There is no effect!");
            return;
        }

        /* Accept any grid when wizard mode */
        if (p_ptr->wizard && !(mode & TELEPORT_PASSIVE) && (!cave[y][x].m_idx || (cave[y][x].m_idx == p_ptr->riding))) break;

        /* Accept teleportable floor grids */
        if (cave_player_teleportable_bold(y, x, mode)) break;

        /* Occasionally advance the distance */
        if (++ctr > (4 * dis * dis + 4 * dis + 1))
        {
            ctr = 0;
            dis++;
        }
    }

    /* Sound */
    sound(SOUND_TELEPORT);

    /* Move the player */
    (void)move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
}

static u32b _flag = 0;
static bool _has_flag(object_type *o_ptr) {
    if (!object_is_cursed(o_ptr))
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        return have_flag(flgs, _flag);
    }
    return FALSE;
}
void teleport_away_followable(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    int          oldfy = m_ptr->fy;
    int          oldfx = m_ptr->fx;
    bool         old_ml = m_ptr->ml;
    int          old_cdis = m_ptr->cdis;

    teleport_away(m_idx, MAX_SIGHT * 2 + 5, 0L);

    if (old_ml && (old_cdis <= MAX_SIGHT) && !world_monster && !p_ptr->inside_battle && los(py, px, oldfy, oldfx))
    {
        bool follow = FALSE;

        if (mut_present(MUT_TELEPORT) || (p_ptr->pclass == CLASS_IMITATOR)) follow = TRUE;
        else if (p_ptr->pclass == CLASS_DUELIST
              && p_ptr->duelist_target_idx == m_idx
              && p_ptr->lev >= 30 )
        {
            follow = TRUE;
        }
        else
        {
            _flag = OF_TELEPORT;
            if (equip_find_first(_has_flag))
                follow = TRUE;
        }

        if (follow)
        {
            if (get_check("Do you follow it? "))
            {
                if (one_in_(3))
                {
                    teleport_player(200, TELEPORT_PASSIVE);
                    msg_print("Failed!");
                }
                else
                {
                    if (p_ptr->pclass == CLASS_DUELIST
                     && p_ptr->duelist_target_idx == m_idx
                     && p_ptr->lev >= 30 )
                    {
                        msg_print("You invoke Unending Pursuit ... The duel continues!");
                    }
                    teleport_player_to(m_ptr->fy, m_ptr->fx, 0L);
                }
                p_ptr->energy_need += ENERGY_NEED();
            }
        }
    }
}


/*
 * Teleport the player one level up or down (random when legal)
 * Note: If m_idx <= 0, target is player.
 */
void teleport_level(int m_idx)
{
    bool         go_up;
    char         m_name[160];
    bool         see_m = TRUE;

    if (m_idx <= 0) /* To player */
    {
        strcpy(m_name, "you");
    }
    else /* To monster */
    {
        monster_type *m_ptr = &m_list[m_idx];

        /* Get the monster name (or "it") */
        monster_desc(m_name, m_ptr, 0);

        see_m = is_seen(m_ptr);
    }

    /* No effect in some case */
    if (TELE_LEVEL_IS_INEFF(m_idx))
    {
        if (see_m) msg_print("There is no effect.");

        return;
    }

    if ((m_idx <= 0) && p_ptr->anti_tele) /* To player */
    {
        msg_print("A mysterious force prevents you from teleporting!");
        equip_learn_flag(OF_NO_TELE);
        return;
    }

    /* Choose up or down */
    if (randint0(100) < 50) go_up = TRUE;
    else go_up = FALSE;

    if ((m_idx <= 0) && p_ptr->wizard)
    {
        if (get_check("Force to go up? ")) go_up = TRUE;
        else if (get_check("Force to go down? ")) go_up = FALSE;
    }

    /* Down only */
    if ((ironman_downward && (m_idx <= 0)) || (dun_level <= d_info[dungeon_type].mindepth))
    {
        if (see_m) msg_format("%^s sink%s through the floor.", m_name, (m_idx <= 0) ? "" : "s");
        if (m_idx <= 0) /* To player */
        {
            if (!dun_level)
            {
                dungeon_type = p_ptr->recall_dungeon;
                p_ptr->oldpy = py;
                p_ptr->oldpx = px;
            }

            if (autosave_l) do_cmd_save_game(TRUE);

            if (!dun_level)
            {
                dun_level = d_info[dungeon_type].mindepth;
                prepare_change_floor_mode(CFM_RAND_PLACE);
            }
            else
            {
                prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_DOWN | CFM_RAND_PLACE | CFM_RAND_CONNECT);
            }

            /* Leaving */
            p_ptr->leaving = TRUE;
            p_ptr->leaving_method = LEAVING_TELEPORT_LEVEL;
        }
    }

    /* Up only */
    else if (quest_number(dun_level) || (dun_level >= d_info[dungeon_type].maxdepth))
    {
        if (see_m) msg_format("%^s rise%s up through the ceiling.", m_name, (m_idx <= 0) ? "" : "s");


        if (m_idx <= 0) /* To player */
        {
            if (autosave_l) do_cmd_save_game(TRUE);

            prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_UP | CFM_RAND_PLACE | CFM_RAND_CONNECT);

            leave_quest_check();

            /* Leaving */
            p_ptr->inside_quest = 0;
            p_ptr->leaving = TRUE;
            p_ptr->leaving_method = LEAVING_TELEPORT_LEVEL;
        }
    }
    else if (go_up)
    {
        if (see_m) msg_format("%^s rise%s up through the ceiling.", m_name, (m_idx <= 0) ? "" : "s");


        if (m_idx <= 0) /* To player */
        {
            if (autosave_l) do_cmd_save_game(TRUE);

            prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_UP | CFM_RAND_PLACE | CFM_RAND_CONNECT);

            /* Leaving */
            p_ptr->leaving = TRUE;
            p_ptr->leaving_method = LEAVING_TELEPORT_LEVEL;
        }
    }
    else
    {
        if (see_m) msg_format("%^s sink%s through the floor.", m_name, (m_idx <= 0) ? "" : "s");

        if (m_idx <= 0) /* To player */
        {
            /* Never reach this code on the surface */
            /* if (!dun_level) dungeon_type = p_ptr->recall_dungeon; */

            if (autosave_l) do_cmd_save_game(TRUE);

            prepare_change_floor_mode(CFM_SAVE_FLOORS | CFM_DOWN | CFM_RAND_PLACE | CFM_RAND_CONNECT);

            /* Leaving */
            p_ptr->leaving = TRUE;
            p_ptr->leaving_method = LEAVING_TELEPORT_LEVEL;
        }
    }

    /* Monster level teleportation is simple deleting now */
    if (m_idx > 0)
    {
        monster_type *m_ptr = &m_list[m_idx];

        /* Check for quest completion */
        check_quest_completion(m_ptr);
        delete_monster_idx(m_idx);
    }

    /* Sound */
    sound(SOUND_TPLEVEL);
}



int choose_dungeon(cptr note, int y, int x)
{
    int select_dungeon;
    int i, num = 0;
    s16b *dun;

    /* Hack -- No need to choose dungeon in some case */
    if (no_wilderness || ironman_downward)
    {
        if (max_dlv[DUNGEON_ANGBAND]) return DUNGEON_ANGBAND;
        else
        {
            msg_format("You haven't entered %s yet.", d_name + d_info[DUNGEON_ANGBAND].name);
            msg_print(NULL);
            return 0;
        }
    }

    /* Allocate the "dun" array */
    C_MAKE(dun, max_d_idx, s16b);

    screen_save();
    for(i = 1; i < max_d_idx; i++)
    {
        char buf[80];
        bool seiha = FALSE;

        if (!d_info[i].maxdepth) continue;
        if (d_info[i].flags1 & DF1_RANDOM) continue;
        if (!max_dlv[i]) continue;
        if (d_info[i].final_guardian)
        {
            if (!r_info[d_info[i].final_guardian].max_num) seiha = TRUE;
        }
        else if (max_dlv[i] == d_info[i].maxdepth) seiha = TRUE;

        sprintf(buf," %c) %c%-16s : Max level %d ", 'a'+num, seiha ? '!' : ' ', d_name + d_info[i].name, max_dlv[i]);
        put_str(buf, y + num, x);
        dun[num++] = i;
    }

    if (!num)
    {
        put_str(" No dungeon is available.", y, x);
    }

    prt(format("Which dungeon do you %s?: ", note), 0, 0);
    while(1)
    {
        i = inkey();
        if ((i == ESCAPE) || !num)
        {
            /* Free the "dun" array */
            C_KILL(dun, max_d_idx, s16b);

            screen_load();
            return 0;
        }
        if (i >= 'a' && i <('a'+num))
        {
            select_dungeon = dun[i-'a'];
            break;
        }
        else bell();
    }
    screen_load();

    /* Free the "dun" array */
    C_KILL(dun, max_d_idx, s16b);

    return select_dungeon;
}


/*
 * Recall the player to town or dungeon
 */
bool recall_player(int turns)
{
    /*
     * TODO: Recall the player to the last
     * visited town when in the wilderness
     */

    /* Ironman option */
    if (p_ptr->inside_arena || ironman_downward)
    {
        msg_print("Nothing happens.");

        return TRUE;
    }

    if ( dun_level
      && !(d_info[dungeon_type].flags1 & DF1_RANDOM)
      && !p_ptr->inside_quest
      && !p_ptr->word_recall
      && max_dlv[dungeon_type] > dun_level )
    {
        if (get_check("Reset recall depth? "))
        {
            max_dlv[dungeon_type] = dun_level;
        }

    }
    if (!p_ptr->word_recall)
    {
        if (!dun_level)
        {
            int select_dungeon;
            select_dungeon = choose_dungeon("recall", 1, 1);
            if (!select_dungeon) return FALSE;
            p_ptr->recall_dungeon = select_dungeon;
        }
        p_ptr->word_recall = turns;
        p_ptr->leaving_method = LEAVING_RECALL;

        cmsg_print(TERM_L_BLUE, "The air about you becomes charged...");

        p_ptr->redraw |= (PR_STATUS);
    }
    else
    {
        p_ptr->word_recall = 0;
        cmsg_print(TERM_L_BLUE, "A tension leaves the air around you...");

        p_ptr->leaving_method = LEAVING_UNKOWN;
        p_ptr->redraw |= (PR_STATUS);
    }
    return TRUE;
}


bool word_of_recall(void)
{
    return(recall_player(randint0(21) + 15));
}


bool reset_recall(void)
{
    int select_dungeon, dummy = 0;
    char ppp[80];
    char tmp_val[160];

    select_dungeon = choose_dungeon("reset", 1, 1);

    /* Ironman option */
    if (ironman_downward)
    {
        msg_print("Nothing happens.");

        return TRUE;
    }

    if (!select_dungeon) return FALSE;
    /* Prompt */
    sprintf(ppp, "Reset to which level (%d-%d): ", d_info[select_dungeon].mindepth, max_dlv[select_dungeon]);


    /* Default */
    sprintf(tmp_val, "%d", MAX(dun_level, 1));

    /* Ask for a level */
    if (get_string(ppp, tmp_val, 10))
    {
        /* Extract request */
        dummy = atoi(tmp_val);

        /* Paranoia */
        if (dummy < 1) dummy = 1;

        /* Paranoia */
        if (dummy > max_dlv[select_dungeon]) dummy = max_dlv[select_dungeon];
        if (dummy < d_info[select_dungeon].mindepth) dummy = d_info[select_dungeon].mindepth;

        max_dlv[select_dungeon] = dummy;

        msg_format("Recall depth set to level %d (%d').", dummy, dummy * 50);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}


/*
 * Apply disenchantment to the player's stuff
 *
 * XXX XXX XXX This function is also called from the "melee" code
 *
 * Return "TRUE" if the player notices anything
 */
bool apply_disenchant(int mode)
{
    int slot = equip_random_slot(object_is_weapon_armour_ammo);

    if (slot)
    {
        object_type     *o_ptr = equip_obj(slot);
        int             t = 0;
        char            o_name[MAX_NLEN];
        u32b            flgs[OF_ARRAY_SIZE];
        int to_h, to_d, to_a, pval;

        if (o_ptr->to_h <= 0 && o_ptr->to_d <= 0 && o_ptr->to_a <= 0 && o_ptr->pval <= 1)
            return FALSE;

        object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

        if (object_is_artifact(o_ptr) && (randint0(100) < 71))
        {
            msg_format("Your %s (%c) resists disenchantment!", o_name, index_to_label(t));
            return TRUE;
        }

        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_RES_DISEN))
        {
            msg_format("Your %s (%c) resists disenchantment!", o_name, index_to_label(t));
            obj_learn_flag(o_ptr, OF_RES_DISEN);
            return TRUE;
        }

        /* Memorize old value */
        to_h = o_ptr->to_h;
        to_d = o_ptr->to_d;
        to_a = o_ptr->to_a;
        pval = o_ptr->pval;

        /* Disenchant tohit */
        if (o_ptr->to_h > 0) o_ptr->to_h--;
        if ((o_ptr->to_h > 5) && (randint0(100) < 20)) o_ptr->to_h--;

        /* Disenchant todam */
        if (o_ptr->to_d > 0) o_ptr->to_d--;
        if ((o_ptr->to_d > 5) && (randint0(100) < 20)) o_ptr->to_d--;

        /* Disenchant toac */
        if (o_ptr->to_a > 0) o_ptr->to_a--;
        if ((o_ptr->to_a > 5) && (randint0(100) < 20)) o_ptr->to_a--;

        /* Disenchant pval (occasionally) */
        /* Unless called from wild_magic() */
        if ((o_ptr->pval > 1) && one_in_(13) && !(mode & 0x01)) o_ptr->pval--;

        if ((to_h != o_ptr->to_h) || (to_d != o_ptr->to_d) ||
            (to_a != o_ptr->to_a) || (pval != o_ptr->pval))
        {
            msg_format("Your %s (%c) was disenchanted!", o_name, index_to_label(t));
            virtue_add(VIRTUE_HARMONY, 1);
            virtue_add(VIRTUE_ENCHANTMENT, -2);

            p_ptr->update |= (PU_BONUS);
            p_ptr->window |= (PW_EQUIP);
            android_calc_exp();
        }
        return TRUE;
    }
    return FALSE;
}


void mutate_player(void)
{
    int max1, cur1, max2, cur2, ii, jj, i;

    /* Pick a pair of stats */
    ii = randint0(6);
    for (jj = ii; jj == ii; jj = randint0(6)) /* loop */;

    max1 = p_ptr->stat_max[ii];
    cur1 = p_ptr->stat_cur[ii];
    max2 = p_ptr->stat_max[jj];
    cur2 = p_ptr->stat_cur[jj];

    p_ptr->stat_max[ii] = max2;
    p_ptr->stat_cur[ii] = cur2;
    p_ptr->stat_max[jj] = max1;
    p_ptr->stat_cur[jj] = cur1;

    for (i=0;i<6;i++)
    {
        if(p_ptr->stat_max[i] > p_ptr->stat_max_max[i]) p_ptr->stat_max[i] = p_ptr->stat_max_max[i];
        if(p_ptr->stat_cur[i] > p_ptr->stat_max_max[i]) p_ptr->stat_cur[i] = p_ptr->stat_max_max[i];
    }

    p_ptr->update |= (PU_BONUS);
}


/*
 * Apply Nexus
 */
void apply_nexus(monster_type *m_ptr)
{
    switch (randint1(7))
    {
        case 1: case 2: case 3:
        {
            teleport_player(200, TELEPORT_PASSIVE);
            break;
        }

        case 4: case 5:
        {
            teleport_player_to(m_ptr->fy, m_ptr->fx, TELEPORT_PASSIVE);
            break;
        }

        case 6:
        {
            if (randint0(100) < p_ptr->skills.sav)
            {
                msg_print("You resist the effects!");

                break;
            }

            /* Teleport Level */
            teleport_level(0);
            break;
        }

        case 7:
        {
			msg_print("You shudder for a moment!");
			/*
			if (randint0(100) < p_ptr->skills.sav || (p_ptr->resist[RES_NEXUS])) // automatically resist if you have nexus-resistance.
            {
                msg_print("You resist the effects!");
                break;
            }

            msg_print("Your body starts to scramble...");
            if (p_ptr->pclass == CLASS_WILD_TALENT)
                wild_talent_scramble();
            else
                mutate_player();
            break;**/
			break;
        }
    }
}


/*
 * Charge a lite (torch or latern)
 */
void phlogiston(void)
{
    int slot = equip_find_object(TV_LITE, SV_ANY);
    if (slot)
    {
        int max_flog = 0;
        object_type *o_ptr = equip_obj(slot);

        if (o_ptr->sval == SV_LITE_LANTERN)
            max_flog = FUEL_LAMP;
        else if (o_ptr->sval == SV_LITE_TORCH)
            max_flog = FUEL_TORCH;

        if (o_ptr->xtra4 >= max_flog || !max_flog)
        {
            msg_print("No more phlogiston can be put in this item.");
            return;
        }

        o_ptr->xtra4 += (max_flog / 2);
        msg_print("You add phlogiston to your light item.");
        if (o_ptr->xtra4 >= max_flog)
        {
            o_ptr->xtra4 = max_flog;
            msg_print("Your light item is full.");
        }

        p_ptr->update |= PU_TORCH;
    }
    else
        msg_print("Nothing happens.");
}


/*
 * Brand the current weapon
 */
bool brand_weapon(int brand_type)
{
    bool        result = FALSE;
    int         item;
    cptr        q, s;

    item_tester_hook = object_allow_enchant_melee_weapon;
    item_tester_no_ryoute = TRUE;

    q = "Enchant which weapon? ";
    s = "You have nothing to enchant.";
    if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

    if (inventory[item].name1 || inventory[item].name2)
    {
    }
    else if (have_flag(inventory[item].flags, OF_NO_REMOVE))
    {
        msg_print("You are already excellent!");
    }
    else if (brand_type == -1)
    {
        result = brand_weapon_aux(item);
    }
    else
    {
        ego_brand_weapon(&inventory[item], brand_type);
        result = TRUE;
    }
    if (result)
    {
        enchant(&inventory[item], randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);
        inventory[item].discount = 99;

        virtue_add(VIRTUE_ENCHANTMENT, 2);
        obj_identify_fully(&inventory[item]);
        obj_display(&inventory[item]);
    }
    else
    {
        if (flush_failure) flush();
        msg_print("The Branding failed.");
        virtue_add(VIRTUE_ENCHANTMENT, -2);
    }
    android_calc_exp();
    return TRUE;
}
/* Hack for old branding spells attempting to make now non-existent ego types! */
bool brand_weapon_slaying(int brand_flag, int res_flag)
{
    bool        result = FALSE;
    int         item;
    cptr        q, s;

    item_tester_hook = object_allow_enchant_melee_weapon;
    item_tester_no_ryoute = TRUE;

    q = "Enchant which weapon? ";
    s = "You have nothing to enchant.";
    if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

    if (inventory[item].name1 || inventory[item].name2)
    {
    }
    else if (have_flag(inventory[item].flags, OF_NO_REMOVE))
    {
        msg_print("You are already excellent!");
    }
    else
    {
        inventory[item].name2 = EGO_WEAPON_SLAYING;
        add_flag(inventory[item].flags, brand_flag);
        if (res_flag != OF_INVALID)
            add_flag(inventory[item].flags, res_flag);
        result = TRUE;
    }
    if (result)
    {
        enchant(&inventory[item], randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);
        inventory[item].discount = 99;

        virtue_add(VIRTUE_ENCHANTMENT, 2);
        obj_identify_fully(&inventory[item]);
        obj_display(&inventory[item]);
    }
    else
    {
        if (flush_failure) flush();
        msg_print("The Branding failed.");
        virtue_add(VIRTUE_ENCHANTMENT, -2);
    }
    android_calc_exp();
    return TRUE;
}
bool brand_weapon_aux(int item)
{
    assert(item >= 0);
    if (have_flag(inventory[item].flags, OF_NO_REMOVE))
        return FALSE;
    apply_magic(&inventory[item], dun_level, AM_GOOD | AM_GREAT | AM_NO_FIXED_ART | AM_CRAFTING);
    return TRUE;
}
bool brand_armour_aux(int item)
{
    assert(item >= 0);
    if (have_flag(inventory[item].flags, OF_NO_REMOVE))
        return FALSE;
    apply_magic(&inventory[item], dun_level, AM_GOOD | AM_GREAT | AM_NO_FIXED_ART | AM_CRAFTING);
    return TRUE;
}

/*
 * Vanish all walls in this floor
 */
static bool vanish_dungeon(void)
{
    int          y, x;
    cave_type    *c_ptr;
    feature_type *f_ptr;
    monster_type *m_ptr;
    char         m_name[80];

    /* Prevent vasishing of quest levels and town */
    if ((p_ptr->inside_quest && is_fixed_quest_idx(p_ptr->inside_quest)) || !dun_level)
    {
        return FALSE;
    }

    /* Scan all normal grids */
    for (y = 1; y < cur_hgt - 1; y++)
    {
        for (x = 1; x < cur_wid - 1; x++)
        {
            c_ptr = &cave[y][x];

            /* Seeing true feature code (ignore mimic) */
            f_ptr = &f_info[c_ptr->feat];

            /* Lose room and vault */
            c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

            m_ptr = &m_list[c_ptr->m_idx];

            /* Awake monster */
            if (c_ptr->m_idx && MON_CSLEEP(m_ptr))
            {
                /* Reset sleep counter */
                (void)set_monster_csleep(c_ptr->m_idx, 0);

                /* Notice the "waking up" */
                if (m_ptr->ml)
                {
                    /* Acquire the monster name */
                    monster_desc(m_name, m_ptr, 0);

                    /* Dump a message */
                    msg_format("%^s wakes up.", m_name);
                }
            }

            /* Process all walls, doors and patterns */
            if (have_flag(f_ptr->flags, FF_HURT_DISI)) cave_alter_feat(y, x, FF_HURT_DISI);
        }
    }

    /* Special boundary walls -- Top and bottom */
    for (x = 0; x < cur_wid; x++)
    {
        c_ptr = &cave[0][x];
        f_ptr = &f_info[c_ptr->mimic];

        /* Lose room and vault */
        c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

        /* Set boundary mimic if needed */
        if (c_ptr->mimic && have_flag(f_ptr->flags, FF_HURT_DISI))
        {
            c_ptr->mimic = feat_state(c_ptr->mimic, FF_HURT_DISI);

            /* Check for change to boring grid */
            if (!have_flag(f_info[c_ptr->mimic].flags, FF_REMEMBER)) c_ptr->info &= ~(CAVE_MARK);
        }

        c_ptr = &cave[cur_hgt - 1][x];
        f_ptr = &f_info[c_ptr->mimic];

        /* Lose room and vault */
        c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

        /* Set boundary mimic if needed */
        if (c_ptr->mimic && have_flag(f_ptr->flags, FF_HURT_DISI))
        {
            c_ptr->mimic = feat_state(c_ptr->mimic, FF_HURT_DISI);

            /* Check for change to boring grid */
            if (!have_flag(f_info[c_ptr->mimic].flags, FF_REMEMBER)) c_ptr->info &= ~(CAVE_MARK);
        }
    }

    /* Special boundary walls -- Left and right */
    for (y = 1; y < (cur_hgt - 1); y++)
    {
        c_ptr = &cave[y][0];
        f_ptr = &f_info[c_ptr->mimic];

        /* Lose room and vault */
        c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

        /* Set boundary mimic if needed */
        if (c_ptr->mimic && have_flag(f_ptr->flags, FF_HURT_DISI))
        {
            c_ptr->mimic = feat_state(c_ptr->mimic, FF_HURT_DISI);

            /* Check for change to boring grid */
            if (!have_flag(f_info[c_ptr->mimic].flags, FF_REMEMBER)) c_ptr->info &= ~(CAVE_MARK);
        }

        c_ptr = &cave[y][cur_wid - 1];
        f_ptr = &f_info[c_ptr->mimic];

        /* Lose room and vault */
        c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

        /* Set boundary mimic if needed */
        if (c_ptr->mimic && have_flag(f_ptr->flags, FF_HURT_DISI))
        {
            c_ptr->mimic = feat_state(c_ptr->mimic, FF_HURT_DISI);

            /* Check for change to boring grid */
            if (!have_flag(f_info[c_ptr->mimic].flags, FF_REMEMBER)) c_ptr->info &= ~(CAVE_MARK);
        }
    }

    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

    return TRUE;
}


void call_the_(void)
{
    int i;
    cave_type *c_ptr;
    bool do_call = TRUE;

    for (i = 0; i < 9; i++)
    {
        c_ptr = &cave[py + ddy_ddd[i]][px + ddx_ddd[i]];

        if (!cave_have_flag_grid(c_ptr, FF_PROJECT))
        {
            if (!c_ptr->mimic || !have_flag(f_info[c_ptr->mimic].flags, FF_PROJECT) ||
                !permanent_wall(&f_info[c_ptr->feat]))
            {
                do_call = FALSE;
                break;
            }
        }
    }

    if (do_call)
    {
        for (i = 1; i < 10; i++)
        {
            if (i - 5) fire_ball(GF_ROCKET, i, 175, 2);
        }

        for (i = 1; i < 10; i++)
        {
            if (i - 5) fire_ball(GF_MANA, i, 175, 3);
        }

        for (i = 1; i < 10; i++)
        {
            if (i - 5) fire_ball(GF_NUKE, i, 175, 4);
        }
    }

    /* Prevent destruction of quest levels and town */
    else if ((p_ptr->inside_quest && is_fixed_quest_idx(p_ptr->inside_quest)) || !dun_level)
    {
        msg_print("The ground trembles.");
    }

    else
    {
        msg_format("You %s the %s too close to a wall!",
            ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "recite" : "cast"),
            ((mp_ptr->spell_book == TV_LIFE_BOOK) ? "prayer" : "spell"));
        msg_print("There is a loud explosion!");

        if (one_in_(666))
        {
            if (!vanish_dungeon()) msg_print("The dungeon silences a moment.");
        }
        else
        {
            if (destroy_area(py, px, 15 + p_ptr->lev + randint0(11), 8 * p_ptr->lev))
                msg_print("The dungeon collapses...");

            else
                msg_print("The dungeon trembles.");
        }

        take_hit(DAMAGE_NOESCAPE, 100 + randint1(150), "a suicidal Call the Void", -1);
    }
}


/*
 * Fetch an item (teleport it right underneath the caster)
 */
void fetch(int dir, int wgt, bool require_los)
{
    int             ty, tx, i;
    cave_type       *c_ptr;
    object_type     *o_ptr;
    char            o_name[MAX_NLEN];

    /* Check to see if an object is already there */
    if (cave[py][px].o_idx)
    {
        msg_print("You can't fetch when you're already standing on something.");

        return;
    }

    /* Use a target */
    if (dir == 5 && target_okay())
    {
        tx = target_col;
        ty = target_row;

        if (distance(py, px, ty, tx) > MAX_RANGE)
        {
            msg_print("You can't fetch something that far away!");

            return;
        }

        c_ptr = &cave[ty][tx];

        /* We need an item to fetch */
        if (!c_ptr->o_idx)
        {
            msg_print("There is no object at this place.");

            return;
        }

        /* No fetching from vault */
        if (c_ptr->info & CAVE_ICKY)
        {
            msg_print("The item slips from your control.");

            return;
        }

        /* We need to see the item */
        if (require_los)
        {
            if (!player_has_los_bold(ty, tx))
            {
                msg_print("You have no direct line of sight to that location.");

                return;
            }
            else if (!projectable(py, px, ty, tx))
            {
                msg_print("You have no direct line of sight to that location.");

                return;
            }
        }
    }
    else
    {
        /* Use a direction */
        ty = py; /* Where to drop the item */
        tx = px;

        do
        {
            ty += ddy[dir];
            tx += ddx[dir];
            c_ptr = &cave[ty][tx];

            if ((distance(py, px, ty, tx) > MAX_RANGE) ||
                !cave_have_flag_bold(ty, tx, FF_PROJECT)) return;
        }
        while (!c_ptr->o_idx);
    }

    o_ptr = &o_list[c_ptr->o_idx];

    if (o_ptr->weight > wgt)
    {
        /* Too heavy to 'fetch' */
        msg_print("The object is too heavy.");

        return;
    }

    i = c_ptr->o_idx;
    c_ptr->o_idx = o_ptr->next_o_idx;
    cave[py][px].o_idx = i; /* 'move' it */
    o_ptr->next_o_idx = 0;
    o_ptr->iy = (byte)py;
    o_ptr->ix = (byte)px;

    object_desc(o_name, o_ptr, OD_NAME_ONLY);
    msg_format("%^s flies through the air to your feet.", o_name);


    note_spot(py, px);
    p_ptr->redraw |= PR_MAP;
}


void alter_reality(void)
{
    /* Ironman option */
    if (p_ptr->inside_arena || ironman_downward)
    {
        msg_print("Nothing happens.");
        return;
    }

    if (!p_ptr->alter_reality)
    {
        int turns = randint0(21) + 15;

        p_ptr->alter_reality = turns;
        msg_print("The view around you begins to change...");

        p_ptr->leaving_method = LEAVING_ALTER_REALITY;
        p_ptr->redraw |= (PR_STATUS);
    }
    else
    {
        p_ptr->alter_reality = 0;
        msg_print("The view around you got back...");

        p_ptr->leaving_method = LEAVING_UNKOWN;
        p_ptr->redraw |= (PR_STATUS);
    }
    return;
}


/*
 * Leave a "glyph of warding" which prevents monster movement
 */
bool warding_glyph(void)
{
    /* XXX XXX XXX */
    if (!cave_clean_bold(py, px))
    {
        msg_print("The object resists the spell.");

        return FALSE;
    }

    /* Create a glyph */
    cave[py][px].info |= CAVE_OBJECT;
    cave[py][px].mimic = feat_glyph;

    /* Notice */
    note_spot(py, px);

    /* Redraw */
    lite_spot(py, px);

    return TRUE;
}


bool set_trap(int y, int x, int feature)
{
    if (!in_bounds(y, x)) return FALSE;

    if (!cave_clean_bold(y, x))
    {
        if (y == py && x == px) /* Hack: I only want the message sometimes ... */
            msg_print("The object resists the spell.");
        return FALSE;
    }

    cave[y][x].info |= CAVE_OBJECT;
    cave[y][x].mimic = feature;
    note_spot(y, x);
    lite_spot(y, x);

    return TRUE;
}

bool explosive_rune(void)
{
    return set_trap(py, px, feat_explosive_rune);
}


/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(void)
{
    int i;

    /* Simply identify and know every item */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        object_type *o_ptr = &inventory[i];

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Identify it */
        identify_item(o_ptr);

        /* Auto-inscription */
        autopick_alter_item(i, FALSE);
    }
}


/*
 * Used by the "enchant" function (chance of failure)
 * (modified for Zangband, we need better stuff there...) -- TY
 */
static int enchant_table[16] =
{
      5,  10,  50, 100, 200,
    300, 400, 500, 650, 800,
    950, 987, 993, 995, 998,
    1000
};


/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */

static int remove_curse_aux(int all)
{
    int slot;
    int ct = 0;

    for (slot = equip_find_first(object_is_cursed);
            slot;
            slot = equip_find_next(object_is_cursed, slot))
    {
        object_type *o_ptr = equip_obj(slot);

        if (!all && (o_ptr->curse_flags & OFC_HEAVY_CURSE)) continue;

        if (o_ptr->curse_flags & OFC_PERMA_CURSE)
        {
            o_ptr->curse_flags &= (OFC_CURSED | OFC_HEAVY_CURSE | OFC_PERMA_CURSE);
            o_ptr->known_curse_flags &= (OFC_CURSED | OFC_HEAVY_CURSE | OFC_PERMA_CURSE);
            continue;
        }

        o_ptr->curse_flags = 0;
        o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
        o_ptr->ident  |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        ct++;
    }

    return ct;
}


/*
 * Remove most curses
 */
bool remove_curse(void)
{
    return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
    return (remove_curse_aux(TRUE));
}

/*
 * Turns an object into gold, gain some of its value in a shop
 */
bool alchemy(void)
{
    int item, amt = 1;
    int old_number;
    int price;
    bool force = FALSE;
    object_type *o_ptr;
    char o_name[MAX_NLEN];
    char out_val[MAX_NLEN+40];

    cptr q, s;

    /* Hack -- force destruction */
    if (command_arg > 0) force = TRUE;

    /* Get an item */
    q = "Turn which item to gold? ";
    s = "You have nothing to turn to gold.";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return (FALSE);

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


    /* See how many items */
    if (o_ptr->number > 1)
    {
        /* Get a quantity */
        amt = get_quantity(NULL, o_ptr->number);

        /* Allow user abort */
        if (amt <= 0) return FALSE;
    }


    /* Describe the object */
    old_number = o_ptr->number;
    o_ptr->number = amt;
    object_desc(o_name, o_ptr, 0);
    o_ptr->number = old_number;

    /* Verify unless quantity given */
    if (!force)
    {
        if (confirm_destroy || (obj_value(o_ptr) > 0))
        {
            /* Make a verification */
            sprintf(out_val, "Really turn %s to gold? ", o_name);

            if (!get_check(out_val)) return FALSE;
        }
    }

    /* Artifacts cannot be destroyed */
    if (!can_player_destroy_object(o_ptr))
    {
        /* Message */
        msg_format("You fail to turn %s to gold!", o_name);

        /* Done */
        return FALSE;
    }

    price = obj_value_real(o_ptr);

    if (price <= 0)
    {
        msg_format("You turn %s to fool's gold.", o_name);
    }
    else
    {
        price /= 3;

        if (amt > 1) price *= amt;

        if (price > 30000) price = 30000;
        msg_format("You turn %s to %d coins worth of gold.", o_name, price);

        p_ptr->au += price;
        stats_on_gold_selling(price); /* ? */

        /* Redraw gold */
        p_ptr->redraw |= (PR_GOLD);

        if (prace_is_(RACE_MON_LEPRECHAUN))
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    }

    /* Eliminate the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -amt);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Eliminate the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -amt);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }

    return TRUE;
}


/*
 * Break the curse of an item
 */
static void break_curse(object_type *o_ptr)
{
    if (object_is_cursed(o_ptr) && !(o_ptr->curse_flags & OFC_PERMA_CURSE) && !(o_ptr->curse_flags & OFC_HEAVY_CURSE) && (randint0(100) < 25))
    {
        msg_print("The curse is broken!");

        o_ptr->curse_flags = 0L;

        o_ptr->ident |= (IDENT_SENSE);

        o_ptr->feeling = FEEL_NONE;
    }
}


/*
 * Enchants a plus onto an item. -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting. Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item. -CFT-
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time. Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(object_type *o_ptr, int n, int eflag)
{
    int     i, chance, prob;
    bool    res = FALSE;
    bool    a = object_is_artifact(o_ptr);
    bool    force = (eflag & ENCH_FORCE);
    int     minor_limit = 2 + p_ptr->lev/5; /* This matches the town service ... */
    u32b    flgs[OF_ARRAY_SIZE];


    /* Large piles resist enchantment */
    prob = o_ptr->number * 100;

    /* Some objects cannot be enchanted */
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_NO_ENCHANT))
        return FALSE;


    /* Missiles are easy to enchant */
    if ((o_ptr->tval == TV_BOLT) ||
        (o_ptr->tval == TV_ARROW) ||
        (o_ptr->tval == TV_SHOT))
    {
        prob = prob / 20;
    }

    /* Try "n" times */
    for (i = 0; i < n; i++)
    {
        /* Hack -- Roll for pile resistance */
        if (!force && randint0(prob) >= 100) continue;

        /* Enchant to hit */
        if (eflag & ENCH_TOHIT)
        {
            int idx = o_ptr->to_h;
            if (eflag & ENCH_PSI_HACK)
            {
                idx -= 2*(psion_enchant_power() - 1);
            }

            if (idx < 0) chance = 0;
            else if (idx > 15) chance = 1000;
            else chance = enchant_table[idx];

            if ((eflag & ENCH_MINOR_HACK) && idx >= minor_limit)
                chance = 1000;

            if (force || ((randint1(1000) > chance) && (!a || (randint0(100) < 50))))
            {
                o_ptr->to_h++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (o_ptr->to_h >= 0)
                    break_curse(o_ptr);
            }
        }

        /* Enchant to damage */
        if (eflag & ENCH_TODAM)
        {
            int idx = o_ptr->to_d;
            if (eflag & ENCH_PSI_HACK)
            {
                idx -= 2*(psion_enchant_power() - 1);
            }

            if (idx < 0) chance = 0;
            else if (idx > 15) chance = 1000;
            else chance = enchant_table[idx];

            if ((eflag & ENCH_MINOR_HACK) && idx >= minor_limit)
                chance = 1000;

            if (force || ((randint1(1000) > chance) && (!a || (randint0(100) < 50))))
            {
                o_ptr->to_d++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (o_ptr->to_d >= 0)
                    break_curse(o_ptr);
            }
        }

        /* Enchant to armor class */
        if (eflag & ENCH_TOAC)
        {
            int idx = o_ptr->to_a;

            if (eflag & ENCH_PSI_HACK)
            {
                idx -= 2*(psion_enchant_power() - 1);
            }

            if (idx < 0) chance = 0;
            else if (idx > 15) chance = 1000;
            else chance = enchant_table[idx];

            if ((eflag & ENCH_MINOR_HACK) && idx >= minor_limit)
                chance = 1000;

            if (force || ((randint1(1000) > chance) && (!a || (randint0(100) < 50))))
            {
                o_ptr->to_a++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (o_ptr->to_a >= 0)
                    break_curse(o_ptr);
            }
        }
    }

    /* Failure */
    if (!res) return (FALSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN | PW_EQUIP);

    android_calc_exp();

    /* Success */
    return (TRUE);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
    int         item;
    bool        okay = FALSE;
    object_type *o_ptr;
    char        o_name[MAX_NLEN];
    cptr        q, s;


    /* Assume enchant weapon */
    item_tester_hook = object_allow_enchant_weapon;
    item_tester_no_ryoute = TRUE;

    /* Enchant armor if requested */
    if (num_ac) item_tester_hook = object_is_armour;

    /* Get an item */
    q = "Enchant which item? ";
    s = "You have nothing to enchant.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

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


    /* Description */
    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Describe */
    msg_format("%s %s glow%s brightly!",
           ((item >= 0) ? "Your" : "The"), o_name,
           ((o_ptr->number > 1) ? "" : "s"));


    /* Enchant */
    if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
    if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
    if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

    /* Failure */
    if (!okay)
    {
        /* Flush */
        if (flush_failure) flush();

        /* Message */
        msg_print("The enchantment failed.");

        if (one_in_(3) && virtue_current(VIRTUE_ENCHANTMENT) < 100)
            virtue_add(VIRTUE_ENCHANTMENT, -1);
    }
    else
        virtue_add(VIRTUE_ENCHANTMENT, 1);

    android_calc_exp();

    /* Something happened */
    return (TRUE);
}


/*
 * Check if an object is nameless weapon or armour
 */
bool item_tester_hook_nameless_weapon_armour(object_type *o_ptr)
{
    if ( !object_is_weapon_armour_ammo(o_ptr)
      && !(o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_FEANOR)
      && !(o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET) /* Testing ... */
      && !(prace_is_(RACE_SNOTLING) && object_is_mushroom(o_ptr)) )
    {
        return FALSE;
    }

    /* Require nameless object if the object is well known */
    if (object_is_known(o_ptr) && !object_is_nameless(o_ptr))
        return FALSE;

    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_RUNESWORD) return FALSE;
    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_POISON_NEEDLE) return FALSE;

    return TRUE;
}

bool artifact_scroll(void)
{
    int             item;
    bool            okay = FALSE;
    object_type     *o_ptr;
    char            o_name[MAX_NLEN];
    cptr            q, s;


    item_tester_no_ryoute = TRUE;

    /* Enchant weapon/armour */
    item_tester_hook = item_tester_hook_nameless_weapon_armour;

    /* Get an item */
    q = "Enchant which item? ";
    s = "You have nothing to enchant.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

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

    /* Description */
    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Describe */
    msg_format("%s %s radiate%s a blinding light!",
          ((item >= 0) ? "Your" : "The"), o_name,
          ((o_ptr->number > 1) ? "" : "s"));

    if (object_is_artifact(o_ptr))
    {
        msg_format("The %s %s already %s!",
            o_name, ((o_ptr->number > 1) ? "are" : "is"),
            ((o_ptr->number > 1) ? "artifacts" : "an artifact"));

        okay = FALSE;
    }

    else if (object_is_ego(o_ptr))
    {
        msg_format("The %s %s already %s!",
            o_name, ((o_ptr->number > 1) ? "are" : "is"),
            ((o_ptr->number > 1) ? "ego items" : "an ego item"));

        okay = FALSE;
    }

    else if (o_ptr->xtra3)
    {
        msg_format("The %s %s already %s!",
            o_name, ((o_ptr->number > 1) ? "are" : "is"),
            ((o_ptr->number > 1) ? "customized items" : "a customized item"));
    }

    else if (have_flag(o_ptr->flags, OF_NO_REMOVE))
    {
        msg_print("You are quite special already!");
        okay = FALSE;
    }

    else
    {
        if (o_ptr->number > 1)
        {
            msg_print("Not enough enough energy to enchant more than one object!");
            msg_format("%d of your %s %s destroyed!",(o_ptr->number)-1, o_name, (o_ptr->number>2?"were":"was"));

            if (item >= 0)
            {
                inven_item_increase(item, 1-(o_ptr->number));
            }
            else
            {
                floor_item_increase(0-item, 1-(o_ptr->number));
            }
        }
        if (object_is_mushroom(o_ptr)) /* Hack for Snotlings ... */
        {
            o_ptr->art_name = quark_add("(Eternal Mushroom)");
            okay = TRUE;
        }
        else
        {
            okay = create_artifact(o_ptr, CREATE_ART_SCROLL | CREATE_ART_GOOD);
        }
    }

    /* Failure */
    if (!okay)
    {
        /* Flush */
        if (flush_failure) flush();

        /* Message */
        msg_print("The enchantment failed.");

        if (one_in_(3)) virtue_add(VIRTUE_ENCHANTMENT, -1);
    }
    else
        virtue_add(VIRTUE_ENCHANTMENT, 1);

    android_calc_exp();

    /* Something happened */
    return (TRUE);
}


/*
 * Identify an object
 */
bool identify_item(object_type *o_ptr)
{
    bool old_known = FALSE;

    if (obj_is_identified(o_ptr))
        old_known = TRUE;

    if (!spoiler_hack && !obj_is_identified_fully(o_ptr))
    {
        if (object_is_artifact(o_ptr) || one_in_(5))
            virtue_add(VIRTUE_KNOWLEDGE, 1);
    }

    obj_identify(o_ptr);
    stats_on_identify(o_ptr);

    /* Player touches it */
    o_ptr->marked |= OM_TOUCHED;

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN | PW_EQUIP);

    return old_known;
}


/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
static object_p _hack_obj_p = NULL;
static bool item_tester_hook_identify(object_type *o_ptr)
{
    if ( !object_is_known(o_ptr)
      && (!_hack_obj_p || _hack_obj_p(o_ptr)) )
    {
        return TRUE;
    }
    return FALSE;
}
bool ident_spell(object_p p)
{
    int             item;
    object_type     *o_ptr;
    char            o_name[MAX_NLEN];
    cptr            q, s;
    bool old_known;

    item_tester_no_ryoute = TRUE;
    _hack_obj_p = p;
    item_tester_hook = item_tester_hook_identify;

    if (can_get_item())
    {
        q = "Identify which item? ";
    }
    else
    {
        item_tester_hook = p;
        q = "All items are identified. ";
    }

    s = "You have nothing to identify.";
    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);
    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];

    old_known = identify_item(o_ptr);

    object_desc(o_name, o_ptr, 0);

    if (equip_is_valid_slot(item))
        msg_format("%^s: %s (%c).", equip_describe_slot(item), o_name, index_to_label(item));
    else if (item >= 0)
        msg_format("In your pack: %s (%c).", o_name, index_to_label(item));
    else
        msg_format("On the ground: %s.", o_name);

    autopick_alter_item(item, (bool)(destroy_identify && !old_known));
    return TRUE;
}


/*
 * Mundanify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was mundanified, else FALSE.
 */
bool mundane_spell(bool only_equip)
{
    int             item;
    object_type     *o_ptr;
    cptr            q, s;

    if (only_equip) item_tester_hook = object_is_weapon_armour_ammo;
    item_tester_no_ryoute = TRUE;

    /* Get an item */
    q = "Use which item? ";
    s = "You have nothing you can use.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);

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

    if (o_ptr->name1 == ART_HAND_OF_VECNA || o_ptr->name1 == ART_EYE_OF_VECNA)
    {
        msg_print("There is no effect.");
        return TRUE;
    }

    if (have_flag(o_ptr->flags, OF_NO_REMOVE))
    {
        msg_print("Failed! You will never be average!");
        return TRUE;
    }

    /* Oops */
    msg_print("There is a bright flash of light!");
    {
        byte iy = o_ptr->iy;                 /* Y-position on map, or zero */
        byte ix = o_ptr->ix;                 /* X-position on map, or zero */
        s16b next_o_idx = o_ptr->next_o_idx; /* Next object in stack (if any) */
        byte marked = o_ptr->marked;         /* Object is marked */
        s16b weight = o_ptr->number * o_ptr->weight;
        u16b inscription = o_ptr->inscription;

        /* Wipe it clean */
        object_prep(o_ptr, o_ptr->k_idx);

        o_ptr->iy = iy;
        o_ptr->ix = ix;
        o_ptr->next_o_idx = next_o_idx;
        o_ptr->marked = marked;
        o_ptr->inscription = inscription;
        if (item >= 0) p_ptr->total_weight += (o_ptr->weight - weight);
    }
    p_ptr->update |= PU_BONUS;
    android_calc_exp();

    /* Something happened */
    return TRUE;
}



static bool item_tester_hook_identify_fully(object_type *o_ptr)
{
    if ( (!object_is_known(o_ptr) || !obj_is_identified_fully(o_ptr))
      && (!_hack_obj_p || _hack_obj_p(o_ptr)) )
    {
        return TRUE;
    }
    if ( obj_is_identified_fully(o_ptr)
      && o_ptr->curse_flags != o_ptr->known_curse_flags )
    {
        return TRUE;
    }
    return FALSE;
}

/*
 * Fully "identify" an object in the inventory  -BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully(object_p p)
{
    int             item;
    object_type     *o_ptr;
    char            o_name[MAX_NLEN];
    cptr            q, s;
    bool old_known;

    item_tester_no_ryoute = TRUE;
    _hack_obj_p = p;
    item_tester_hook = item_tester_hook_identify_fully;

    if (can_get_item())
    {
        q = "*Identify* which item? ";
    }
    else
    {
        item_tester_hook = p;
        q = "All items are *identified*. ";
    }

    /* Get an item */
    s = "You have nothing to *identify*.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return (FALSE);
    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];

    old_known = identify_item(o_ptr); /* For the stat tracking and old_known ... */
    obj_identify_fully(o_ptr);

    handle_stuff();
    object_desc(o_name, o_ptr, 0);

/*  Message seems redundant ...
    if (equip_is_valid_slot(item))
        msg_format("%^s: %s (%c).", equip_describe_slot(item), o_name, index_to_label(item));
    else if (item >= 0)
        msg_format("In your pack: %s (%c).", o_name, index_to_label(item));
    else
        msg_format("On the ground: %s.", o_name);
*/
    if ( p_ptr->prace == RACE_MON_POSSESSOR
      && o_ptr->tval == TV_CORPSE
      && o_ptr->sval == SV_CORPSE )
    {
        if (!(r_info[o_ptr->pval].r_xtra1 & MR1_POSSESSOR))
            msg_print("You learn more about this body.");
        lore_do_probe(o_ptr->pval);
    }

    obj_display(o_ptr);
    autopick_alter_item(item, (bool)(destroy_identify && !old_known));
    return TRUE;
}

/* Recharging
 * Move mana from either the player or another device into a target device.
 * The source device is destroyed (but not the player :) but the target never is.
 * The effect may fail, however, wasting either the source device or the player's sp.
 */

static object_type *_obj_recharge_src_ptr = NULL;
static bool _obj_recharge_dest(object_type *o_ptr)
{
    /* dest must be different from source */
    if (o_ptr == _obj_recharge_src_ptr)
        return FALSE;

    switch (o_ptr->tval)
    {
    case TV_WAND: case TV_ROD: case TV_STAFF:
        if (device_sp(o_ptr) < device_max_sp(o_ptr))
            return TRUE;
        break;
    }
    return FALSE;
}

static bool _obj_recharge_src(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_WAND: case TV_ROD: case TV_STAFF:
        if (device_sp(o_ptr) > 0)
            return TRUE;
        break;
    }
    return FALSE;
}

static void _recharge_aux(object_type *o_ptr, int amt, int power)
{
    int fail_odds = 0;
    int lev = o_ptr->activation.difficulty;
    int div = 15;

    if (devicemaster_is_speciality(o_ptr))
        div = 8;

    if (power > lev/2)
        fail_odds = (power - lev/2) / div;

    if (one_in_(fail_odds))
    {
        /* Do nothing for now. My experience is that players get too conservative
         * using recharge in the dungeon if there is any chance of failure at all.
         * Remember, you're lucky if you find just one wand of rockets all game long!
         * I plan on removing the town recharging service to compensate for this
         * generosity, though. */
         msg_print("Failed!");
         return;
    }

    device_increase_sp(o_ptr, amt);
}

bool recharge_from_player(int power)
{
    int          item;
    int          amt, max;
    object_type *o_ptr;

    /* Get destination device */
    _obj_recharge_src_ptr = NULL;
    item_tester_hook = _obj_recharge_dest;
    if (!get_item(&item, "Recharge which item? ", "You have nothing to recharge.", USE_INVEN | USE_FLOOR))
        return FALSE;

    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];

    amt = power;
    max = device_max_sp(o_ptr) - device_sp(o_ptr);
    if (amt > max)
        amt = max;
    if (p_ptr->prace == RACE_MON_LEPRECHAUN)
    {
        if (amt > p_ptr->au / 100)
            amt = p_ptr->au / 100;
    }
    else if (amt > p_ptr->csp)
        amt = p_ptr->csp;

    if (p_ptr->prace == RACE_MON_LEPRECHAUN)
    {
        p_ptr->au -= amt * 100;
        stats_on_gold_services(amt * 100); /* ? */
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
    }
    else
        sp_player(-amt);

    _recharge_aux(o_ptr, amt, power);

    p_ptr->notice |= (PN_COMBINE | PN_REORDER);
    p_ptr->window |= PW_INVEN;

    return TRUE;
}

bool recharge_from_device(int power)
{
    int          src_item, dest_item;
    int          amt, max;
    bool         destroy = TRUE;
    object_type *src_ptr, *dest_ptr;

    /* Get source device */
    item_tester_hook = _obj_recharge_src;
    if (!get_item(&src_item, "Pick a source device? ", "You need a source device to power the recharging.", USE_INVEN | USE_FLOOR))
        return FALSE;

    if (src_item >= 0)
        src_ptr = &inventory[src_item];
    else
        src_ptr = &o_list[0 - src_item];

    /* Get destination device */
    _obj_recharge_src_ptr = src_ptr;
    item_tester_hook = _obj_recharge_dest;
    if (!get_item(&dest_item, "Recharge which item? ", "You have nothing to recharge.", USE_INVEN | USE_FLOOR))
        return FALSE;

    if (dest_item >= 0)
        dest_ptr = &inventory[dest_item];
    else
        dest_ptr = &o_list[0 - dest_item];

    amt = device_sp(src_ptr);
    max = device_max_sp(dest_ptr) - device_sp(dest_ptr);
    if (max > power)
        max = power;

    if (amt > max)
        amt = max;

    if (!one_in_(3) || devicemaster_is_speciality(src_ptr))
    {
        device_decrease_sp(src_ptr, amt);
        destroy = FALSE;
    }

    _recharge_aux(dest_ptr, amt, power);

    /* Destroy at the end ... otherwise, the inventory may slide and
     * invalidate our pointers (dest_ptr could end up pointing at
     * the wrong object!) */
    if (destroy)
    {
        if (object_is_fixed_artifact(src_ptr))
            device_decrease_sp(src_ptr, amt);
        else
        {
            char name[MAX_NLEN];

            object_desc(name, src_ptr, OD_OMIT_PREFIX);
            src_ptr = NULL;

            msg_format("Recharging consumes your %s!", name);
            if (src_item >= 0)
            {
                inven_item_increase(src_item, -1);
                inven_item_describe(src_item);
                inven_item_optimize(src_item);
            }
            else
            {
                floor_item_increase(0 - src_item, -1);
                floor_item_describe(0 - src_item);
                floor_item_optimize(0 - src_item);
            }
        }
    }

    p_ptr->notice |= (PN_COMBINE | PN_REORDER);
    p_ptr->window |= PW_INVEN;

    return TRUE;
}

/*
 * Bless a weapon
 */
bool bless_weapon(void)
{
    int             item;
    object_type     *o_ptr;
    u32b flgs[OF_ARRAY_SIZE];
    char            o_name[MAX_NLEN];
    cptr            q, s;

    item_tester_no_ryoute = TRUE;

    /* Bless only weapons */
    item_tester_hook = object_is_weapon;

    /* Get an item */
    q = "Bless which weapon? ";
    s = "You have weapon to bless.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
        return FALSE;

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


    /* Description */
    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Extract the flags */
    obj_flags(o_ptr, flgs);

    if (object_is_cursed(o_ptr))
    {
        if (((o_ptr->curse_flags & OFC_HEAVY_CURSE) && (randint1(100) < 33)) ||
            (o_ptr->curse_flags & OFC_PERMA_CURSE))
        {
            msg_format("The black aura on %s %s disrupts the blessing!",
                ((item >= 0) ? "your" : "the"), o_name);

            return TRUE;
        }

        msg_format("A malignant aura leaves %s %s.",
            ((item >= 0) ? "your" : "the"), o_name);


        /* Uncurse it */
        o_ptr->curse_flags = 0L;

        /* Hack -- Assume felt */
        o_ptr->ident |= (IDENT_SENSE);

        /* Take note */
        o_ptr->feeling = FEEL_NONE;

        /* Recalculate the bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Window stuff */
        p_ptr->window |= (PW_EQUIP);
    }

    /*
     * Next, we try to bless it. Artifacts have a 1/3 chance of
     * being blessed, otherwise, the operation simply disenchants
     * them, godly power negating the magic. Ok, the explanation
     * is silly, but otherwise priests would always bless every
     * artifact weapon they find. Ego weapons and normal weapons
     * can be blessed automatically.
     */
    if (have_flag(flgs, OF_BLESSED))
    {
        msg_format("%s %s %s blessed already.",
            ((item >= 0) ? "Your" : "The"), o_name,
            ((o_ptr->number > 1) ? "were" : "was"));

        return TRUE;
    }

    if (!(object_is_artifact(o_ptr) || object_is_ego(o_ptr)) || one_in_(3))
    {
        /* Describe */
        msg_format("%s %s shine%s!",
            ((item >= 0) ? "Your" : "The"), o_name,
            ((o_ptr->number > 1) ? "" : "s"));

        add_flag(o_ptr->flags, OF_BLESSED);
        add_flag(o_ptr->known_flags, OF_BLESSED);
        o_ptr->discount = 99;
    }
    else
    {
        bool dis_happened = FALSE;

        msg_print("The weapon resists your blessing!");


        /* Disenchant tohit */
        if (o_ptr->to_h > 0)
        {
            o_ptr->to_h--;
            dis_happened = TRUE;
        }

        if ((o_ptr->to_h > 5) && (randint0(100) < 33)) o_ptr->to_h--;

        /* Disenchant todam */
        if (o_ptr->to_d > 0)
        {
            o_ptr->to_d--;
            dis_happened = TRUE;
        }

        if ((o_ptr->to_d > 5) && (randint0(100) < 33)) o_ptr->to_d--;

        /* Disenchant toac */
        if (o_ptr->to_a > 0)
        {
            o_ptr->to_a--;
            dis_happened = TRUE;
        }

        if ((o_ptr->to_a > 5) && (randint0(100) < 33)) o_ptr->to_a--;

        if (dis_happened)
        {
            msg_print("There is a static feeling in the air...");

            msg_format("%s %s %s disenchanted!",
                ((item >= 0) ? "Your" : "The"), o_name,
                ((o_ptr->number > 1) ? "were" : "was"));

        }
    }

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Window stuff */
    p_ptr->window |= PW_EQUIP;

    android_calc_exp();

    return TRUE;
}


/*
 * polish shield
 */
bool polish_shield(void)
{
    int             item;
    object_type     *o_ptr;
    u32b flgs[OF_ARRAY_SIZE];
    char            o_name[MAX_NLEN];
    cptr            q, s;

    item_tester_no_ryoute = TRUE;
    /* Assume enchant weapon */
    item_tester_tval = TV_SHIELD;

    /* Get an item */
    q = "Polish which weapon? ";
    s = "You have no shield to polish.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
        return FALSE;

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


    /* Description */
    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Extract the flags */
    obj_flags(o_ptr, flgs);

    if (o_ptr->k_idx && !object_is_artifact(o_ptr) && !object_is_ego(o_ptr) &&
        !object_is_cursed(o_ptr) && (o_ptr->sval != SV_MIRROR_SHIELD))
    {
        msg_format("%s %s shine%s!",
            ((item >= 0) ? "Your" : "The"), o_name,
            ((o_ptr->number > 1) ? "" : "s"));
        o_ptr->name2 = EGO_SHIELD_REFLECTION;
        enchant(o_ptr, randint0(3) + 4, ENCH_TOAC);

        o_ptr->discount = 99;
        virtue_add(VIRTUE_ENCHANTMENT, 2);

        return TRUE;
    }
    else
    {
        if (flush_failure) flush();

        msg_print("Failed.");

        virtue_add(VIRTUE_ENCHANTMENT, -2);
    }
    android_calc_exp();

    return FALSE;
}


/*
 * Potions "smash open" and cause an area effect when
 * (1) they are shattered while in the player's inventory,
 * due to cold (etc) attacks;
 * (2) they are thrown at a monster, or obstacle;
 * (3) they are shattered by a "cold ball" or other such spell
 * while lying on the floor.
 *
 * Arguments:
 *    who   ---  who caused the potion to shatter (0=player)
 *          potions that smash on the floor are assumed to
 *          be caused by no-one (who = 1), as are those that
 *          shatter inside the player inventory.
 *          (Not anymore -- I changed this; TY)
 *    y, x  --- coordinates of the potion (or player if
 *          the potion was in her inventory);
 *    o_ptr --- pointer to the potion object.
 */
bool potion_smash_effect(int who, int y, int x, int k_idx)
{
    int     radius = 2;
    int     dt = 0;
    int     dam = 0;
    bool    angry = FALSE;

    object_kind *k_ptr = &k_info[k_idx];

    switch (k_ptr->sval)
    {
        case SV_POTION_SALT_WATER:
        case SV_POTION_SLIME_MOLD:
        case SV_POTION_LOSE_MEMORIES:
        case SV_POTION_DEC_STR:
        case SV_POTION_DEC_INT:
        case SV_POTION_DEC_WIS:
        case SV_POTION_DEC_DEX:
        case SV_POTION_DEC_CON:
        case SV_POTION_DEC_CHR:
        case SV_POTION_WATER:   /* perhaps a 'water' attack? */
        case SV_POTION_APPLE_JUICE:
            return TRUE;

        case SV_POTION_INFRAVISION:
        case SV_POTION_DETECT_INVIS:
        case SV_POTION_SLOW_POISON:
        case SV_POTION_CURE_POISON:
        case SV_POTION_BOLDNESS:
        case SV_POTION_RESIST_HEAT:
        case SV_POTION_RESIST_COLD:
        case SV_POTION_HEROISM:
        case SV_POTION_BERSERK_STRENGTH:
        case SV_POTION_RES_STR:
        case SV_POTION_RES_INT:
        case SV_POTION_RES_WIS:
        case SV_POTION_RES_DEX:
        case SV_POTION_RES_CON:
        case SV_POTION_RES_CHR:
        case SV_POTION_INC_STR:
        case SV_POTION_INC_INT:
        case SV_POTION_INC_WIS:
        case SV_POTION_INC_DEX:
        case SV_POTION_INC_CON:
        case SV_POTION_INC_CHR:
        case SV_POTION_AUGMENTATION:
        case SV_POTION_ENLIGHTENMENT:
        case SV_POTION_STAR_ENLIGHTENMENT:
        case SV_POTION_SELF_KNOWLEDGE:
        case SV_POTION_EXPERIENCE:
        case SV_POTION_RESISTANCE:
        case SV_POTION_INVULNERABILITY:
        case SV_POTION_NEW_LIFE:
            /* All of the above potions have no effect when shattered */
            return FALSE;
        case SV_POTION_SLOWNESS:
            dt = GF_OLD_SLOW;
            dam = 5;
            angry = TRUE;
            break;
        case SV_POTION_POISON:
            dt = GF_POIS;
            dam = 3;
            angry = TRUE;
            break;
        case SV_POTION_BLINDNESS:
            dt = GF_DARK;
            angry = TRUE;
            break;
        case SV_POTION_CONFUSION: /* Booze */
            dt = GF_OLD_CONF;
            angry = TRUE;
            break;
        case SV_POTION_SLEEP:
            dt = GF_OLD_SLEEP;
            angry = TRUE;
            break;
        case SV_POTION_RUINATION:
        case SV_POTION_DETONATIONS:
            dt = GF_SHARDS;
            dam = damroll(25, 25);
            angry = TRUE;
            break;
        case SV_POTION_DEATH:
            dt = GF_DEATH_RAY;    /* !! */
            dam = k_ptr->level * 10;
            angry = TRUE;
            radius = 1;
            break;
        case SV_POTION_SPEED:
            dt = GF_OLD_SPEED;
            break;
        case SV_POTION_CURE_LIGHT:
            dt = GF_OLD_HEAL;
            dam = damroll(2, 3);
            break;
        case SV_POTION_CURE_SERIOUS:
            dt = GF_OLD_HEAL;
            dam = damroll(4, 3);
            break;
        case SV_POTION_CURE_CRITICAL:
        case SV_POTION_CURING:
            dt = GF_OLD_HEAL;
            dam = damroll(6, 3);
            break;
        case SV_POTION_HEALING:
            dt = GF_OLD_HEAL;
            dam = damroll(10, 10);
            break;
        case SV_POTION_RESTORE_EXP:
            dt = GF_STAR_HEAL;
            dam = 0;
            radius = 1;
            break;
        case SV_POTION_LIFE:
            dt = GF_STAR_HEAL;
            dam = damroll(50, 50);
            radius = 1;
            break;
        case SV_POTION_STAR_HEALING:
            dt = GF_OLD_HEAL;
            dam = damroll(50, 50);
            radius = 1;
            break;
        case SV_POTION_RESTORE_MANA:   /* MANA */
            dt = GF_MANA;
            dam = damroll(10, 10);
            radius = 1;
            break;
        default:
            /* Do nothing */  ;
    }

    (void)project(who, radius, y, x, dam, dt,
        (PROJECT_JUMP | PROJECT_ITEM | PROJECT_KILL), -1);

    /* XXX  those potions that explode need to become "known" */
    return angry;
}


/*
 * Returns experience of a spell
 */
s16b experience_of_spell(int spell, int use_realm)
{
    if (p_ptr->pclass == CLASS_SORCERER) return SPELL_EXP_MASTER;
    else if (p_ptr->pclass == CLASS_RED_MAGE) return SPELL_EXP_SKILLED;
    else if (use_realm == p_ptr->realm1) return p_ptr->spell_exp[spell];
    else if (use_realm == p_ptr->realm2) return p_ptr->spell_exp[spell + 32];
    else return 0;
}


/*
 * Modify mana consumption rate using spell exp and p_ptr->dec_mana
 */
int mod_need_mana(int need_mana, int spell, int realm)
{
#define MANA_CONST   2400
#define MANA_DIV        4
#define DEC_MANA_DIV    3

    bool dec_mana = p_ptr->dec_mana;

    if (p_ptr->easy_realm1 && realm == p_ptr->easy_realm1)
        dec_mana = TRUE;

    /* Realm magic */
    if ((realm > REALM_NONE) && (realm <= MAX_REALM))
    {
        /*
         * need_mana defaults if spell exp equals SPELL_EXP_EXPERT and !p_ptr->dec_mana.
         * MANA_CONST is used to calculate need_mana effected from spell proficiency.
         */
        need_mana = need_mana * (MANA_CONST + SPELL_EXP_EXPERT - experience_of_spell(spell, realm)) + (MANA_CONST - 1);
        need_mana *= dec_mana ? DEC_MANA_DIV : MANA_DIV;
        need_mana /= MANA_CONST * MANA_DIV;
        if (need_mana < 1) need_mana = 1;
    }

    /* Non-realm magic */
    else
    {
        if (p_ptr->dec_mana) need_mana = (need_mana + 1) * DEC_MANA_DIV / MANA_DIV;
    }

#undef DEC_MANA_DIV
#undef MANA_DIV
#undef MANA_CONST

    if (p_ptr->tim_blood_rite)
        need_mana *= 2;

    return need_mana;
}


/*
 * Modify spell fail rate
 * Using p_ptr->to_m_chance, p_ptr->dec_mana, p_ptr->easy_spell and p_ptr->heavy_spell
 */
int mod_spell_chance_1(int chance, int realm)
{
    bool dec_mana = p_ptr->dec_mana;

    if (realm && realm == p_ptr->easy_realm1)
        dec_mana = TRUE;

    chance += p_ptr->to_m_chance;

    if (p_ptr->heavy_spell) chance += 20;

    if (p_ptr->pclass == CLASS_BLOOD_MAGE)
    {
        if (p_ptr->cumber_glove) chance += 20;
        if (p_ptr->cumber_armor) chance += 100 * p_ptr->cumber_armor_amt / 600;
    }

    if (dec_mana && p_ptr->easy_spell) chance -= 4;
    else if (p_ptr->easy_spell) chance -= 3;
    else if (dec_mana) chance -= 2;

    if (mut_present(MUT_ARCANE_MASTERY))
        chance -= 3;
    if (prace_is_(RACE_DEMIGOD) && p_ptr->psubrace == DEMIGOD_ATHENA)
        chance -= 2;

    return chance;
}


/*
 * Modify spell fail rate (as "suffix" process)
 * Using p_ptr->dec_mana, p_ptr->easy_spell and p_ptr->heavy_spell
 * Note: variable "chance" cannot be negative.
 */
int mod_spell_chance_2(int chance, int realm)
{
    bool dec_mana = p_ptr->dec_mana;

    if (realm && realm == p_ptr->easy_realm1)
        dec_mana = TRUE;

    if (dec_mana) chance--;
    if (p_ptr->heavy_spell) chance += 5;
    if (p_ptr->pclass == CLASS_BLOOD_MAGE)
    {
        if (p_ptr->cumber_glove) chance += 5;
        if (p_ptr->cumber_armor) chance += 5;
    }

    return MAX(chance, 0);
}


/*
 * Returns spell chance of failure for spell -RAK-
 */
s16b spell_chance(int spell, int use_realm)
{
    int             chance, minfail;
    magic_type      *s_ptr;
    int             need_mana;
    caster_info        *caster_ptr = get_caster_info();

    /* Paranoia -- must be literate */
    if (!mp_ptr->spell_book) return (100);

    if (use_realm == REALM_HISSATSU) return 0;

    /* Access the spell */
    if (!is_magic(use_realm))
    {
        s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
    }
    else
    {
        s_ptr = &mp_ptr->info[use_realm - 1][spell];
    }

    /* Extract the base spell failure rate */
    chance = s_ptr->sfail;

    /* Reduce failure rate by "effective" level adjustment */
    chance -= 3 * (p_ptr->lev - s_ptr->slevel);

    /* Reduce failure rate by INT/WIS adjustment */
    chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[mp_ptr->spell_stat]] - 1);

    if (p_ptr->riding)
        chance += MAX(r_info[m_list[p_ptr->riding].r_idx].level - skills_riding_current() / 100 - 10, 0);

    /* Extract mana consumption rate */
    need_mana = mod_need_mana(s_ptr->smana, spell, use_realm);

    /* Not enough mana to cast */
    if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
    {
        /* Spells can't be cast without enough hp, so just leave the fail rate alone! */
    }
    else if (need_mana > p_ptr->csp)
    {
        chance += 5 * (need_mana - p_ptr->csp);
    }

    if ((use_realm != p_ptr->realm1) && ((p_ptr->pclass == CLASS_MAGE) || (p_ptr->pclass == CLASS_BLOOD_MAGE) || (p_ptr->pclass == CLASS_PRIEST))) chance += 5;

    /* Extract the minimum failure rate */
    minfail = adj_mag_fail[p_ptr->stat_ind[mp_ptr->spell_stat]];

    /*
     * Non mage/priest characters never get too good
     * (added high mage, mindcrafter)
     */
    if (mp_ptr->spell_xtra & MAGIC_FAIL_5PERCENT)
    {
        if (minfail < 5) minfail = 5;
    }
    if (prace_is_(RACE_DEMIGOD) && p_ptr->psubrace == DEMIGOD_ATHENA && minfail > 0)
        minfail -= 1;

    /* Hack -- Priest prayer penalty for "edged" weapons  -DGK */
    if (((p_ptr->pclass == CLASS_PRIEST) || (p_ptr->pclass == CLASS_SORCERER)) && p_ptr->weapon_info[0].icky_wield) chance += 25;
    if (((p_ptr->pclass == CLASS_PRIEST) || (p_ptr->pclass == CLASS_SORCERER)) && p_ptr->weapon_info[1].icky_wield) chance += 25;

    chance = mod_spell_chance_1(chance, use_realm);

    /* Goodness or evilness gives a penalty to failure rate */
    {
        int    penalty = 2;
        if (caster_ptr && caster_ptr->which_stat == A_WIS)
            penalty = 5;

        switch (use_realm)
        {
        case REALM_NATURE:
            if (p_ptr->align > 50 || p_ptr->align < -50) chance += penalty;
            break;
        case REALM_LIFE: case REALM_CRUSADE:
            if (p_ptr->align < -20) chance += penalty;
            if (p_ptr->align >= 50) chance -= penalty;
            break;
        case REALM_DEATH: case REALM_DAEMON: case REALM_HEX:
            if (p_ptr->align > 20) chance += penalty;
            if (p_ptr->align <= -50) chance -= penalty;
            break;
        }
    }

    /* Minimum failure rate */
    if (chance < minfail) chance = minfail;

    /* Stunning makes spells harder */
    if (p_ptr->stun > 50) chance += 25;
    else if (p_ptr->stun) chance += 15;

    /* Always a 5 percent chance of working */
    if (chance > 95) chance = 95;

    if ((use_realm == p_ptr->realm1) || (use_realm == p_ptr->realm2)
        || (p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE))
    {
        s16b exp = experience_of_spell(spell, use_realm);
        if (exp >= SPELL_EXP_EXPERT) chance--;
        if (exp >= SPELL_EXP_MASTER) chance--;
    }

    /* Return the chance */
    return mod_spell_chance_2(chance, use_realm);
}



/*
 * Determine if a spell is "okay" for the player to cast or study
 * The spell must be legible, not forgotten, and also, to cast,
 * it must be known, and to study, it must not be known.
 */
bool spell_okay(int spell, bool learned, bool study_pray, int use_realm)
{
    magic_type *s_ptr;

    /* Access the spell */
    if (!is_magic(use_realm))
    {
        s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
    }
    else
    {
        s_ptr = &mp_ptr->info[use_realm - 1][spell];
    }

    /* Spell is illegal */
    if (s_ptr->slevel > p_ptr->lev) return (FALSE);

    /* Spell is forgotten */
    if ((use_realm == p_ptr->realm2) ?
        (p_ptr->spell_forgotten2 & (1L << spell)) :
        (p_ptr->spell_forgotten1 & (1L << spell)))
    {
        /* Never okay */
        return (FALSE);
    }

    if (p_ptr->pclass == CLASS_SORCERER) return (TRUE);
    if (p_ptr->pclass == CLASS_RED_MAGE) return (TRUE);

    /* Spell is learned */
    if ((use_realm == p_ptr->realm2) ?
        (p_ptr->spell_learned2 & (1L << spell)) :
        (p_ptr->spell_learned1 & (1L << spell)))
    {
        /* Always true */
        return (!study_pray);
    }

    /* Okay to study, not to cast */
    return (!learned);
}


/*
 * Print a list of spells (for browsing or casting or viewing)
 */
void print_spells(int target_spell, byte *spells, int num, rect_t display, int use_realm)
{
    int             i, spell, exp_level, increment = 64;
    magic_type     *s_ptr;
    cptr            comment;
    char            info[80];
    char            out_val[160];
    byte            line_attr;
    int             need_mana;
    char            ryakuji[15];
    char            buf[256];
    bool            max = FALSE;
    caster_info    *caster_ptr = get_caster_info();


    if (((use_realm <= REALM_NONE) || (use_realm > MAX_REALM)) && p_ptr->wizard)
        msg_print("Warning! print_spells called with null realm");


    /* Title the list */
    if (use_realm == REALM_HISSATSU)
        strcpy(buf,"  Lvl  SP");
    else
    {
        if (caster_ptr && (caster_ptr->options & CASTER_USE_HP))
            strcpy(buf,"Profic Lvl  HP Fail Desc");
        else
            strcpy(buf,"Profic Lvl  SP Fail Desc");
    }

    Term_erase(display.x, display.y, display.cx);
    put_str("Name", display.y, display.x + 5);
    put_str(buf, display.y, display.x + 29);

    if ((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE)) increment = 0;
    else if (use_realm == p_ptr->realm1) increment = 0;
    else if (use_realm == p_ptr->realm2) increment = 32;

    /* Dump the spells */
    for (i = 0; i < num; i++)
    {
        Term_erase(display.x, display.y + i + 1, display.cx);

        /* Access the spell */
        spell = spells[i];

        /* Access the spell */
        if (!is_magic(use_realm))
        {
            s_ptr = &technic_info[use_realm - MIN_TECHNIC][spell];
        }
        else
        {
            s_ptr = &mp_ptr->info[use_realm - 1][spell];
        }

        if (use_realm == REALM_HISSATSU)
            need_mana = s_ptr->smana;
        else
        {
            s16b exp = experience_of_spell(spell, use_realm);

            /* Extract mana consumption rate */
            need_mana = mod_need_mana(s_ptr->smana, spell, use_realm);

            if ((increment == 64) || (s_ptr->slevel >= 99)) exp_level = EXP_LEVEL_UNSKILLED;
            else exp_level = spell_exp_level(exp);

            max = FALSE;
            if (!increment && (exp_level == EXP_LEVEL_MASTER)) max = TRUE;
            else if ((increment == 32) && (exp_level >= EXP_LEVEL_EXPERT)) max = TRUE;
            else if (s_ptr->slevel >= 99) max = TRUE;
            else if ((p_ptr->pclass == CLASS_RED_MAGE) && (exp_level >= EXP_LEVEL_SKILLED)) max = TRUE;

            strncpy(ryakuji, exp_level_str[exp_level], 4);
            ryakuji[3] = ']';
            ryakuji[4] = '\0';
        }

        if (use_menu && target_spell)
        {
            if (i == (target_spell-1))
                strcpy(out_val, "  >  ");
            else
                strcpy(out_val, "     ");
        }
        else sprintf(out_val, "  %c) ", I2A(i));
        /* Skip illegible spells */
        if (s_ptr->slevel >= 99)
        {
                strcat(out_val, format("%-30s", "(illegible)"));
                c_put_str(TERM_L_DARK, out_val, display.y + i + 1, display.x);
                continue;
        }

        /* XXX XXX Could label spells above the players level */

        /* Get extra info */
        strcpy(info, do_spell(use_realm, spell, SPELL_INFO));

        /* Use that info */
        comment = info;

        /* Assume spell is known and tried */
        line_attr = TERM_WHITE;

        /* Analyze the spell */
        if ((p_ptr->pclass == CLASS_SORCERER) || (p_ptr->pclass == CLASS_RED_MAGE))
        {
            if (s_ptr->slevel > p_ptr->max_plv)
            {
                comment = "unknown";

                line_attr = TERM_L_BLUE;
            }
            else if (s_ptr->slevel > p_ptr->lev)
            {
                comment = "forgotten";

                line_attr = TERM_YELLOW;
            }
        }
        else if ((use_realm != p_ptr->realm1) && (use_realm != p_ptr->realm2))
        {
            comment = "unknown";

            line_attr = TERM_L_BLUE;
        }
        else if ((use_realm == p_ptr->realm1) ?
            ((p_ptr->spell_forgotten1 & (1L << spell))) :
            ((p_ptr->spell_forgotten2 & (1L << spell))))
        {
            comment = "forgotten";

            line_attr = TERM_YELLOW;
        }
        else if (!((use_realm == p_ptr->realm1) ?
            (p_ptr->spell_learned1 & (1L << spell)) :
            (p_ptr->spell_learned2 & (1L << spell))))
        {
            comment = "unknown";

            line_attr = TERM_L_BLUE;
        }
        else if (!((use_realm == p_ptr->realm1) ?
            (p_ptr->spell_worked1 & (1L << spell)) :
            (p_ptr->spell_worked2 & (1L << spell))))
        {
            comment = "untried";

            line_attr = TERM_L_GREEN;
        }

        /* Dump the spell --(-- */
        if (use_realm == REALM_HISSATSU)
        {
            strcat(out_val, format("%-25s %3d %3d",
                do_spell(use_realm, spell, SPELL_NAME), /* realm, spell */
                s_ptr->slevel, need_mana));
        }
        else
        {
            strcat(out_val, format("%-25s%c%-4s %3d %3d %3d%% %s",
                do_spell(use_realm, spell, SPELL_NAME), /* realm, spell */
                (max ? '!' : ' '), ryakuji,
                s_ptr->slevel, need_mana, spell_chance(spell, use_realm), comment));
        }
        c_put_str(line_attr, out_val, display.y + i + 1, display.x);
    }

    /* Clear the bottom line */
    Term_erase(display.x, display.y + i + 1, display.cx);
}


/*
 * Note that amulets, rods, and high-level spell books are immune
 * to "inventory damage" of any kind. Also sling ammo and shovels.
 */


/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
bool hates_acid(object_type *o_ptr)
{
    /* Analyze the type */
    switch (o_ptr->tval)
    {
        /* Wearable items */
        case TV_ARROW:
        case TV_BOLT:
        case TV_BOW:
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        {
            return (TRUE);
        }

        /* Staffs/Scrolls are wood/paper */
        case TV_STAFF:
        case TV_SCROLL:
        {
            return (TRUE);
        }

        /* Ouch */
        case TV_CHEST:
        {
            return (TRUE);
        }

        /* Junk is useless */
        case TV_SKELETON:
        case TV_BOTTLE:
        case TV_JUNK:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * Does a given object (usually) hate electricity?
 */
bool hates_elec(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_RING:
        case TV_WAND:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
bool hates_fire(object_type *o_ptr)
{
    /* Analyze the type */
    switch (o_ptr->tval)
    {
        /* Wearable */
        case TV_LITE:
        case TV_ARROW:
        case TV_BOW:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        {
            return (TRUE);
        }

        /* Books */
        case TV_LIFE_BOOK:
        case TV_SORCERY_BOOK:
        case TV_NATURE_BOOK:
        case TV_CHAOS_BOOK:
        case TV_DEATH_BOOK:
        case TV_TRUMP_BOOK:
        case TV_ARCANE_BOOK:
        case TV_CRAFT_BOOK:
        case TV_DAEMON_BOOK:
        case TV_CRUSADE_BOOK:
        case TV_NECROMANCY_BOOK:
        case TV_ARMAGEDDON_BOOK:
        case TV_MUSIC_BOOK:
        case TV_HISSATSU_BOOK:
        case TV_HEX_BOOK:
        {
            return (TRUE);
        }

        /* Chests */
        case TV_CHEST:
        {
            return (TRUE);
        }

        /* Staffs/Scrolls burn */
        case TV_STAFF:
        case TV_SCROLL:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
bool hates_cold(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_POTION:
        case TV_FLASK:
        case TV_BOTTLE:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * Melt something
 */
int set_acid_destroy(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (!hates_acid(o_ptr)) return (FALSE);
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_IGNORE_ACID)) return (FALSE);
    return (TRUE);
}


/*
 * Electrical damage
 */
int set_elec_destroy(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (!hates_elec(o_ptr)) return (FALSE);
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_IGNORE_ELEC)) return (FALSE);
    return (TRUE);
}


/*
 * Burn something
 */
int set_fire_destroy(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (!hates_fire(o_ptr)) return (FALSE);
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_IGNORE_FIRE)) return (FALSE);
    return (TRUE);
}


/*
 * Freeze things
 */
int set_cold_destroy(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (!hates_cold(o_ptr)) return (FALSE);
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_IGNORE_COLD)) return (FALSE);
    return (TRUE);
}


/*
 * Destroys a type of item on a given percent chance
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * New-style wands and rods handled correctly. -LM-
 * Returns number of items destroyed.
 */
int inven_damage(inven_func typ, int p1, int which)
{
    int         i, j, k, amt;
    object_type *o_ptr;
    char        o_name[MAX_NLEN];
    int         p2 = 100;

    if (CHECK_MULTISHADOW()) return 0;
    if (p_ptr->inside_arena) return 0;

    if (p_ptr->rune_elem_prot) p2 = 50;
    if (p_ptr->inven_prot) p2 = 50;

    /* Count the casualties */
    k = 0;

    /* Scan through the slots backwards */
    for (i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Hack -- for now, skip artifacts */
        if (object_is_artifact(o_ptr)) continue;

        /* Give this item slot a shot at death */
        if ((*typ)(o_ptr))
        {
            /* Count the casualties */
            for (amt = j = 0; j < o_ptr->number; ++j)
            {
                if ( randint0(100) < p1    /* Effects of Breath Quality */
                  && randint0(100) < p2 /* Effects of Inventory Protection (Rune or Spell) */
                  && !res_save_inventory(which) ) /* Effects of Resistance */
                {
                    amt++;
                }
            }

            /* Some casualities */
            if (amt)
            {
                /* Get a description */
                object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_COLOR_CODED);

                msg_format("%d of your %s (%c) %s destroyed!",
                            amt, o_name, index_to_label(i), (amt > 1) ? "were" : "was");

                /* Potions smash open */
                if (object_is_potion(o_ptr))
                {
                    (void)potion_smash_effect(0, py, px, o_ptr->k_idx);
                }

                /* Reduce the charges of rods/wands */
                reduce_charges(o_ptr, amt);

                stats_on_m_destroy(o_ptr, amt);

                /* Destroy "amt" items */
                inven_item_increase(i, -amt);
                inven_item_optimize(i);

                /* Count the casualties */
                k += amt;
            }
        }
    }

    /* Return the casualty count */
    return (k);
}


/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(void)
{
    int slot = equip_random_slot(object_is_armour);

    if (slot)
    {
        object_type *o_ptr = equip_obj(slot);
        u32b         flgs[OF_ARRAY_SIZE];
        char         o_name[MAX_NLEN];

        if (o_ptr->ac + o_ptr->to_a <= 0) return FALSE;

        object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
        obj_flags(o_ptr, flgs);

        if ( have_flag(flgs, OF_IGNORE_ACID)
          || demigod_is_(DEMIGOD_HEPHAESTUS) )
        {
            msg_format("Your %s is unaffected!", o_name);
            obj_learn_flag(o_ptr, OF_IGNORE_ACID);
            return TRUE;
        }

        msg_format("Your %s is damaged!", o_name);
        o_ptr->to_a--;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        android_calc_exp();
        return TRUE;
    }
    return FALSE;
}


/*
 * Hurt the player with Acid
 */
static int _inv_dam_pct(int dam)
{
    return (dam < 30) ? 3 : (dam < 60) ? 6 : 9;
}

int acid_dam(int dam, cptr kb_str, int monspell)
{
    int get_damage;
    int inv = _inv_dam_pct(dam);

    dam = res_calc_dam(RES_ACID, dam);

    /* Total Immunity */
    if (dam <= 0)
    {
        learn_spell(monspell);
        return 0;
    }

    if (!CHECK_MULTISHADOW())
    {
        if (!res_save_default(RES_ACID) && one_in_(HURT_CHANCE))
            (void)do_dec_stat(A_CHR);

        /* If any armor gets hit, defend the player */
        if (minus_ac()) dam = (dam + 1) / 2;
    }

    /* Take damage */
    get_damage = take_hit(DAMAGE_ATTACK, dam, kb_str, monspell);

    /* Inventory damage */
    inven_damage(set_acid_destroy, inv, RES_ACID);

    return get_damage;
}


/*
 * Hurt the player with electricity
 */
int elec_dam(int dam, cptr kb_str, int monspell)
{
    int get_damage;
    int inv = _inv_dam_pct(dam);

    dam = res_calc_dam(RES_ELEC, dam);

    /* Total immunity */
    if (dam <= 0)
    {
        learn_spell(monspell);
        return 0;
    }

    if (!CHECK_MULTISHADOW())
    {
        if (!res_save_default(RES_ELEC) && one_in_(HURT_CHANCE))
            (void)do_dec_stat(A_DEX);
    }

    /* Take damage */
    get_damage = take_hit(DAMAGE_ATTACK, dam, kb_str, monspell);

    /* Inventory damage */
    inven_damage(set_elec_destroy, inv, RES_ELEC);

    return get_damage;
}


/*
 * Hurt the player with Fire
 */
int fire_dam(int dam, cptr kb_str, int monspell)
{
    int get_damage;
    int inv = _inv_dam_pct(dam);

    dam = res_calc_dam(RES_FIRE, dam);

    /* Totally immune */
    if (dam <= 0)
    {
        learn_spell(monspell);
        return 0;
    }

    if (!CHECK_MULTISHADOW())
    {
        if (!res_save_default(RES_FIRE) && one_in_(HURT_CHANCE))
            (void)do_dec_stat(A_STR);
    }

    /* Take damage */
    get_damage = take_hit(DAMAGE_ATTACK, dam, kb_str, monspell);

    /* Inventory damage */
    inven_damage(set_fire_destroy, inv, RES_FIRE);

    return get_damage;
}


/*
 * Hurt the player with Cold
 */
int cold_dam(int dam, cptr kb_str, int monspell)
{
    int get_damage;
    int inv = _inv_dam_pct(dam);

    dam = res_calc_dam(RES_COLD, dam);

    /* Total immunity */
    if (dam <= 0)
    {
        learn_spell(monspell);
        return 0;
    }

    if (!CHECK_MULTISHADOW())
    {
        if (!res_save_default(RES_COLD) && one_in_(HURT_CHANCE))
            (void)do_dec_stat(A_STR);
    }

    /* Take damage */
    get_damage = take_hit(DAMAGE_ATTACK, dam, kb_str, monspell);

    /* Inventory damage */
    inven_damage(set_cold_destroy, inv, RES_COLD);

    return get_damage;
}


bool rustproof(void)
{
    int         item;
    object_type *o_ptr;
    char        o_name[MAX_NLEN];
    cptr        q, s;

    item_tester_no_ryoute = TRUE;
    /* Select a piece of armour */
    item_tester_hook = object_is_armour;

    /* Get an item */
    q = "Rustproof which piece of armour? ";
    s = "You have nothing to rustproof.";

    if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return FALSE;

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


    /* Description */
    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    add_flag(o_ptr->flags, OF_IGNORE_ACID);
    add_flag(o_ptr->known_flags, OF_IGNORE_ACID);

    if ((o_ptr->to_a < 0) && !object_is_cursed(o_ptr))
    {
        msg_format("%s %s look%s as good as new!",
            ((item >= 0) ? "Your" : "The"), o_name,
            ((o_ptr->number > 1) ? "" : "s"));
        o_ptr->to_a = 0;
    }

    msg_format("%s %s %s now protected against corrosion.",
        ((item >= 0) ? "Your" : "The"), o_name,
        ((o_ptr->number > 1) ? "are" : "is"));

    android_calc_exp();
    return TRUE;
}

/*
 * Helper for Cursing Equipment (?Curse Armor and ?Curse Weapn)
 * Also used when sacrificing a worn piece of equipment.
 */

void blast_object(object_type *o_ptr)
{
    bool is_armor = object_is_armour(o_ptr);
    bool is_weapon = object_is_weapon(o_ptr);
    int i;

    if (have_flag(o_ptr->flags, OF_NO_REMOVE))
        return;

    o_ptr->name1 = 0;
    o_ptr->name2 = EGO_SPECIAL_BLASTED;
    o_ptr->name3 = 0;
    o_ptr->to_a = 0;
    o_ptr->to_h = 0;
    o_ptr->to_d = 0;
    o_ptr->ac = 0;
    o_ptr->dd = 0;
    o_ptr->ds = 0;

    if (is_armor)
        o_ptr->to_a -= randint1(5) + randint1(5);

    if (is_weapon)
    {
        o_ptr->to_h -= randint1(5) + randint1(5);
        o_ptr->to_d -= randint1(5) + randint1(5);
    }

    for (i = 0; i < OF_ARRAY_SIZE; i++)
        o_ptr->flags[i] = 0;

    o_ptr->rune = 0;

    o_ptr->ident |= (IDENT_BROKEN);

    p_ptr->update |= (PU_BONUS);
    p_ptr->update |= (PU_MANA);
    p_ptr->window |= (PW_INVEN | PW_EQUIP);
}

/*
 * Curse the players armor
 */
bool curse_armor(int slot)
{
    object_type *o_ptr;
    char o_name[MAX_NLEN];

    o_ptr = &inventory[slot];
    if (!o_ptr->k_idx) return (FALSE);

    object_desc(o_name, o_ptr, OD_OMIT_PREFIX);

    if (object_is_artifact(o_ptr) && (randint0(100) < 50))
    {
        msg_format("A %s tries to %s, but your %s resists the effects!",
               "terrible black aura", "surround your armor", o_name);
    }
    else
    {
        msg_format("A terrible black aura blasts your %s!", o_name);
        virtue_add(VIRTUE_ENCHANTMENT, -5);
        blast_object(o_ptr);
        o_ptr->curse_flags = OFC_CURSED;
    }

    return TRUE;
}


/*
 * Curse the players weapon
 */
bool curse_weapon(bool force, int slot)
{
    object_type *o_ptr;
    char o_name[MAX_NLEN];

    o_ptr = &inventory[slot];
    if (!o_ptr->k_idx) return FALSE;

    object_desc(o_name, o_ptr, OD_OMIT_PREFIX);

    /* Attempt a saving throw */
    if (object_is_artifact(o_ptr) && (randint0(100) < 50) && !force)
    {
        msg_format("A %s tries to %s, but your %s resists the effects!",
               "terrible black aura", "surround your weapon", o_name);
    }
    else
    {
        if (!force) msg_format("A terrible black aura blasts your %s!", o_name);
        virtue_add(VIRTUE_ENCHANTMENT, -5);
        blast_object(o_ptr);
        o_ptr->curse_flags = OFC_CURSED;
    }
    return TRUE;
}

/*
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
static s16b poly_r_idx(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    int i, r, lev1, lev2;

    /* Hack -- Uniques/Questors never polymorph */
    if ((r_ptr->flags1 & RF1_UNIQUE) ||
        (r_ptr->flags1 & RF1_QUESTOR))
        return (r_idx);

    /* Allowable range of "levels" for resulting monster */
    lev1 = r_ptr->level - ((randint1(20) / randint1(9)) + 1);
    lev2 = r_ptr->level + ((randint1(20) / randint1(9)) + 1);

    /* Pick a (possibly new) non-unique race */
    for (i = 0; i < 1000; i++)
    {
        /* Pick a new race, using a level calculation */
        r = get_mon_num((dun_level + r_ptr->level) / 2 + 5);

        /* Handle failure */
        if (!r) break;

        /* Obtain race */
        r_ptr = &r_info[r];

        /* Ignore unique monsters */
        if (r_ptr->flags1 & RF1_UNIQUE) continue;

        /* Ignore monsters with incompatible levels */
        if ((r_ptr->level < lev1) || (r_ptr->level > lev2)) continue;

        /* Use that index */
        r_idx = r;

        /* Done */
        break;
    }

    /* Result */
    return (r_idx);
}


bool polymorph_monster(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];
    monster_type *m_ptr = &m_list[c_ptr->m_idx];
    bool polymorphed = FALSE;
    int new_r_idx;
    int old_r_idx = m_ptr->r_idx;
    bool targeted = (target_who == c_ptr->m_idx) ? TRUE : FALSE;
    bool health_tracked = (p_ptr->health_who == c_ptr->m_idx) ? TRUE : FALSE;
    monster_type back_m;

    if (p_ptr->inside_arena || p_ptr->inside_battle) return (FALSE);

    if ((p_ptr->riding == c_ptr->m_idx) || (m_ptr->mflag2 & MFLAG2_KAGE)) return (FALSE);

    /* Memorize the monster before polymorphing */
    back_m = *m_ptr;

    /* Pick a "new" monster race */
    new_r_idx = poly_r_idx(old_r_idx);

    /* Handle polymorph */
    if (new_r_idx != old_r_idx)
    {
        u32b mode = 0L;
        bool preserve_hold_objects = back_m.hold_o_idx ? TRUE : FALSE;
        s16b this_o_idx, next_o_idx = 0;

        /* Get the monsters attitude */
        if (is_friendly(m_ptr)) mode |= PM_FORCE_FRIENDLY;
        if (is_pet(m_ptr)) mode |= PM_FORCE_PET;
        if (m_ptr->mflag2 & MFLAG2_NOPET) mode |= PM_NO_PET;

        /* Mega-hack -- ignore held objects */
        m_ptr->hold_o_idx = 0;

        /* "Kill" the "old" monster */
        delete_monster_idx(c_ptr->m_idx);

        /* Create a new monster (no groups) */
        if (place_monster_aux(0, y, x, new_r_idx, mode))
        {
            m_list[hack_m_idx_ii].nickname = back_m.nickname;
            mon_set_parent(&m_list[hack_m_idx_ii], back_m.parent_m_idx);
            m_list[hack_m_idx_ii].hold_o_idx = back_m.hold_o_idx;

            /* Success */
            polymorphed = TRUE;
        }
        else
        {
            /* Placing the new monster failed */
            if (place_monster_aux(0, y, x, old_r_idx, (mode | PM_NO_KAGE | PM_IGNORE_TERRAIN)))
            {
                m_list[hack_m_idx_ii] = back_m;

                /* Re-initialize monster process */
                mproc_init();
            }
            else preserve_hold_objects = FALSE;
        }

        /* Mega-hack -- preserve held objects */
        if (preserve_hold_objects)
        {
            for (this_o_idx = back_m.hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
            {
                /* Acquire object */
                object_type *o_ptr = &o_list[this_o_idx];

                /* Acquire next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Held by new monster */
                o_ptr->held_m_idx = hack_m_idx_ii;
            }
        }
        else if (back_m.hold_o_idx) /* Failed (paranoia) */
        {
            /* Delete objects */
            for (this_o_idx = back_m.hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
            {
                /* Acquire next object */
                next_o_idx = o_list[this_o_idx].next_o_idx;

                /* Delete the object */
                delete_object_idx(this_o_idx);
            }
        }

        if (targeted) target_who = hack_m_idx_ii;
        if (health_tracked) health_track(hack_m_idx_ii);
    }

    return polymorphed;
}


/*
 * Dimension Door
 */
bool dimension_door_aux(int x, int y, int rng)
{
    int    plev = p_ptr->lev;

    if (!mut_present(MUT_ASTRAL_GUIDE))
        p_ptr->energy_need += (s16b)((s32b)(60 - plev) * ENERGY_NEED() / 100L);

    if (p_ptr->wizard)
    {
        teleport_player_to(y, x, 0L);
        return TRUE;
    }
    else if ( !cave_player_teleportable_bold(y, x, 0L)
           || distance(y, x, py, px) > rng
           || !randint0(plev / 10 + 10) )
    {
        if (!mut_present(MUT_ASTRAL_GUIDE))
            p_ptr->energy_need += (s16b)((s32b)(60 - plev) * ENERGY_NEED() / 100L);
        teleport_player((plev + 2) * 2, TELEPORT_PASSIVE);
        return FALSE;
    }
    else
    {
        teleport_player_to(y, x, 0L);
        return TRUE;
    }
}


/*
 * Dimension Door
 */
bool dimension_door(int rng)
{
    int x = 0, y = 0;

    /* Rerutn FALSE if cancelled */
    if (!tgt_pt(&x, &y, rng)) return FALSE;

    if (dimension_door_aux(x, y, rng)) return TRUE;

    if (p_ptr->pclass == CLASS_TIME_LORD)
        msg_print("You fail to exit the temporal plane correctly!");
    else
        msg_print("You fail to exit the astral plane correctly!");

    return TRUE;
}


bool eat_magic(int power)
{
    object_type * o_ptr;
    int item, amt;
    int fail_odds = 0, lev;

    if (p_ptr->pclass == CLASS_RUNE_KNIGHT)
    {
        msg_print("You are not allowed to Eat Magic!");
        return FALSE;
    }

    /* Get an item */
    item_tester_hook = _obj_recharge_src;
    if (!get_item(&item, "Drain which item? ", "You have nothing to drain.", USE_INVEN | USE_FLOOR))
        return FALSE;

    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];

    amt = o_ptr->activation.difficulty;
    if (amt > device_sp(o_ptr))
        amt = device_sp(o_ptr);

    lev = o_ptr->activation.difficulty;
    if (power > lev/2)
        fail_odds = (power - lev/2) / 5;

    if (one_in_(fail_odds))
    {
        char name[MAX_NLEN];
        bool drain = FALSE;

        object_desc(name, o_ptr, OD_OMIT_PREFIX);

        if (object_is_fixed_artifact(o_ptr) || !one_in_(10))
            drain = TRUE;

        if (drain)
        {
            msg_format("Failed! Your %s is completely drained.", name);
            device_decrease_sp(o_ptr, device_sp(o_ptr));
        }
        else
        {
            msg_format("Failed! Your %s is destroyed.", name);
            if (item >= 0)
            {
                inven_item_increase(item, -1);
                inven_item_describe(item);
                inven_item_optimize(item);
            }

            /* Reduce and describe floor item */
            else
            {
                floor_item_increase(0 - item, -1);
                floor_item_describe(0 - item);
                floor_item_optimize(0 - item);
            }
        }
    }
    else
    {
        device_decrease_sp(o_ptr, amt);
        sp_player(amt);
    }

    p_ptr->redraw |= PR_MANA;
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);
    p_ptr->window |= PW_INVEN;
    return TRUE;
}


bool summon_kin_player(int level, int y, int x, u32b mode)
{
    bool pet = (bool)(mode & PM_FORCE_PET);
    if (!pet) mode |= PM_NO_PET;

    switch (p_ptr->mimic_form)
    {
    case MIMIC_NONE:
        switch (p_ptr->prace)
        {
            case RACE_HUMAN:
            case RACE_AMBERITE:
            case RACE_BARBARIAN:
            case RACE_BEASTMAN:
            case RACE_DUNADAN:
            case RACE_DEMIGOD:
                summon_kin_type = 'p';
                break;
            case RACE_TONBERRY:
            case RACE_HOBBIT:
            case RACE_GNOME:
            case RACE_DWARF:
            case RACE_HIGH_ELF:
            case RACE_NIBELUNG:
            case RACE_DARK_ELF:
            case RACE_MIND_FLAYER:
            case RACE_KUTAR:
            case RACE_SHADOW_FAIRY:
                summon_kin_type = 'h';
                break;
            case RACE_SNOTLING:
                summon_kin_type = 'o';
                break;
            case RACE_HALF_TROLL:
                summon_kin_type = 'T';
                break;
            case RACE_HALF_OGRE:
                summon_kin_type = 'O';
                break;
            case RACE_HALF_GIANT:
            case RACE_HALF_TITAN:
            case RACE_CYCLOPS:
                summon_kin_type = 'P';
                break;
            case RACE_YEEK:
                summon_kin_type = 'y';
                break;
            case RACE_KLACKON:
                summon_kin_type = 'K';
                break;
            case RACE_KOBOLD:
                summon_kin_type = 'k';
                break;
            case RACE_IMP:
                if (one_in_(13)) summon_kin_type = 'U';
                else summon_kin_type = 'u';
                break;
            case RACE_DRACONIAN:
                summon_kin_type = 'd';
                if (p_ptr->lev >= 40)
                    summon_kin_type = 'D';
                break;
            case RACE_GOLEM:
            case RACE_ANDROID:
                summon_kin_type = 'g';
                break;
            case RACE_SKELETON:
                if (one_in_(13)) summon_kin_type = 'L';
                else summon_kin_type = 's';
                break;
            case RACE_ZOMBIE:
                summon_kin_type = 'z';
                break;
            case RACE_VAMPIRE:
                summon_kin_type = 'V';
                break;
            case RACE_SPECTRE:
                summon_kin_type = 'G';
                break;
            case RACE_SPRITE:
                summon_kin_type = 'I';
                break;
            case RACE_ENT:
                summon_kin_type = '#';
                break;
            case RACE_ARCHON:
                summon_kin_type = 'A';
                break;
            case RACE_BALROG:
                summon_kin_type = 'U';
                break;
            default:
                summon_kin_type = 'p';
                break;
        }
        break;
    case MIMIC_DEMON:
        if (one_in_(13)) summon_kin_type = 'U';
        else summon_kin_type = 'u';
        break;
    case MIMIC_DEMON_LORD:
        summon_kin_type = 'U';
        break;
    case MIMIC_VAMPIRE:
        summon_kin_type = 'V';
        break;
    case MIMIC_CLAY_GOLEM:
    case MIMIC_IRON_GOLEM:
    case MIMIC_MITHRIL_GOLEM:
    case MIMIC_COLOSSUS:
        summon_kin_type = 'g';
        break;
    }

    if (warlock_is_(WARLOCK_GIANTS))
        summon_kin_type = 'P';

    if (p_ptr->current_r_idx)
        summon_kin_type = r_info[p_ptr->current_r_idx].d_char;

    return summon_specific((pet ? -1 : 0), y, x, level, SUMMON_KIN, mode);
}
