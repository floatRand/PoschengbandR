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
{ return UI_OK;
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
        doc_insert(doc, "<style:heading><color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n");
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
}

static void _race_class_top(doc_ptr doc)
{
    doc_ptr cols[2];

    cols[0] = doc_alloc(30);
    cols[1] = doc_alloc(46);

    _race_class_header(cols[0]);
    _race_class_info(cols[1]);
    doc_insert_cols(doc, cols, 2, 1);

    doc_free(cols[0]);
    doc_free(cols[1]);
}

/************************************************************************
 * 2.1) Race
 ***********************************************************************/ 
/************************************************************************
 * 2.2) Class
 ***********************************************************************/ 
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
        vec_add(v, get_personality_aux(i));
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
            /*races_birth(_doc);*/
            break;
        case 'c':
            /*classes_birth(_doc);*/
            break;
        case 'p':
            _pers_ui();
            break;
        case 's':
            if (p_ptr->psex == SEX_MALE) p_ptr->psex = SEX_FEMALE;
            else p_ptr->psex = SEX_MALE;
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

