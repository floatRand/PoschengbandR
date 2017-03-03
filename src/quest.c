#include "angband.h"

#include <assert.h>

static int_map_ptr _quests = NULL;
static int         _current = 0;

void quest_complete(quest_ptr q, point_t p)
{
    assert(q);
    q->status = QS_COMPLETED;
    q->completed_lev = p_ptr->lev;

    virtue_add(VIRTUE_VALOUR, 2);
    p_ptr->fame += randint1(2);
    if (q->id == QUEST_OBERON || q->id == QUEST_SERPENT)
        p_ptr->fame += 50;

    if (!(q->flags & QF_NO_MSG))
        cmsg_print(TERM_L_BLUE, "You just completed your quest!");

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

        _current = 0;
        p_ptr->inside_quest = 0;
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
    msg_add_tiny_screenshot(50, 24);
    p_ptr->redraw |= PR_DEPTH;
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
