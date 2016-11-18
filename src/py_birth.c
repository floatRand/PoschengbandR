#include "angband.h"

#include <assert.h>

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

/************************************************************************
 * Final Confirmation
 ***********************************************************************/ 
static int _final_ui(void)
{
    return UI_CANCEL;
}

/************************************************************************
 * Stats
 ***********************************************************************/ 
static int _stats_ui(void)
{
    return _final_ui();
}

/************************************************************************
 * 2) Race/Class/Personality
 ***********************************************************************/ 
enum {
    _RCP_STATS,
    _RCP_SKILLS1,
    _RCP_SKILLS2,
    _RCP_COUNT
};
static int _rcp_state = _RCP_STATS;

static void _sex_line(doc_ptr doc)
{
    doc_printf(doc, "Sex   : <color:B>%s</color>\n", sex_info[p_ptr->psex].title);
}

static void _race_line(doc_ptr doc)
{
    race_t *race_ptr = get_race();
    doc_printf(doc, "Race  : <color:B>%s", race_ptr->name);
    if (race_ptr->subname)
        doc_printf(doc, " (%s)", race_ptr->subname);
    doc_insert(doc, "</color>\n");
}

static void _class_line(doc_ptr doc)
{
    class_t *class_ptr = get_class();
    doc_printf(doc, "Class : <color:B>%s", class_ptr->name);
    if (class_ptr->subname)
        doc_printf(doc, " (%s)", class_ptr->subname);
    doc_insert(doc, "</color>\n");
}

static void _pers_line(doc_ptr doc)
{
    personality_ptr pers_ptr = get_personality();
    doc_printf(doc, "Pers  : <color:B>%s</color>\n", pers_ptr->name);
}

static void _race_class_header(doc_ptr doc)
{
    _sex_line(doc);
    _race_line(doc);
    _class_line(doc);
    _pers_line(doc);
}

static void _race_class_info(doc_ptr doc)
{
    race_t *race_ptr = get_race();
    class_t *class_ptr = get_class();
    personality_ptr pers_ptr = get_personality();

    if (_rcp_state == _RCP_STATS)
    {
        doc_insert(doc, "<style:heading><color:w>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n");
        doc_printf(doc, "%+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp);
        doc_printf(doc, "%+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp);
        doc_printf(doc, "%+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%       %3d%%\n",
            pers_ptr->stats[A_STR], pers_ptr->stats[A_INT], pers_ptr->stats[A_WIS],
            pers_ptr->stats[A_DEX], pers_ptr->stats[A_CON], pers_ptr->stats[A_CHR],
            pers_ptr->life, pers_ptr->exp);
        doc_printf(doc, "<color:R>%+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%</color>\n",
            race_ptr->stats[A_STR] + class_ptr->stats[A_STR] + pers_ptr->stats[A_STR],
            race_ptr->stats[A_INT] + class_ptr->stats[A_INT] + pers_ptr->stats[A_INT],
            race_ptr->stats[A_WIS] + class_ptr->stats[A_WIS] + pers_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX] + class_ptr->stats[A_DEX] + pers_ptr->stats[A_DEX],
            race_ptr->stats[A_CON] + class_ptr->stats[A_CON] + pers_ptr->stats[A_CON],
            race_ptr->stats[A_CHR] + class_ptr->stats[A_CHR] + pers_ptr->stats[A_CHR],
            race_ptr->life * class_ptr->life * pers_ptr->life / 10000,
            race_ptr->base_hp + class_ptr->base_hp,
            race_ptr->exp * class_ptr->exp * pers_ptr->exp / 10000
        );
        doc_insert(doc, "</style>");
    }
    else if (_rcp_state == _RCP_SKILLS1 || _rcp_state == _RCP_SKILLS2)
    {
        skills_desc_t r_desc, c_desc, p_desc, tot_desc;
        skills_t      base, xtra;

        base = class_ptr->base_skills;
        skills_add(&base, &race_ptr->skills);
        skills_add(&base, &pers_ptr->skills);

        xtra = pers_ptr->skills;
        skills_scale(&xtra, 1, 5);
        skills_add(&xtra, &class_ptr->extra_skills);

        skills_desc_race(race_ptr, &r_desc);
        skills_desc_class(class_ptr, &c_desc);
        skills_desc_pers(pers_ptr, &p_desc);
        skills_desc_aux(&base, &xtra, &tot_desc);

        if (_rcp_state == _RCP_SKILLS1)
        {
            doc_printf(doc, "<color:w>%-10.10s %-10.10s %-10.10s %-10.10s</color>\n",
                "Disarming", "Device", "Save", "Stealth");
            doc_printf(doc, "%s %s %s %s\n", /* Note: descriptions contain formatting tags, so %-10.10s won't work ... cf skills.c */
                r_desc.dis, r_desc.dev, r_desc.sav, r_desc.stl);
            doc_printf(doc, "%s %s %s %s\n",
                c_desc.dis, c_desc.dev, c_desc.sav, c_desc.stl);
            doc_printf(doc, "%s %s %s %s\n",
                p_desc.dis, p_desc.dev, p_desc.sav, p_desc.stl);
            doc_printf(doc, "%s %s %s %s\n",
                tot_desc.dis, tot_desc.dev, tot_desc.sav, tot_desc.stl);
        }
        else
        {
            doc_printf(doc, "<color:w>%-10.10s %-10.10s %-10.10s %-10.10s</color>\n",
                "Searching", "Perception", "Melee", "Bows");
            doc_printf(doc, "%s %s %s %s\n",
                r_desc.srh, r_desc.fos, r_desc.thn, r_desc.thb);
            doc_printf(doc, "%s %s %s %s\n",
                c_desc.srh, c_desc.fos, c_desc.thn, c_desc.thb);
            doc_printf(doc, "%s %s %s %s\n",
                p_desc.srh, p_desc.fos, p_desc.thn, p_desc.thb);
            doc_printf(doc, "%s %s %s %s\n",
                tot_desc.srh, tot_desc.fos, tot_desc.thn, tot_desc.thb);
        }
    }
}

static void _race_class_top(doc_ptr doc)
{
    doc_ptr cols[2];

    cols[0] = doc_alloc(29);
    cols[1] = doc_alloc(50);

    _race_class_header(cols[0]);
    _race_class_info(cols[1]);
    doc_insert_cols(doc, cols, 2, 1);

    doc_free(cols[0]);
    doc_free(cols[1]);
}

/************************************************************************
 * 2.1) Race
 ***********************************************************************/ 
static bool _is_valid_race_class(int race_id, int class_id)
{
    if (class_id == CLASS_BLOOD_KNIGHT || class_id == CLASS_BLOOD_MAGE)
    {
        if (get_race_aux(race_id, 0)->flags & RACE_IS_NONLIVING)
            return FALSE;
    }
    if (class_id == CLASS_BEASTMASTER || class_id == CLASS_CAVALRY)
    {
        if (race_id == RACE_CENTAUR)
            return FALSE;
    }
    if (class_id == CLASS_DUELIST || class_id == CLASS_MAULER)
    {
        if (race_id == RACE_TONBERRY)
            return FALSE;
    }
    return TRUE;
}

static vec_ptr _get_races_aux(int ids[])
{
    vec_ptr v = vec_alloc(NULL);
    int     i;

    for (i = 0; ; i++)
    {
        int id = ids[i];
        if (id == -1) break;
        if (!_is_valid_race_class(id, p_ptr->pclass)) continue;
        vec_add(v, get_race_aux(id, 0));
    }

    return v;
}

#define _MAX_RACES_PER_GROUP 23
#define _MAX_RACE_GROUPS      8
typedef struct _race_group_s {
    cptr name;
    int ids[_MAX_RACES_PER_GROUP];
} _race_group_t, *_race_group_ptr;
static _race_group_t _race_groups[_MAX_RACE_GROUPS] = {
    { "Human",
        {RACE_AMBERITE, RACE_BARBARIAN, RACE_DEMIGOD, RACE_DUNADAN, RACE_HUMAN, -1} },
    { "Elf",
        {RACE_DARK_ELF, RACE_HIGH_ELF, RACE_WOOD_ELF, -1} },
    { "Hobbit/Dwarf",
        {RACE_DWARF, RACE_GNOME, RACE_HOBBIT, RACE_NIBELUNG, -1} },
    { "Fairy",
        {RACE_SHADOW_FAIRY, RACE_SPRITE, -1} },
    { "Angel/Demon",
        {RACE_ARCHON, RACE_BALROG, RACE_IMP, -1} },
    { "Orc/Troll/Giant",
        {RACE_CYCLOPS, RACE_HALF_GIANT, RACE_HALF_OGRE,
         RACE_HALF_TITAN, RACE_HALF_TROLL, RACE_KOBOLD, RACE_SNOTLING, -1} },
    { "Undead",
        {RACE_SKELETON, RACE_SPECTRE, RACE_VAMPIRE, RACE_ZOMBIE, -1} },
    { "Other",
        {RACE_ANDROID, RACE_BEASTMAN, RACE_CENTAUR, RACE_DRACONIAN, RACE_DOPPELGANGER, RACE_ENT,
         RACE_GOLEM, RACE_KLACKON, RACE_KUTAR, RACE_MIND_FLAYER, RACE_TONBERRY, RACE_YEEK,-1 } },
};

static int _race_ui(_race_group_ptr group)
{
    vec_ptr v = _get_races_aux(group->ids);
    int     result = UI_NONE;

    while (result == UI_NONE)
    {
        int cmd, i, split = vec_length(v);
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        /* Choices */
        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        if (split > 10)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            race_t *race_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                race_ptr->id == p_ptr->prace ? 'B' : 'w',
                race_ptr->name
            );
        }

        doc_insert(_doc, "<color:G>Choose Your Race</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE)
            result = UI_CANCEL;
        else if (cmd == '\t')
        {
            _rcp_state++;
            if (_rcp_state == _RCP_COUNT)
                _rcp_state = _RCP_STATS;
        }
        else if (cmd == '?')
            doc_display_help("Races.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                race_t *race_ptr = vec_get(v, i);
                doc_display_help("Races.txt", race_ptr->name);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                race_t *race_ptr = vec_get(v, i);
                p_ptr->prace = race_ptr->id;
                /* TODO: Subraces */
                result = UI_OK;
            }
        }
    }
    vec_free(v);
    return result;
}

static void _race_group_ui(void)
{
    vec_ptr groups = vec_alloc(NULL);
    int     i;

    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        _race_group_ptr g_ptr = &_race_groups[i];
        vec_ptr         races = _get_races_aux(g_ptr->ids);

        if (vec_length(races))
            vec_add(groups, g_ptr);
        vec_free(races);
    }

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        _race_class_top(_doc);

        /* Choices */
        doc_insert(_doc, "<color:G>Choose a Type of Race to Play</color>\n");
        for (i = 0; i < vec_length(groups); i++)
        {
            _race_group_ptr g_ptr = vec_get(groups, i);
            doc_printf( _doc, "  <color:y>%c</color>) %s\n", I2A(i), g_ptr->name);
        }
        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE)
            break;
        else if (cmd == '\t')
        {
            _rcp_state++;
            if (_rcp_state == _RCP_COUNT)
                _rcp_state = _RCP_STATS;
        }
        else if (cmd == '?')
            doc_display_help("Races.txt", NULL);
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(groups))
            {
                _race_group_ptr g_ptr = vec_get(groups, i);
                if (_race_ui(g_ptr) == UI_OK)
                    break;
            }
        }
    }
    vec_free(groups);
}

/************************************************************************
 * 2.2) Class
 ***********************************************************************/ 
static vec_ptr _get_classes_aux(int ids[])
{
    vec_ptr v = vec_alloc(NULL);
    int     i;

    for (i = 0; ; i++)
    {
        int id = ids[i];
        if (id == -1) break;
        if (!_is_valid_race_class(p_ptr->prace, id)) continue;
        vec_add(v, get_class_aux(id, 0));
    }

    return v;
}

#define _MAX_CLASSES_PER_GROUP 20
#define _MAX_CLASS_GROUPS      11
typedef struct _class_group_s {
    cptr name;
    int ids[_MAX_CLASSES_PER_GROUP];
} _class_group_t, *_class_group_ptr;
static _class_group_t _class_groups[_MAX_CLASS_GROUPS] = {
    { "Melee", {CLASS_BERSERKER, CLASS_BLOOD_KNIGHT, CLASS_DUELIST, CLASS_MAULER,
                    CLASS_RUNE_KNIGHT, CLASS_SAMURAI, CLASS_WARRIOR, CLASS_WEAPONMASTER,
                    CLASS_WEAPONSMITH, -1} },
    { "Archery", {CLASS_ARCHER, CLASS_SNIPER, -1} },
    { "Martial Arts", {CLASS_FORCETRAINER, CLASS_MONK, CLASS_MYSTIC, -1} },
    { "Magic", {CLASS_BLOOD_MAGE, CLASS_BLUE_MAGE, CLASS_GRAY_MAGE, CLASS_HIGH_MAGE, CLASS_MAGE,
                    CLASS_NECROMANCER, CLASS_SORCERER, CLASS_YELLOW_MAGE, -1} },
    { "Devices", {CLASS_DEVICEMASTER, CLASS_MAGIC_EATER, -1} },
    { "Prayer", {CLASS_PRIEST, -1} },
    { "Stealth", {CLASS_NINJA, CLASS_ROGUE, CLASS_SCOUT, -1} },
    { "Hybrid", {CLASS_CHAOS_WARRIOR, CLASS_PALADIN, CLASS_RANGER, CLASS_RED_MAGE,
                    CLASS_WARRIOR_MAGE, -1} },
    { "Riding", {CLASS_BEASTMASTER, CLASS_CAVALRY, -1} },
    { "Mind", {CLASS_MINDCRAFTER, CLASS_MIRROR_MASTER, CLASS_PSION,
                    CLASS_TIME_LORD, CLASS_WARLOCK, -1} },
    { "Other", {CLASS_ARCHAEOLOGIST, CLASS_BARD, CLASS_IMITATOR, CLASS_RAGE_MAGE,
                    CLASS_TOURIST, CLASS_WILD_TALENT, -1} },
};

static int _class_ui(_class_group_ptr group)
{
    vec_ptr v = _get_classes_aux(group->ids);
    int     result = UI_NONE;

    while (result == UI_NONE)
    {
        int cmd, i, split = vec_length(v);
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        /* Choices */
        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        if (split > 10)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            class_t *class_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                class_ptr->id == p_ptr->pclass ? 'B' : 'w',
                class_ptr->name
            );
        }

        doc_insert(_doc, "<color:G>Choose Your Class</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE)
            result = UI_CANCEL;
        else if (cmd == '\t')
        {
            _rcp_state++;
            if (_rcp_state == _RCP_COUNT)
                _rcp_state = _RCP_STATS;
        }
        else if (cmd == '?')
            doc_display_help("Classes.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                class_t *class_ptr = vec_get(v, i);
                doc_display_help("Classes.txt", class_ptr->name);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                class_t *class_ptr = vec_get(v, i);
                p_ptr->pclass = class_ptr->id;
                /* TODO: Subclass */
                /* TODO: Magic */
                result = UI_OK;
            }
        }
    }
    vec_free(v);
    return result;
}

static void _class_group_ui(void)
{
    vec_ptr groups = vec_alloc(NULL);
    int     i;

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        _class_group_ptr g_ptr = &_class_groups[i];
        vec_ptr          classes = _get_classes_aux(g_ptr->ids);

        if (vec_length(classes))
            vec_add(groups, g_ptr);
        vec_free(classes);
    }

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        _race_class_top(_doc);

        /* Choices */
        doc_insert(_doc, "<color:G>Choose a Type of Class to Play</color>\n");
        for (i = 0; i < vec_length(groups); i++)
        {
            _class_group_ptr g_ptr = vec_get(groups, i);
            doc_printf( _doc, "  <color:y>%c</color>) %s\n", I2A(i), g_ptr->name);
        }
        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE)
            break;
        else if (cmd == '\t')
        {
            _rcp_state++;
            if (_rcp_state == _RCP_COUNT)
                _rcp_state = _RCP_STATS;
        }
        else if (cmd == '?')
            doc_display_help("Classes.txt", NULL);
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(groups))
            {
                _class_group_ptr g_ptr = vec_get(groups, i);
                if (_class_ui(g_ptr) == UI_OK)
                    break;
            }
        }
    }
    vec_free(groups);
}

/************************************************************************
 * 2.3) Personality
 ***********************************************************************/ 
static int _pers_cmp(personality_ptr l, personality_ptr r)
{
    return strcmp(l->name, r->name);
}

static vec_ptr _pers_choices(void)
{
    vec_ptr v = vec_alloc(NULL);
    int i;
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (p_ptr->psex == SEX_MALE && i == PERS_SEXY) continue;
        if (p_ptr->psex == SEX_FEMALE && i == PERS_LUCKY) continue;
        vec_add(v, pers_ptr);
    }
    vec_sort(v, (vec_cmp_f)_pers_cmp);
    return v;
}

static void _pers_ui(void)
{
    vec_ptr v = _pers_choices();
    for (;;)
    {
        int cmd, i, split = vec_length(v);
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        /* Choices */
        cols[0] = doc_alloc(20);
        cols[1] = doc_alloc(20);

        if (split > 10)
            split = (split + 1)/2;
        for (i = 0; i < vec_length(v); i++)
        {
            personality_ptr pers_ptr = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "  <color:y>%c</color>) <color:%c>%s</color>\n",
                I2A(i),
                pers_ptr->id == p_ptr->personality ? 'B' : 'w',
                pers_ptr->name
            );
        }

        doc_insert(_doc, "<color:G>Choose Your Personality</color>\n");
        doc_insert_cols(_doc, cols, 2, 1);
        doc_insert(_doc, "     Use SHIFT+choice to display help topic\n");

        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();

        if (cmd == ESCAPE)
            break;
        else if (cmd == '\t')
        {
            _rcp_state++;
            if (_rcp_state == _RCP_COUNT)
                _rcp_state = _RCP_STATS;
        }
        else if (cmd == '?')
            doc_display_help("Personalities.txt", NULL);
        else if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < vec_length(v))
            {
                personality_ptr pers_ptr = vec_get(v, i);
                doc_display_help("Personalities.txt", pers_ptr->name);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                personality_ptr pers_ptr = vec_get(v, i);
                p_ptr->personality = pers_ptr->id;
                break;
            }
        }
    }
    vec_free(v);
}

static int _race_class_ui(void)
{
    for (;;)
    {
        int cmd;
        doc_ptr cols[2];

        doc_clear(_doc);
        _race_class_top(_doc);

        /* Choices */
        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(46);

        doc_insert(cols[0], "  <color:y>s</color>) Change Sex\n");
        doc_insert(cols[0], "  <color:y>r</color>) Change Race\n");
        doc_insert(cols[0], "  <color:y>c</color>) Change Class\n");
        doc_insert(cols[0], "  <color:y>p</color>) Change Personality\n");

        doc_insert(cols[1], "<color:y>  ?</color>) Help\n");
        doc_insert(cols[1], "<color:y>RET</color>) Next Screen\n");
        doc_insert(cols[1], "<color:y>ESC</color>) Prev Screen\n");

        doc_insert_cols(_doc, cols, 2, 1);
        doc_free(cols[0]);
        doc_free(cols[1]);

        _sync_term(_doc);

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            return _stats_ui();
        case ESCAPE:
            return UI_CANCEL;
        case '\t':
            _rcp_state++;
            if (_rcp_state == _RCP_COUNT)
                _rcp_state = _RCP_STATS;
            break;
        case 'r':
            _race_group_ui();
            break;
        case 'c':
            _class_group_ui();
            break;
        case 'p':
            _pers_ui();
            break;
        case 's':
            if (p_ptr->psex == SEX_MALE)
            {
                if (p_ptr->personality != PERS_LUCKY)
                    p_ptr->psex = SEX_FEMALE;
            }
            else if (p_ptr->personality != PERS_SEXY)
                p_ptr->psex = SEX_MALE;
            break;
        }
    }
}

/************************************************************************
 * Welcome to Poschengband!
 ***********************************************************************/ 
static int _welcome_ui(void)
{
    return _race_class_ui();
}

int py_birth(void)
{
    int result = UI_OK;

    assert(!_doc);
    _doc = doc_alloc(80);

    msg_line_clear();
    Term_clear();
    Term_save();
    result = _welcome_ui();
    Term_load();

    doc_free(_doc);
    _doc = NULL;

    return result;
}

