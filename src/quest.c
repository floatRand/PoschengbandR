#include "angband.h"
#include "grid.h"

#include <assert.h>

static cptr _strcpy(cptr s)
{
    char *r = malloc(strlen(s)+1);
    strcpy(r, s);
    return r;
}

/************************************************************************
 * Quest
 ***********************************************************************/
quest_ptr quest_alloc(cptr name)
{
    quest_ptr q = malloc(sizeof(quest_t));
    memset(q, 0, sizeof(quest_t));
    assert(name);
    q->name = _strcpy(name);
    return q;
}

void quest_free(quest_ptr q)
{
    if (q)
    {
        free((vptr)q->name);
        if (q->file) free((vptr)q->file);
        free(q);
    }
}

void quest_change_file(quest_ptr q, cptr file)
{
    assert(q);
    if (q->file) free((vptr)q->file);
    if (file) q->file = _strcpy(file);
    else q->file = NULL;
}

/************************************************************************
 * Quest Status
 ***********************************************************************/
void quest_take(quest_ptr q)
{
    string_ptr s;
    assert(q);
    assert(q->status == QS_UNTAKEN);
    q->status = QS_TAKEN;
    q->seed = randint0(0x10000000);
    s = quest_get_description(q);
    msg_format("<color:R>%s</color> (<color:U>Level %d</color>): %s",
        q->name, q->level, string_buffer(s));
    string_free(s);
}

void quest_complete(quest_ptr q, point_t p)
{
    assert(q);
    assert(q->status == QS_IN_PROGRESS);
    q->status = QS_COMPLETED;
    q->completed_lev = p_ptr->lev;

    virtue_add(VIRTUE_VALOUR, 2);
    p_ptr->fame += randint1(2);
    if (q->id == QUEST_OBERON || q->id == QUEST_SERPENT)
        p_ptr->fame += 50;

    if (!(q->flags & QF_NO_MSG))
        cmsg_print(TERM_L_BLUE, "You just completed your quest!");
    msg_add_tiny_screenshot(50, 24);

    /* create stairs before the reward */
    if (q->dungeon)
    {
        int x = p.x;
        int y = p.y;
        int nx,ny;

        while (cave_perma_bold(y, x) || cave[y][x].o_idx || (cave[y][x].info & CAVE_OBJECT) )
        {
            scatter(&ny, &nx, y, x, 1, 0);
            y = ny; x = nx;
        }

        cmsg_print(TERM_L_BLUE, "A magical staircase appears...");
        cave_set_feat(y, x, feat_down_stair);
        p_ptr->update |= PU_FLOW;
    }
    if (!(q->flags & QF_TOWN)) /* non-town quest get rewarded immediately */
    {
        int i, ct = dun_level/15 + 1;
        for (i = 0; i < ct; i++)
        {
            obj_t forge = {0};
            if (make_object(&forge, AM_GOOD | AM_GREAT | AM_TAILORED))
                drop_near(&forge, -1, p.y, p.x);
            else
                msg_print("Software Bug ... you missed out on your reward!");
        }
        if (no_wilderness)
            gain_chosen_stat();

        q->status = QS_FINISHED;
    }
    p_ptr->redraw |= PR_DEPTH;
}

void quest_reward(quest_ptr q)
{
    obj_ptr o = quest_get_reward(q);
    assert(q->status == QS_COMPLETED);
    if (o)
    {
        pack_carry(o);
        obj_free(o);
    }
}

void quest_fail(quest_ptr q)
{
    assert(q);
    q->status = QS_FAILED;
    q->completed_lev = p_ptr->lev;
    msg_format("You have <color:v>failed</color> the quest: <color:R>%s</color>.", q->name);
    virtue_add(VIRTUE_VALOUR, -2);
    fame_on_failure();
    if (!(q->flags & QF_TOWN))
        q->status = QS_FAILED_DONE;
}

/************************************************************************
 * Quest Info from q->file
 ***********************************************************************/
string_ptr quest_get_description(quest_ptr q)
{
    return NULL;
}

obj_ptr quest_get_reward(quest_ptr q)
{
    return NULL;
}

room_ptr quest_get_map(quest_ptr q)
{
    return NULL;
}

/************************************************************************
 * Quest Levels
 ***********************************************************************/
static void _generate(room_ptr room)
{
    transform_ptr xform = transform_alloc_room(room, size(MAX_WID, MAX_HGT));
    int           panels_x, panels_y, x, y;

    /* figure out the dungeon size ... not sure if we need the panels crap */
    panels_y = xform->dest.cy / SCREEN_HGT;
    if (xform->dest.cy % SCREEN_HGT) panels_y++;
    cur_hgt = panels_y * SCREEN_HGT;

    panels_x = (xform->dest.cx / SCREEN_WID);
    if (xform->dest.cx % SCREEN_WID) panels_x++;
    cur_wid = panels_x * SCREEN_WID;

    /* Start with perm walls */
    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
            place_solid_perm_bold(y, x);
        }
    }

    /* generate the level */
    get_mon_num_prep(get_monster_hook(), NULL);
    build_room_template_aux(room, xform, wild_scroll);
    transform_free(xform);
    wild_scroll = NULL;
}
void quest_generate(quest_ptr q)
{
    room_ptr room;
    assert(q);
    assert(q->flags & QF_GENERATE);
    room = quest_get_map(q);
    assert(room);
    assert(vec_length(room->map));

    base_level = q->level;
    dun_level = base_level;
    object_level = base_level;
    monster_level = base_level;

    _generate(room);
    room_free(room);
}

void quest_analyze(quest_ptr q)
{
}

bool quest_post_generate(quest_ptr q)
{
    assert(q);
    assert(q->status == QS_IN_PROGRESS);
    if (q->goal == QG_KILL_MON)
    {
        monster_race *r_ptr = &r_info[q->goal_idx];
        int           mode = PM_NO_KAGE | PM_NO_PET, i, j, k;
        int           ct = q->goal_count - q->goal_current;

        if ((r_ptr->flags1 & RF1_UNIQUE) && r_ptr->cur_num >= r_ptr->max_num)
        {
            q->status = QS_FAILED_DONE; /* XXX */
            return TRUE;
        }

        if (!(r_ptr->flags1 & RF1_FRIENDS))
            mode |= PM_ALLOW_GROUP; /* allow escorts but not friends */

        for (i = 0; i < ct; i++)
        {
            for (j = 1000; j > 0; j--)
            {
                int x = 0, y = 0;

                /* Find an empty grid */
                for (k = 1000; k > 0; k--)
                {
                    cave_type    *c_ptr;
                    feature_type *f_ptr;

                    y = randint0(cur_hgt);
                    x = randint0(cur_wid);

                    c_ptr = &cave[y][x];
                    f_ptr = &f_info[c_ptr->feat];

                    if (!have_flag(f_ptr->flags, FF_MOVE) && !have_flag(f_ptr->flags, FF_CAN_FLY)) continue;
                    if (!monster_can_enter(y, x, r_ptr, 0)) continue;
                    if (distance(y, x, py, px) < 10) continue;
                    if (c_ptr->info & CAVE_ICKY) continue;
                    else break;
                }

                /* No empty grids */
                if (!k) return FALSE;

                if (place_monster_aux(0, y, x, q->goal_idx, mode)) break;
            }

            /* Failed to place */
            if (!j) return FALSE;
        }
    }
    return TRUE;
}

/************************************************************************
 * Quests (cf q_info.txt)
 ***********************************************************************/
static int_map_ptr _quests = NULL;
static int         _current = 0;

bool quests_init(void)
{
    assert(!_quests);
    _quests = int_map_alloc((int_map_free_f)quest_free);
    /* XXX Parse q_info.txt */
    return TRUE;
}

void quests_add(quest_ptr q)
{
    assert(q);
    assert(q->id);
    int_map_add(_quests, q->id, q);
}

void quests_cleanup(void)
{
    int_map_free(_quests);
    _quests = NULL;
    _current = 0;
}

quest_ptr quests_get_current(void)
{
    if (!_current) return NULL;
    return int_map_find(_quests, _current);
}

quest_ptr quests_get(int id)
{
    return int_map_find(_quests, id);
}

cptr quests_get_name(int id)
{
    quest_ptr q = quests_get(id);
    if (!q) return NULL;
    return q->name;
}

static int _quest_cmp_level(quest_ptr l, quest_ptr r)
{
    if (l->level < r->level) return -1;
    if (l->level > r->level) return 1;
    if (l->id < r->id) return -1;
    if (l->id > r->id) return 1;
    return 0;
}

static vec_ptr _quests_get(quest_p p)
{
    vec_ptr v = vec_alloc(NULL);
    int_map_iter_ptr i;

    for (i = int_map_iter_alloc(_quests);
            int_map_iter_is_valid(i);
            int_map_iter_next(i))
    {
        quest_ptr q = int_map_iter_current(i);
        if (!p || p(q))
            vec_add(v, q);
    }
    int_map_iter_free(i);

    vec_sort(v, (vec_cmp_f)_quest_cmp_level);
    return v;
}


static bool _is_active(quest_ptr q) { return q->status == QS_TAKEN || q->status == QS_IN_PROGRESS || q->status == QS_COMPLETED; }
static bool _is_finished(quest_ptr q) { return q->status == QS_FINISHED; }
static bool _is_failed(quest_ptr q) { return q->status == QS_FAILED || q->status == QS_FAILED_DONE; }
static bool _is_hidden(quest_ptr q) { return (q->flags & QF_RANDOM) && q->status == QS_UNTAKEN; }
static bool _is_random(quest_ptr q) { return BOOL(q->flags & QF_RANDOM); }

vec_ptr quests_get_all(void) { return _quests_get(NULL); }
vec_ptr quests_get_active(void) { return _quests_get(_is_active); }
vec_ptr quests_get_finished(void) { return _quests_get(_is_finished); }
vec_ptr quests_get_failed(void) { return _quests_get(_is_failed); }
vec_ptr quests_get_hidden(void) { return _quests_get(_is_hidden); }
vec_ptr quests_get_random(void) { return _quests_get(_is_random); }

/************************************************************************
 * Quests: Randomize on Birth (from birth.c with slight mods)
 ***********************************************************************/
static bool _r_can_quest(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    if (r_ptr->flags8 & RF8_WILD_ONLY) return FALSE;
    if (r_ptr->flags7 & RF7_AQUATIC) return FALSE;
    if (r_ptr->flags2 & RF2_MULTIPLY) return FALSE;
    if (r_ptr->flags7 & RF7_FRIENDLY) return FALSE;
    return TRUE;
}

static bool _r_is_unique(int r_idx) { return BOOL(r_info[r_idx].flags1 & RF1_UNIQUE); }
static bool _r_is_nonunique(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];
    if (r_ptr->flags1 & RF1_UNIQUE) return FALSE;
    if (r_ptr->flags7 & RF7_UNIQUE2) return FALSE;
    if (r_ptr->flags7 & RF7_NAZGUL) return FALSE;
    return TRUE;
}
static void _get_questor(quest_ptr q)
{
    int           r_idx = 0;
    monster_race *r_ptr;
    int           attempt;
    bool          force_unique = FALSE;
    bool          prevent_unique = FALSE;

    /* High Level quests are stacked with uniques. Everything else
       is stacked the other way. So lets make some attempt at balance.
       Of course, users can force all quests to be for uniques, in
       true Hengband spirit. */
    if (quest_unique || one_in_(3))
    {
        get_mon_num_prep(_r_can_quest, _r_is_unique);
        force_unique = TRUE;
    }
    else if (one_in_(2))
    {
        get_mon_num_prep(_r_can_quest, _r_is_nonunique);
        prevent_unique = TRUE;
    }
    else
        get_mon_num_prep(_r_can_quest, NULL);

    for(attempt = 0;; attempt++)
    {
        int accept_lev = q->level + q->level / 20;
        int mon_lev = q->level + 5 + randint1(q->level / 10);

        /* Hacks for high level quests */
        if (accept_lev > 88)
            accept_lev = 88;

        if (mon_lev > 88)
            mon_lev = 88;

        unique_count = 0; /* Hack: get_mon_num assume level generation and restricts uniques per level */
        r_idx = get_mon_num(mon_lev);
        r_ptr = &r_info[r_idx];

        if (r_idx == MON_ROBIN_HOOD) continue; /* TODO: RF?_NO_QUEST */
        if (r_idx == MON_JACK_SHADOWS) continue;

        /* Try to enforce preferences, but its virtually impossible to prevent
           high level quests for uniques */
        if (attempt < 4000)
        {
            if (prevent_unique && (r_ptr->flags1 & RF1_UNIQUE)) continue;
            if (force_unique && !(r_ptr->flags1 & RF1_UNIQUE)) continue;
        }

        if (r_ptr->flags1 & RF1_QUESTOR) continue;
        if (r_ptr->rarity > 100) continue;
        if (r_ptr->flags7 & RF7_FRIENDLY) continue;
        if (r_ptr->flags7 & RF7_AQUATIC) continue;
        if (r_ptr->flags8 & RF8_WILD_ONLY) continue;
        if (no_questor_or_bounty_uniques(r_idx)) continue;
        if (r_ptr->level > q->level + 12) continue;
        if (r_ptr->level > accept_lev || attempt > 5000)
        {
            q->goal_idx = r_idx;
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                r_ptr->flags1 |= RF1_QUESTOR;
                q->goal_count = 1;
            }
            else
                q->goal_count = rand_range(10, 20);
            q->goal_current = 0;
            break;
        }
    }
}

void quests_on_birth(void)
{
    vec_ptr v = quests_get_random();
    int i;
    for (i = 0; i < vec_length(v); i++)
    {
        quest_ptr q = vec_get(v, i);
        if (q->goal == QG_KILL_MON) _get_questor(q);
    }
    vec_free(v);

    quests_get(QUEST_OBERON)->status = QS_TAKEN;
    quests_get(QUEST_SERPENT)->status = QS_TAKEN;
}

/************************************************************************
 * Quests: Hooks
 ***********************************************************************/
void quests_on_leave_floor(void)
{
    quest_ptr q;
    if (!_current) return;
    q = quests_get(_current);
    assert(q);
    /* From floors.c ... I'm not quite sure what we are after here.
     * I suppose if the user flees an uncompleted quest for non-unique
     * monsters (e.g. Kill 16 Wargs) then, if they re-enter the quest
     * level, those quest monsters should not be present. Hey, they
     * missed their chance! It's weird that we let the uniques remain, though. */
    if (q->goal == QG_KILL_MON)
    {
        int i;
        for (i = 1; i < m_max; i++)
        {
            monster_race *r_ptr;
            monster_type *m_ptr = &m_list[i];

            /* Skip dead monsters */
            if (!m_ptr->r_idx) continue;

            /* Only maintain quest monsters */
            if (q->goal_idx != m_ptr->r_idx) continue;

            /* Extract real monster race */
            r_ptr = real_r_ptr(m_ptr);

            /* Ignore unique monsters */
            if ((r_ptr->flags1 & RF1_UNIQUE) ||
                (r_ptr->flags7 & RF7_NAZGUL)) continue;

            /* Delete non-unique quest monsters */
            delete_monster_idx(i);
        }
    }
}

static int _quest_dungeon(quest_ptr q)
{
    int d = q->dungeon;
    /* move wargs quest from 'Stronghold' to 'Angband' */
    if (d && no_wilderness)
        d = DUNGEON_ANGBAND;
    return d;
}

static bool _find_quest_p(quest_ptr q) { return q->dungeon && q->status < QS_COMPLETED; }
static quest_ptr _find_quest(int dungeon, int level)
{
    int     i;
    vec_ptr v = _quests_get(_find_quest_p);
    quest_ptr result = NULL;

    for (i = 0; i < vec_length(v); i++)
    {
        quest_ptr q = vec_get(v, i);
        int       d = _quest_dungeon(q);

        if (d == dungeon && q->level == level)
        {
            result = q;
            break;
        }
    }
    vec_clear(v);
    return result;
}

void quests_on_generate(int dungeon, int level)
{
    quest_ptr q = _find_quest(dungeon, level);
    if (q)
    {
        _current = q->id;
        q->status = QS_IN_PROGRESS;
    }
}

void quests_generate(int id)
{
    quest_ptr q = quests_get(id);
    _current = id;
    q->status = QS_IN_PROGRESS;
    quest_generate(q);
}

void quests_on_restore_floor(int dungeon, int level)
{
    quest_ptr q = _find_quest(dungeon, level);
    if (q)
    {
        _current = q->id;
        q->status = QS_IN_PROGRESS;
        quest_post_generate(q); /* replace quest monsters */
    }
}

void quests_on_kill_mon(mon_ptr mon)
{
    quest_ptr q;
    if (!_current) return;
    q = quests_get(_current);
    assert(q);
    assert(mon);
    if (q->goal == QG_KILL_MON && mon->r_idx == q->goal_idx)
    {
        q->goal_current++;
        if (q->goal_current >= q->goal_count)
            quest_complete(q, point(mon->fx, mon->fy));
    }
    else if (q->goal == QG_CLEAR_LEVEL)
    {
        int i;
        bool done = TRUE;
        if (!is_hostile(mon)) return;
        for (i = 1; i < max_m_idx && done; i++)
        {
            mon_ptr m = &m_list[i];
            if (!m->r_idx) continue;
            if (m == mon) continue;
            if (is_hostile(m)) done = FALSE;
        }
        if (done)
            quest_complete(q, point(mon->fx, mon->fy));
    }
}

void quests_on_get_obj(obj_ptr obj)
{
    quest_ptr q;
    if (!_current) return;
    q = quests_get(_current);
    assert(q);
    assert(obj);
    if (q->goal == QG_FIND_ART && (obj->name1 == q->goal_idx || obj->name3 == q->goal_idx))
        quest_complete(q, point(px, py));
}

bool quests_check_leave(void)
{
    quest_ptr q;
    if (!_current) return TRUE;
    q = quests_get(_current);
    assert(q);
    if (q->status == QS_IN_PROGRESS)
    {
        if (q->flags & QF_RETAKE)
        {
            string_ptr s = string_alloc_format(
                "<color:r>Warning,</color> you are about to leave the quest: "
                "<color:R>%s</color>. You may return to this quest later though. "
                "Are you sure you want to leave? <color:y>[Y,n]</color>", q->name);
            char c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
            string_free(s);
            if (c == 'n') return FALSE;
        }
        else
        {
            string_ptr s = string_alloc_format(
                "<color:r>Warning,</color> you are about to leave the quest: "
                "<color:R>%s</color>. <color:v>You will fail this quest if you leave!</color> "
                "Are you sure you want to leave? <color:y>[Y,n]</color>", q->name);
            char c = msg_prompt(string_buffer(s), "nY", PROMPT_YES_NO | PROMPT_CASE_SENSITIVE);
            string_free(s);
            if (c == 'n') return FALSE;
        }
    }
    return TRUE;
}

void quests_on_leave(void)
{
    quest_ptr q;
    if (!_current) return;
    q = quests_get(_current);
    assert(q);
    if (q->status == QS_IN_PROGRESS)
    {
        if (q->flags & QF_RETAKE)
            q->status = QS_TAKEN;
        else
            quest_fail(q);
    }
}

bool quests_allow_downstairs(void)
{
    return !_current;
}

bool quests_allow_downshaft(void)
{
    quest_ptr q;
    if (_current) return FALSE;
    q = _find_quest(dungeon_type, dun_level + 1);
    if (q) return FALSE;
    return TRUE;
}

bool quests_allow_all_spells(void)
{
    quest_ptr q;
    if (!_current) return TRUE;
    q = quests_get(_current);
    assert(q);
    return !(q->flags & QF_GENERATE);
}

/************************************************************************
 * Quests: Savefiles
 ***********************************************************************/
void quests_load(savefile_ptr file)
{
    int ct = savefile_read_s16b(file);
    int i;
    for (i = 0; i < ct; i++)
    {
        int       id = savefile_read_s16b(file);
        quest_ptr q = quests_get(id);

        assert(q);
        q->status = savefile_read_byte(file);
        q->completed_lev = savefile_read_byte(file);
        q->goal_current = savefile_read_s16b(file);
        if (q->flags & QF_RANDOM)
        {
            q->level = savefile_read_s16b(file);
            q->goal_idx  = savefile_read_s32b(file);
            q->goal_count = savefile_read_s16b(file);
            q->seed  = savefile_read_u32b(file);
        }

        if (q->goal == QG_FIND_ART)
            a_info[q->goal_idx].gen_flags |= OFG_QUESTITEM;
        if (q->goal == QG_KILL_MON)
        {
            monster_race *r_ptr = &r_info[q->goal_idx];
            if (r_ptr->flags1 & RF1_UNIQUE)
                r_ptr->flags1 |= RF1_QUESTOR;
        }
    }
}

void quests_save(savefile_ptr file)
{
    int i;
    vec_ptr v = quests_get_all();
    savefile_write_s16b(file, vec_length(v));
    for (i = 0; i < vec_length(v); i++)
    {
        quest_ptr q = vec_get(v, i);
        savefile_write_s16b(file, q->id);
        savefile_write_byte(file, q->status);
        savefile_write_byte(file, q->completed_lev);
        savefile_write_s16b(file, q->goal_current);
        if (q->flags & QF_RANDOM)
        {
            savefile_write_s16b(file, q->level); /* in case I randomize later ... */
            savefile_write_s32b(file, q->goal_idx);
            savefile_write_s16b(file, q->goal_count);
            savefile_write_u32b(file, q->seed);
        }
    }

    vec_clear(v);
}

