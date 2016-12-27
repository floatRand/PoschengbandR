#include "angband.h"

#include <assert.h>

/************************************************************************
 * The Skill System
 ***********************************************************************/
#define _MAX_SKILLS    15
#define _MARTIAL_ARTS 300
#define _ARCHERY      301
#define _THROWING     302

enum _type_e {
    _TYPE_NONE = 0,
    _TYPE_MELEE = 1,
    _TYPE_SHOOT = 2,
    _TYPE_MAGIC = 3,
    _TYPE_PRAYER = 4,
    _TYPE_SKILLS = 5,
    _TYPE_TECHNIQUE = 6,
    _TYPE_ABILITY = 7,
};

typedef struct _skill_s {
    int  type;
    int  subtype;
    cptr name;
    int  max;
    int  current;
} _skill_t, *_skill_ptr;

typedef struct _group_s {
    int  type;
    cptr name;
    int  max;
    cptr desc;
    _skill_t skills[_MAX_SKILLS];
} _group_t, *_group_ptr;

static _group_t _groups[] = {
    { _TYPE_MELEE, "Melee", 10,
        "Melee skills increase your ability to fight monsters in hand to hand combat. "
        "The total number of points spent in this group will determine your base fighting "
        "skill, as well as how much melee skill you gain with experience. In addition, this "
        "total gives bonuses to STR and DEX. The individual skills determine your base "
        "proficiency with the given type of weapon, as well as the maximum number of blows "
        "per round you may gain. For martial arts, the number of points determines your "
        "effective fighting level which is used to determine which types of hits you land.",
        {{ _TYPE_MELEE, TV_SWORD, "Swords", 5, 0 },
         { _TYPE_MELEE, TV_POLEARM, "Polearms", 5, 0 },
         { _TYPE_MELEE, TV_HAFTED, "Hafted", 5, 0 },
         { _TYPE_MELEE, TV_DIGGING, "Diggers", 5, 0 },
         { _TYPE_MELEE, _MARTIAL_ARTS, "Martial Arts", 5, 0 },
         { 0 }}},
    { _TYPE_SHOOT, "Ranged", 10,
        "Ranged skills increase your ability to fight monsters using distance weapons such as bows and slings, as "
        "well as the ability to throw melee weapons effectively. The total number of points spent in this group will "
        "determine your base shooting skill, as well as how fast this skill increases with experience. Archery grants "
        "proficiency with the various classes of shooters as well as increasing your number of shots per round.",
        {{ _TYPE_SHOOT, _ARCHERY, "Archery", 5, 0 },
         { _TYPE_SHOOT, _THROWING, "Throwing", 5, 0 },
         { 0 }}},
    { 0 }
};

static _group_ptr _get_group(int id)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!_groups[i].type) return NULL;
        if (_groups[i].type == id) return &_groups[i];
    }
}

static _skill_ptr _get_skill_aux(_group_ptr g, int subtype)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!g->skills[i].type) return NULL;
        if (g->skills[i].subtype == subtype) return &g->skills[i];
    }
}

static _skill_ptr _get_skill(int type, int subtype)
{
    _group_ptr g = _get_group(type);
    assert(g);
    return _get_skill_aux(g, subtype);
}

static int _get_group_pts_aux(_group_ptr g)
{
    int i, ct = 0;
    for (i = 0; i < _MAX_SKILLS; i++)
    {
        if (!g->skills[i].type) break;
        ct += g->skills[i].current;
    }
    return ct;
}

static int _get_group_pts(int id)
{
    _group_ptr g = _get_group(id);
    assert(g);
    return _get_group_pts_aux(g);
}

static int _get_pts(void)
{
    int i, ct = 0;
    for (i = 0; ; i++)
    {
        _group_ptr g = &_groups[i];
        if (!g->type) break;
        ct += _get_group_pts_aux(g);
    }
    return ct;
}

static int _get_max_pts(void)
{
    return 5 + p_ptr->lev/5;
}

static int _get_free_pts(void)
{
    return MAX(0, _get_max_pts() - _get_pts());
}

int skillmaster_new_skills(void)
{
    return _get_free_pts();
}

static int _get_skill_pts(int type, int subtype)
{
    _skill_ptr s = _get_skill(type, subtype);
    assert(s);
    return s->current;
}

static void _reset_group(_group_ptr g)
{
    int i;
    for (i = 0; ; i++)
    {
        _skill_ptr s = &g->skills[i];
        if (!s->type) break;
        s->current = 0;
    }
}

static void _reset_groups(void)
{
    int i;
    for (i = 0; ; i++)
    {
        _group_ptr g = &_groups[i];
        if (!g->type) break;
        _reset_group(g);
    }
}

static void _load_group(savefile_ptr file,_group_ptr g)
{
    for (;;)
    {
        int subtype = savefile_read_u16b(file);
        if (subtype == 0xFFFE) break;
        _get_skill_aux(g, subtype)->current = savefile_read_u16b(file);
    }
}

static void _load_player(savefile_ptr file)
{
    _reset_groups();

    for (;;)
    {
        int type = savefile_read_u16b(file);
        if (type == 0xFFFF) break;
        _load_group(file, _get_group(type));
    }
}

static void _save_player(savefile_ptr file)
{
    int i, j;
    for (i = 0; ; i++)
    {
        _group_ptr g = &_groups[i];
        if (!g->type) break;
        savefile_write_u16b(file, g->type);
        for (j = 0; ; j++)
        {
            _skill_ptr s = &g->skills[j];
            if (!s->type) break;
            savefile_write_u16b(file, s->subtype);
            savefile_write_u16b(file, s->current);
        }
        savefile_write_u16b(file, 0xFFFE);
    }
    savefile_write_u16b(file, 0xFFFF);
}

/************************************************************************
 * User Interface
 ***********************************************************************/
static doc_ptr _doc = NULL;

static int _inkey(void)
{
    return inkey_special(TRUE);
}

static void _sync_term(doc_ptr doc)
{
    rect_t r = ui_screen_rect();
    Term_load();
    doc_sync_term(doc, doc_range_top_lines(doc, r.cy), doc_pos_create(r.x, r.y));
}

static int _confirm_skill_ui(_skill_ptr s)
{
    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        doc_printf(_doc, "<color:v>Warning</color>: <indent>You are about to learn the <color:B>%s</color> skill. "
                         "Are you sure you want to do this? The skills you learn are permanent choices and you "
                         "won't be able to change your mind later!</indent>\n", s->name);
        doc_newline(_doc);
        doc_insert(_doc, "<color:y>RET</color>) <color:v>Accept</color>\n");
        doc_insert(_doc, "<color:y>ESC</color>) Cancel\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '\r') return UI_OK;
    }
}

static int _gain_skill_ui(_group_ptr g)
{
    for (;;)
    {
        int cmd, i, ct = 0;
        int free_pts = _get_free_pts();
        int group_pts = _get_group_pts_aux(g);

        doc_clear(_doc);
        doc_printf(_doc, "<color:B>%-10.10s</color>: <indent>%s</indent>\n\n", g->name, g->desc);
        doc_printf(_doc, "%-10.10s: <color:%c>%d</color>\n", "Learned", group_pts ? 'G' : 'r', group_pts);
        doc_printf(_doc, "%-10.10s: <color:%c>%d</color>\n", "Available", free_pts ? 'G' : 'r', free_pts);
        doc_newline(_doc);
        doc_insert(_doc, "<color:G>Choose the Skill</color>\n");
        for (i = 0; ; i++, ct++)
        {
            _skill_ptr s = &g->skills[i];
            if (!s->type) break;
            doc_printf(_doc, "  <color:%c>%c</color>) %s",
                free_pts > 0 && s->current < s->max ? 'y' : 'D',
                I2A(i),
                s->name
            );
            if (s->current && s->max)
                doc_printf(_doc, " (%d%%)", 100*s->current/s->max);
            doc_newline(_doc);
        }
        doc_newline(_doc);
        doc_insert(_doc, "  <color:y>?</color>) Help\n");
        doc_insert(_doc, "<color:y>ESC</color>) Cancel\n");

        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '?') doc_display_help("Skillmaster.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < ct)
            {
                _skill_ptr s = &g->skills[i];
                doc_display_help("Skillmaster.txt", s->name);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < ct)
            {
                _skill_ptr s = &g->skills[i];
                if (free_pts > 0 && s->current < s->max && _confirm_skill_ui(s) == UI_OK)
                {
                    s->current++;
                    return UI_OK;
                }
            }
        }
    }
}

static int _gain_type_ui(void)
{
    for (;;)
    {
        int cmd, i, pts, ct = 0;
        int free_pts = _get_free_pts();
        int learned_pts = _get_pts();

        doc_clear(_doc);
        doc_printf(_doc, "<color:B>%-10.10s</color>: <indent>%s</indent>\n\n", "Skills",
            "As a Skillmaster, you must choose which skills to learn. The various skills are organized "
            "into groups. Often, the total number of points in a given group determines broad level "
            "skills and perhaps grants boosts to your starting stats, while the specific skill points "
            "grant targetted benefits (such as proficiency with a class of weapons, or access to "
            "a specific spell realm). Details are given in the submenu for each group. Feel free to "
            "explore the various groups since you may always <color:keypress>ESC</color> to return here.");
        doc_printf(_doc, "%-10.10s: <color:%c>%d</color>\n", "Learned", learned_pts ? 'G' : 'r', learned_pts);
        doc_printf(_doc, "%-10.10s: <color:%c>%d</color>\n", "Available", free_pts ? 'G' : 'r', free_pts);
        doc_newline(_doc);
        doc_insert(_doc, "<color:G>Choose the Skill Type</color>\n");
        for (i = 0; ; i++, ct++)
        {
            _group_ptr g = &_groups[i];
            if (!g->type) break;
            pts = _get_group_pts_aux(g);
            doc_printf(_doc, "  <color:y>%c</color>) %s", I2A(i), g->name);
            if (pts && g->max)
                doc_printf(_doc, " (%d%%)", 100*pts/g->max);
            doc_newline(_doc);
        }
        doc_newline(_doc);
        doc_insert(_doc, "  <color:y>?</color>) Help\n");
        doc_insert(_doc, "<color:y>ESC</color>) <color:v>Quit</color>\n");


        /*doc_newline(_doc);
        doc_insert(_doc, "<color:G>Tip:</color> <indent>You can often get specific "
                         "help by entering the uppercase letter for a command. For "
                         "example, type <color:keypress>A</color> on this screen "
                         "to receive help on Melee Skills.</indent>");*/
        _sync_term(_doc);
        cmd = _inkey();
        if (cmd == ESCAPE) return UI_CANCEL;
        else if (cmd == '?') doc_display_help("Skillmaster.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < ct)
            {
                _group_ptr g = &_groups[i];
                doc_display_help("Skillmaster.txt", g->name);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < ct)
            {
                _group_ptr g = &_groups[i];
                if (_gain_skill_ui(g) == UI_OK) return UI_OK;
            }
        }
    }
}

void skillmaster_gain_skill(void)
{
    assert(!_doc);
    _doc = doc_alloc(80);

    msg_line_clear();
    Term_save();
    if (_gain_type_ui() == UI_OK)
    {
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_EFFECTS;
    }
    Term_load();

    doc_free(_doc);
    _doc = NULL;
}

/************************************************************************
 * Melee Skills
 ***********************************************************************/
static void _melee_init_class(class_t *class_ptr)
{
    typedef struct { int base_thn; int xtra_thn; int str; int dex; } _melee_skill_t;
    static _melee_skill_t _tbl[11] = {
        { 34,  6, 0, 0 },
        { 34, 10, 1, 0 },
        { 35, 12, 1, 0 },
        { 48, 13, 1, 1 },
        { 50, 14, 2, 1 },
        { 52, 15, 2, 1 },

        { 56, 18, 2, 2 },
        { 64, 18, 3, 2 },
        { 70, 23, 3, 2 },
        { 70, 25, 3, 2 },
        { 70, 30, 4, 2 }
    };
    int pts = MIN(10, _get_group_pts(_TYPE_MELEE));
    _melee_skill_t row = _tbl[pts];
    class_ptr->base_skills.thn = row.base_thn;
    class_ptr->extra_skills.thn = row.xtra_thn;
    class_ptr->stats[A_STR] += row.str;
    class_ptr->stats[A_DEX] += row.dex;
}

typedef struct { int to_h; int to_d; int prof; int blows; int ma_wgt; } _melee_info_t;
static _melee_info_t _melee_info[6] = {
    {  0,  0, 2000, 400,   0 },
    {  0,  0, 4000, 500,  60 },
    {  0,  0, 6000, 525,  70 },
    {  5,  0, 7000, 550,  80 },
    { 10,  3, 8000, 575,  90 },
    { 20, 10, 8000, 600, 100 }
};

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int           pts = _get_skill_pts(_TYPE_MELEE, o_ptr->tval);
    _melee_info_t info;

    assert(0 <= pts && pts <= 5);
    info = _melee_info[pts];

    info_ptr->to_h += info.to_h;
    info_ptr->dis_to_h += info.to_h;

    info_ptr->to_d += info.to_d;
    info_ptr->dis_to_d += info.to_d;
}

int skillmaster_get_max_blows(object_type *o_ptr)
{
    int pts = _get_skill_pts(_TYPE_MELEE, o_ptr->tval);
    assert(0 <= pts && pts <= 5);
    return _melee_info[pts].blows;
}

int skillmaster_weapon_prof(int tval)
{
    int pts = _get_skill_pts(_TYPE_MELEE, tval);
    assert(0 <= pts && pts <= 5);
    return _melee_info[pts].prof;
}

int skillmaster_martial_arts_prof(void)
{
    int pts = _get_skill_pts(_TYPE_MELEE, _MARTIAL_ARTS);
    assert(0 <= pts && pts <= 5);
    if (!pts)
        return 0; /* rather than 2000 which would allow the player to attempt bare-handed combat */
    return _melee_info[pts].prof;
}

static void _melee_calc_bonuses(void)
{
    int pts = _get_skill_pts(_TYPE_MELEE, _MARTIAL_ARTS);
    assert(0 <= pts && pts <= 5);
    p_ptr->monk_lvl = (p_ptr->lev * _melee_info[pts].ma_wgt + 50) / 100;
}

/************************************************************************
 * Shoot Skills
 ***********************************************************************/
static void _shoot_init_class(class_t *class_ptr)
{
    typedef struct { int base_thb; int xtra_thb; } _shoot_skill_t;
    static _shoot_skill_t _tbl[6] = {
        { 20, 11 },
        { 30, 15 },
        { 50, 20 },
        { 55, 25 },
        { 55, 30 },
        { 72, 28 }
    };
    int pts = MIN(5, _get_group_pts(_TYPE_SHOOT));
    _shoot_skill_t row = _tbl[pts];
    class_ptr->base_skills.thb = row.base_thb;
    class_ptr->extra_skills.thb = row.xtra_thb;
}

typedef struct { int to_h; int to_d; int prof; int shots; } _shoot_info_t;
static _shoot_info_t _shoot_info[6] = {
    {  0,  0, 2000,   0 },

    {  0,  0, 4000,   0 },
    {  1,  0, 6000,  25 },
    {  3,  0, 7000,  50 },
    {  5,  2, 8000,  75 },
    { 10,  5, 8000, 100 }
};

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if ( !p_ptr->shooter_info.heavy_shoot
      && info_ptr->tval_ammo <= TV_BOLT
      && info_ptr->tval_ammo >= TV_SHOT )
    {
        int pts = _get_skill_pts(_TYPE_SHOOT, _ARCHERY);
        _shoot_info_t row;
        assert(0 <= pts && pts <= 5);
        row = _shoot_info[pts];
        p_ptr->shooter_info.to_h += row.to_h;
        p_ptr->shooter_info.dis_to_h += row.to_h;
        p_ptr->shooter_info.to_d += row.to_d;
        p_ptr->shooter_info.dis_to_d += row.to_d;
        p_ptr->shooter_info.num_fire += row.shots;
    }
}

int skillmaster_bow_prof(void)
{
    int pts = _get_skill_pts(_TYPE_SHOOT, _ARCHERY);
    assert(0 <= pts && pts <= 5);
    return _shoot_info[pts].prof;
}

/************************************************************************
 * Techniques
 ***********************************************************************/
int skillmaster_riding_prof(void)
{
    return 0;
}

int skillmaster_dual_wielding_prof(void)
{
    return 0;
}

/************************************************************************
 * Class
 ***********************************************************************/
static void _birth(void)
{
    _reset_groups();
    p_ptr->au += 500; /* build your own class, so buy your own gear! */
}

static void _gain_level(int new_lvl)
{
    if (new_lvl % 5 == 0)
        p_ptr->redraw |= PR_EFFECTS;
}

static void _calc_bonuses(void)
{
    _melee_calc_bonuses();
}

class_t *skillmaster_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;
    int i;

    if (!init)
    {
        me.name = "Skillmaster";
        me.desc = "The Skillmaster is not your ordinary class. Instead, you "
                  "may design your own class, on the fly, using a point based "
                  "skill system. Upon birth, you will get 5 points to spend "
                  "and you should use these wisely to set the basic direction "
                  "of your class. Upon gaining every fifth level, you will "
                  "receive an additional point to spend, for fifteen points "
                  "overall. You may use these points to learn melee or to "
                  "gain access to spell realms. You can improve your speed or "
                  "your stealth. Or you may gain increased magic device skills. "
                  "In addition, you may learn riding skills, dual wielding or "
                  "even martial arts. You will have a lot of flexibility in "
                  "how you choose to play this class.\n \n"
                  "Most skills allow the investment of multiple points for "
                  "increased proficiency, but some are abilities that you may "
                  "buy with a single point (e.g. Luck). This class is not recommended for "
                  "beginning or immediate players. You only have a limited "
                  "amount of points to spend, and your choices are irreversible.";

        me.exp = 130;

        me.birth = _birth;
        me.gain_level = _gain_level;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        /*me.get_powers = _get_powers;
        me.get_flags = _get_flags;*/
        me.load_player = _load_player;
        me.save_player = _save_player;
        init = TRUE;
    }
    /* Reset Stats and Skills on Each Call. Having these things
     * calculated here, rather than in calc_bonuses, enhances the
     * flavor of "building your own class" */
    for (i = 0; i < MAX_STATS; i++)
        me.stats[i] = 0;

    skills_init(&me.base_skills);
    skills_init(&me.extra_skills);

    me.life = 100;
    me.base_hp = 10;
    me.pets = 40;

    /* Rebuild the class_t, using the current skill allocation */
    if (!spoiler_hack && !birth_hack)
    {
        _melee_init_class(&me);
        _shoot_init_class(&me);
    }

    return &me;
}

