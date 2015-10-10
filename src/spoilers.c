#include "angband.h"

bool spoiler_hack = FALSE;

#ifdef ALLOW_SPOILERS

typedef void(*_file_fn)(FILE*);
static void _text_file(cptr name, _file_fn fn)
{
    FILE    *fp = NULL;
    char    buf[1024];

    path_build(buf, sizeof(buf), ANGBAND_DIR_HELP, name);
    fp = my_fopen(buf, "w");

    if (!fp)
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
        fp = my_fopen(buf, "w");

        if (!fp)
        {
            prt("Failed!", 0, 0);
            (void)inkey();
            return;
        }
    }

    fn(fp);
    fprintf(fp, "\n\n<color:s>Automatically generated for PosChengband %d.%d.%d.</color>\n",
            VER_MAJOR, VER_MINOR, VER_PATCH);

    my_fclose(fp);
    msg_format("Created %s", buf);
}

static void _race_help(FILE *fp, int idx)
{
    race_t *race_ptr = get_race_t_aux(idx, 0);

    fprintf(fp, "<topic:%s><style:heading>%s</style>\n\n", race_ptr->name, race_ptr->name);
    fprintf(fp, "%s\n", race_ptr->desc);
    switch(idx)
    {
    case RACE_DEMIGOD:
        fputs("\nSee <link:Demigods.txt> for more details on demigod parentage.\n", fp);
        break;
    case RACE_DRACONIAN:
        fputs("\nSee <link:Draconians.txt> for more details on draconians.\n", fp);
        break;
    case RACE_MON_RING:
        fputs("\nSee <link:rings.txt> for more details on rings.\n", fp);
        break;
    case RACE_MON_DRAGON:
        fputs("\nSee <link:DragonRealms.txt> for more details on dragons.\n", fp);
        break;
    }
    fputc('\n', fp);
}

static void _races_help(FILE* fp)
{
    int i, r;

    fprintf(fp, "<style:title>The Races</style>\n\n");
    for (i = 0; i < MAX_RACES; i++)
    {
        race_t *race_ptr = get_race_t_aux(i, 0);
        if (race_ptr->flags & RACE_IS_MONSTER) continue;
        /*if (race_ptr->flags & RACE_IS_DEPRECATED) continue;*/
        _race_help(fp, i);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Race Statistic Bonus Table</style>\n<style:table>\n", fp);

    for (i = 0, r = 0; i < MAX_RACES; i++)
    {
        race_t *race_ptr = get_race_t_aux(i, 0);
        if (race_ptr->flags & RACE_IS_MONSTER) continue;
        /*if (race_ptr->flags & RACE_IS_DEPRECATED) continue;*/

        if (r % 20 == 0)
            fputs("<color:U>               STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", fp);
        r++;

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->name,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS], 
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR], 
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Race Skill Bonus Table</style>\n<style:table>\n", fp);
    for (i = 0, r = 0; i < MAX_RACES; i++)
    {
        race_t *race_ptr = get_race_t_aux(i, 0);
        if (race_ptr->flags & RACE_IS_MONSTER) continue;
        /*if (race_ptr->flags & RACE_IS_DEPRECATED) continue;*/

        if (r % 20 == 0)
            fputs("<color:U>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows  Infra</color>\n", fp);
        r++;

        fprintf(fp, "%-14s %+4d  %+4d  %+4d  %+4d  %+4d  %+4d  %+5d  %+4d  %4d'\n",
            race_ptr->name,
            race_ptr->skills.dis, race_ptr->skills.dev, race_ptr->skills.sav,
            race_ptr->skills.stl, race_ptr->skills.srh, race_ptr->skills.fos,
            race_ptr->skills.thn, race_ptr->skills.thb, race_ptr->infra*10
        );
    }
    fputs("\n</style>\n", fp);
}

static void _monster_races_help(FILE* fp)
{
    int i;
    player_type old = *p_ptr;

    fprintf(fp, "<style:title>Monster Races</style>\n\n");
    for (i = 0; i < MAX_RACES; i++)
    {
        int max = 1, j;
        race_t *race_ptr = get_race_t_aux(i, 0);

        if (!(race_ptr->flags & RACE_IS_MONSTER)) continue;

        _race_help(fp, i);

        if (i == RACE_MON_SWORD) continue;
        if (i == RACE_MON_RING) continue;

        switch (i)
        {
        case RACE_MON_SPIDER: max = SPIDER_MAX; break;
        case RACE_MON_DEMON: max = DEMON_MAX; break;
        case RACE_MON_DRAGON: max = DRAGON_MAX; break;
        case RACE_MON_GIANT: max = GIANT_MAX; break;
        case RACE_MON_TROLL: max = TROLL_MAX; break;
        case RACE_MON_ELEMENTAL: max = ELEMENTAL_MAX; break;
        case RACE_MON_GOLEM: max = GOLEM_MAX; break;
        }

        for (j = 0; j < max; j++)
        {
            race_t *race_ptr;
            int     current_r_idx = 0;

            p_ptr->lev = 1;
            p_ptr->exp = 0;
            p_ptr->prace = i;
            p_ptr->psubrace = j;

            race_ptr = get_race_t();

            fprintf(fp, "<style:table><color:U>%21s STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
            if (race_ptr->birth)
                race_ptr->birth();

            for (;;)
            {
                p_ptr->lev++;
                if (race_ptr->gain_level)
                    race_ptr->gain_level(p_ptr->lev);

                if (p_ptr->current_r_idx != current_r_idx)
                {
                    race_ptr = get_race_t();

                    fprintf(fp, "%-21.21s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
                        race_ptr->subname,
                        race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS], 
                        race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR], 
                        race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
                    );

                    current_r_idx = p_ptr->current_r_idx;
                }

                if (p_ptr->lev >= PY_MAX_LEVEL)
                    break;
            }
            fputs("</style>\n", fp);

            current_r_idx = 0;
            p_ptr->lev = 1;
            p_ptr->exp = 0;
            p_ptr->prace = i;
            p_ptr->psubrace = j;

            race_ptr = get_race_t();

            fprintf(fp, "<style:table><color:U>%-21s Dsrm   Dvce   Save   Stlh  Srch  Prcp  Melee  Bows</color>\n", "");
            if (race_ptr->birth)
                race_ptr->birth();

            for (;;)
            {
                p_ptr->lev++;
                if (race_ptr->gain_level)
                    race_ptr->gain_level(p_ptr->lev);

                if (p_ptr->current_r_idx != current_r_idx)
                {
                    race_ptr = get_race_t();

                    fprintf(fp,
                        "%-21.21s %2d+%-2d  %2d+%-2d  %2d+%-2d  %2d+%-2d %2d+%-2d %2d+%-2d %2d+%-2d  %2d+%-2d\n",
                        race_ptr->subname,
                        race_ptr->skills.dis, race_ptr->extra_skills.dis, 
                        race_ptr->skills.dev, race_ptr->extra_skills.dev, 
                        race_ptr->skills.sav, race_ptr->extra_skills.sav,
                        race_ptr->skills.stl, race_ptr->extra_skills.stl,
                        race_ptr->skills.srh, race_ptr->extra_skills.srh,
                        race_ptr->skills.fos, race_ptr->extra_skills.fos,
                        race_ptr->skills.thn, race_ptr->extra_skills.thn, 
                        race_ptr->skills.thb, race_ptr->extra_skills.thb
                    );

                    current_r_idx = p_ptr->current_r_idx;
                }

                if (p_ptr->lev >= PY_MAX_LEVEL)
                    break;
            }
            fputs("</style>\n", fp);
        }
        if (i == RACE_MON_HOUND)
        {
            fputs("Evolution in each tier is actually random. All of the possible forms in each tier are not displayed.\n", fp);
        }
        fputc('\n', fp);
    }
    fputs("\n\n", fp);
    *p_ptr = old;
    do_cmd_redraw();
}

struct _name_desc_s { string_ptr name; string_ptr desc; };
typedef struct _name_desc_s _name_desc_t, *_name_desc_ptr;
static int _compare_name_desc(const _name_desc_ptr left, const _name_desc_ptr right) {
    return string_compare(left->name, right->name);
}
static void _name_desc_free(_name_desc_ptr p) {
    string_free(p->name);
    string_free(p->desc);
    free(p);
}
static _name_desc_ptr _name_desc_alloc(void) {
    _name_desc_ptr result = malloc(sizeof(_name_desc_t));
    result->name = string_alloc();
    result->desc = string_alloc();
    return result;
}

static void _demigods_help(FILE* fp)
{
    int i;

    fputs("<style:title>Demigod Parentage</style>\n\n", fp);
    for (i = 0; i < MAX_DEMIGOD_TYPES; i++)
    {
        fprintf(fp, "<color:heading>%s</color>\n\n", demigod_info[i].name);
        fputs(demigod_info[i].desc, fp);
        fputs("\n\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Demigod Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < MAX_DEMIGOD_TYPES; i++)
    {
        race_t *race_ptr = get_race_t_aux(RACE_DEMIGOD, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            demigod_info[i].name,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS], 
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR], 
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Demigod Skill Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows  Infra</color>\n", fp);
    for (i = 0; i < MAX_DEMIGOD_TYPES; i++)
    {
        race_t *race_ptr = get_race_t_aux(RACE_DEMIGOD, i);

        fprintf(fp, "%-14s %+4d  %+4d  %+4d  %+4d  %+4d  %+4d  %+5d  %+4d  %4d'\n",
            demigod_info[i].name,
            race_ptr->skills.dis, race_ptr->skills.dev, race_ptr->skills.sav,
            race_ptr->skills.stl, race_ptr->skills.srh, race_ptr->skills.fos,
            race_ptr->skills.thn, race_ptr->skills.thb, race_ptr->infra*10
        );
    }
    fputs("\n</style>\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<style:heading>Table 3 - Demigod Special Powers</style>\n\n", fp);
        fputs("All demigods have access to special powers. When they reach level 20, they may choose "
                    "a single power from the following list. When they reach level, 40, they may choose another. "
                    "These powers can never be removed or changed, so you might want to study this list to "
                    "decide which powers you will choose for your character.\n\n", fp);

        for (i = 0; i < MAX_MUTATIONS; i++)
        {
            if (mut_demigod_pred(i))
            {
                char buf[1024];
                _name_desc_ptr nd = _name_desc_alloc();

                mut_name(i, buf);
                string_append_s(nd->name, buf);

                mut_help_desc(i, buf);
                string_append_s(nd->desc, buf);
                vec_add(vec, nd);
            }
        }

        vec_sort(vec, (vec_cmp_f)_compare_name_desc);

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            fprintf(fp, "  <indent><color:B>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n\n", fp);
}

static void _draconians_help(FILE* fp)
{
    int i;

    fputs("<style:title>Draconians</style>\n\n", fp);
    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        /* TODO */
    }

    fputs("<topic:Tables><style:heading>Table 1 - Draconian Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_t_aux(RACE_DRACONIAN, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Draconian Skill Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows  Infra</color>\n", fp);

    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_t_aux(RACE_DRACONIAN, i);

        fprintf(fp, "%-14s %+4d  %+4d  %+4d  %+4d  %+4d  %+4d  %+5d  %+4d  %4d'\n",
            race_ptr->subname,
            race_ptr->skills.dis, race_ptr->skills.dev, race_ptr->skills.sav,
            race_ptr->skills.stl, race_ptr->skills.srh, race_ptr->skills.fos,
            race_ptr->skills.thn, race_ptr->skills.thb, race_ptr->infra*10
        );
    }
    fputs("\n</style>\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<style:heading>Table 3 - Draconian Special Powers</style>\n\n", fp);
        fputs("All draconians have access to special powers. When they reach level 35, they may choose "
                "a single power from the following list. "
                "These powers can never be removed or changed, so you might want to study this list to "
                "decide which power you will choose for your character.\n\n", fp);

        for (i = 0; i < MAX_MUTATIONS; i++)
        {
            if (mut_draconian_pred(i))
            {
                char buf[1024];
                _name_desc_ptr nd = _name_desc_alloc();

                mut_name(i, buf);
                string_append_s(nd->name, buf);

                mut_help_desc(i, buf);
                string_append_s(nd->desc, buf);
                vec_add(vec, nd);
            }
        }

        vec_sort(vec, (vec_cmp_f)_compare_name_desc);

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            fprintf(fp, "  <indent><color:B>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n\n", fp);
}

static void _dragon_realms_help(FILE* fp)
{
    int i, j;
    fputs("<style:title>Dragon Realms</style>\n\n", fp);
    fputs("Dragons are magical creatures and may choose to learn a particular branch of "
           "dragon magic. Unlike normal spell casters, dragons do not need spell books to "
           "cast or learn powers. Instead, they simply gain spells as they mature. Each "
           "realm of dragon magic has a direct impact on the player's stats and skills, and "
           "each realm also requires a different stat for casting purposes.\n\n", fp);
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "<style:heading>%s</style>\n\n", realm->name);
        fputs(realm->desc, fp);
        fputs("\n\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Dragon Realm Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               STR  INT  WIS  DEX  CON  CHR  Life  Exp</color>\n", fp);
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        char             line[255];
        char             tmp[255];

        sprintf(line, "%-14s", realm->name);
        for (j = 0; j < 6; j++)
        {
            if (j == realm->spell_stat)
                sprintf(tmp, "<color:G> %+3d </color>", realm->stats[j]);
            else
                sprintf(tmp, " %+3d ", realm->stats[j]);
            strcat(line, tmp);
        }
        sprintf(tmp, " %3d%%  %3d%%", realm->life, realm->exp);
        strcat(line, tmp);
        fprintf(fp, "%s\n", line);
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Dragon Realm Skill Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Attack  Breath</color>\n", fp);
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "%-14s %+4d  %+4d  %+4d  %+4d  %+4d  %+4d  %+5d  %5d%%  %5d%%\n",
            realm->name,
            realm->skills.dis, 
            realm->skills.dev,
            realm->skills.sav,
            realm->skills.stl, 
            realm->skills.srh, 
            realm->skills.fos,
            realm->skills.thn,
            realm->attack,
            realm->breath
        );
    }
    fputs("\n</style>\n", fp);
}

static void _class_help(FILE *fp, int idx)
{
    class_t *class_ptr = get_class_t_aux(idx, 0);

    fprintf(fp, "<style:heading>%s</style>\n", class_ptr->name);
    fputs(class_ptr->desc, fp);
    fputs("\n\n", fp);
}

static void _classes_help(FILE* fp)
{
    int i, j, r;

    fputs("<style:title>The Classes</style>\n\n", fp);
    for (i = 0; i < MAX_CLASS; i++)
    {
        if (i == CLASS_MONSTER) continue;
        _class_help(fp, i);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Class Statistic Bonus Table</style>\n<style:table>\n", fp);

    for (i = 0, r = 0; i < MAX_CLASS; i++)
    {
        class_t     *class_ptr = get_class_t_aux(i, 0);
        caster_info *caster_ptr = 0;
        char         line[255];
        char         tmp[255];

        if (class_ptr->caster_info)
            caster_ptr = class_ptr->caster_info();

        if (i == CLASS_MONSTER) continue;

        if (r % 20 == 0)
            fprintf(fp, "<color:U>               STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n");
        r++;

        sprintf(line, "%-14s", class_ptr->name);
        for (j = 0; j < 6; j++)
        {
            if (caster_ptr && j == caster_ptr->which_stat && i != CLASS_PSION)
                sprintf(tmp, "<color:G> %+3d </color>", class_ptr->stats[j]);
            else
                sprintf(tmp, " %+3d ", class_ptr->stats[j]);
            strcat(line, tmp);
        }
        sprintf(tmp, " %3d%%  %+3d  %3d%%", class_ptr->life, class_ptr->base_hp, class_ptr->exp);
        strcat(line, tmp);
        fprintf(fp, "%s\n", line);
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Class Skill Bonus Table</style>\n<style:table>\n", fp);
    for (i = 0, r = 0; i < MAX_CLASS; i++)
    {
        class_t *class_ptr = get_class_t_aux(i, 0);

        if (i == CLASS_BERSERKER) continue;
        if (i == CLASS_MONSTER) continue;

        if (r % 20 == 0)
            fprintf(fp, "<color:U>               Dsrm   Dvce   Save   Stlh  Srch  Prcp  Melee  Bows</color>\n");
        r++;

        fprintf(fp, "%-14s %2d+%-2d  %2d+%-2d  %2d+%-2d  %4d  %4d  %4d  %2d+%-2d  %2d+%-2d\n",
            class_ptr->name,
            class_ptr->base_skills.dis, class_ptr->extra_skills.dis, 
            class_ptr->base_skills.dev, class_ptr->extra_skills.dev, 
            class_ptr->base_skills.sav, class_ptr->extra_skills.sav,
            class_ptr->base_skills.stl, 
            class_ptr->base_skills.srh, 
            class_ptr->base_skills.fos,
            class_ptr->base_skills.thn, class_ptr->extra_skills.thn, 
            class_ptr->base_skills.thb, class_ptr->extra_skills.thb
        );
    }
    fputs("\n</style>\n", fp);
}

static void _personality_help(FILE *fp, int idx)
{
    player_seikaku *a_ptr = &seikaku_info[idx];
    fprintf(fp, "<style:heading>%s</style>\n\n", a_ptr->title);
    fputs(birth_get_personality_desc(idx), fp);
    fputs("\n\n", fp);
}

static void _personalities_help(FILE* fp)
{
    int i;

    fprintf(fp, "<style:title>The Personalities</style>\n\n");
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        _personality_help(fp, i);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Personality Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               STR  INT  WIS  DEX  CON  CHR  Life  Exp</color>\n", fp);

    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        player_seikaku *a_ptr = &seikaku_info[i];

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%%\n",
            a_ptr->title,
            a_ptr->a_adj[0], a_ptr->a_adj[1], a_ptr->a_adj[2], 
            a_ptr->a_adj[3], a_ptr->a_adj[4], a_ptr->a_adj[5], 
            a_ptr->life, a_ptr->a_exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Personality Skill Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:U>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows</color>\n", fp);
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        player_seikaku *a_ptr = &seikaku_info[i];

        fprintf(fp, "%-14s %+4d  %+4d  %+4d  %+4d  %+4d  %+4d  %+5d  %+4d\n",
            a_ptr->title,
            a_ptr->skills.dis, a_ptr->skills.dev, a_ptr->skills.sav,
            a_ptr->skills.stl, a_ptr->skills.srh, a_ptr->skills.fos,
            a_ptr->skills.thn, a_ptr->skills.thb
        );
    }
    fputs("\n</style>\n", fp);
}

static void _show_help(cptr helpfile)
{
    screen_save();
    show_file(TRUE, helpfile, NULL, 0, 0);
    screen_load();
}

static void _mon_dam_help(FILE* fp)
{
    int i, j;
    fprintf(fp, "Name,Idx,Lvl,HP,Ac,El,Fi,Co,Po,Li,Dk,Cf,Nt,Nx,So,Sh,Ca,Di\n");
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        int           hp = 0;
        int           dam[RES_MAX] = {0};
        bool          show = FALSE;

        if (r_ptr->flags1 & RF1_FORCE_MAXHP)
            hp = r_ptr->hdice * r_ptr->hside;
        else
            hp = r_ptr->hdice * (1 + r_ptr->hside)/2;

        /* Damage Logic Duplicated from mspells1.c */
        if (r_ptr->flags4 & RF4_ROCKET)
            dam[RES_SHARDS] = MAX(dam[RES_SHARDS], MIN(hp / 4, 600));
        if (r_ptr->flags4 & RF4_BR_ACID)
            dam[RES_ACID] = MAX(dam[RES_ACID], MIN(hp / 4, 900));
        if (r_ptr->flags4 & RF4_BR_ELEC)
            dam[RES_ELEC] = MAX(dam[RES_ELEC], MIN(hp / 4, 900));
        if (r_ptr->flags4 & RF4_BR_FIRE)
            dam[RES_FIRE] = MAX(dam[RES_FIRE], MIN(hp / 4, 900));
        if (r_ptr->flags4 & RF4_BR_COLD)
            dam[RES_COLD] = MAX(dam[RES_COLD], MIN(hp / 4, 900));
        if (r_ptr->flags4 & RF4_BR_POIS)
            dam[RES_POIS] = MAX(dam[RES_POIS], MIN(hp / 5, 600));
        if (r_ptr->flags4 & RF4_BR_NETH)
            dam[RES_NETHER] = MAX(dam[RES_NETHER], MIN(hp / 6, 550));
        if (r_ptr->flags4 & RF4_BR_LITE)
            dam[RES_LITE] = MAX(dam[RES_LITE], MIN(hp / 6, 400));
        if (r_ptr->flags4 & RF4_BR_DARK)
            dam[RES_DARK] = MAX(dam[RES_DARK], MIN(hp / 6, 400));
        if (r_ptr->flags4 & RF4_BR_CONF)
            dam[RES_CONF] = MAX(dam[RES_CONF], MIN(hp / 6, 400));
        if (r_ptr->flags4 & RF4_BR_SOUN)
            dam[RES_SOUND] = MAX(dam[RES_SOUND], MIN(hp / 6, 450));
        if (r_ptr->flags4 & RF4_BR_CHAO)
            dam[RES_CHAOS] = MAX(dam[RES_CHAOS], MIN(hp / 6, 600));
        if (r_ptr->flags4 & RF4_BR_DISE)
            dam[RES_DISEN] = MAX(dam[RES_DISEN], MIN(hp / 6, 500));
        if (r_ptr->flags4 & RF4_BR_NEXU)
            dam[RES_NEXUS] = MAX(dam[RES_NEXUS], MIN(hp / 3, 250));
        if (r_ptr->flags4 & RF4_BR_SHAR)
            dam[RES_SHARDS] = MAX(dam[RES_SHARDS], MIN(hp / 6, 500));
        if (r_ptr->flags4 & RF4_BR_NUKE)
            dam[RES_POIS] = MAX(dam[RES_POIS], MIN(hp / 5, 600));
        if (r_ptr->flags5 & RF5_BA_DARK)
            dam[RES_DARK] = MAX(dam[RES_DARK], r_ptr->level*4 + 105);
        if (r_ptr->flags5 & RF5_BA_LITE)
            dam[RES_LITE] = MAX(dam[RES_LITE], r_ptr->level*4 + 105);

        for (j = 0; j < RES_MAX; j++)
        {
            if (dam[j] > 0)
                show = TRUE;
        }

        if (show)
        {
            fprintf(fp, "\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                r_name + r_ptr->name, i, r_ptr->level, hp,
                dam[RES_ACID], dam[RES_ELEC], dam[RES_FIRE], dam[RES_COLD], dam[RES_POIS],
                dam[RES_LITE], dam[RES_DARK], dam[RES_CONF], dam[RES_NETHER], dam[RES_NEXUS],
                dam[RES_SOUND], dam[RES_SHARDS], dam[RES_CHAOS], dam[RES_DISEN]
            );
        }
    }
}

static void _possessor_stats_help(FILE* fp)
{
    int i;
    fprintf(fp, "Name,Idx,Lvl,Speed,AC,Attacks,Dam,Body,Str,Int,Wis,Dex,Con,Chr,Life,Disarm,Device,Save,Stealth,Search,Perception,Melee,Bows\n");
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        if (r_ptr->flags9 & RF9_DROP_CORPSE)
        {
            int ac = 0, dam = 0, attacks = 0, j;

            if (r_ptr->flags9 & RF9_POS_GAIN_AC)
                ac = r_ptr->ac;

            for (j = 0; j < 4; j++)
            {
                if (!r_ptr->blow[j].effect) continue;
                if (r_ptr->blow[j].method == RBM_EXPLODE) continue;
                if (r_ptr->blow[j].method == RBM_SHOOT) continue;

                dam += r_ptr->blow[j].d_dice * (r_ptr->blow[j].d_side + 1) / 2;
                attacks++;
            }

            fprintf(fp, "\"%s\",%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,=\"%d+%d\",=\"%d+%d\",=\"%d+%d\",%d,%d,%d,=\"%d+%d\",=\"%d+%d\"\n",
                r_name + r_ptr->name, i, r_ptr->level, 
                r_ptr->speed - 110, ac, attacks, dam,
                b_name + b_info[r_ptr->body.body_idx].name,
                r_ptr->body.stats[A_STR], r_ptr->body.stats[A_INT], r_ptr->body.stats[A_WIS],
                r_ptr->body.stats[A_DEX], r_ptr->body.stats[A_CON], r_ptr->body.stats[A_CHR],
                r_ptr->body.life,
                r_ptr->body.skills.dis, r_ptr->body.extra_skills.dis, 
                r_ptr->body.skills.dev, r_ptr->body.extra_skills.dev, 
                r_ptr->body.skills.sav, r_ptr->body.extra_skills.sav,
                r_ptr->body.skills.stl,
                r_ptr->body.skills.srh, 
                r_ptr->body.skills.fos,
                r_ptr->body.skills.thn, r_ptr->body.extra_skills.thn, 
                r_ptr->body.skills.thb, r_ptr->body.extra_skills.thb
            );
        }
    }
}

void generate_spoilers(void)
{
    spoiler_hack = TRUE;

    _text_file("Races.txt", _races_help);
    _text_file("MonsterRaces.txt", _monster_races_help);
    _text_file("Demigods.txt", _demigods_help);
    _text_file("Draconians.txt", _draconians_help);
    _text_file("Classes.txt", _classes_help);
    _text_file("Personalities.txt", _personalities_help);
    _text_file("PossessorStats.csv", _possessor_stats_help);
    _text_file("MonsterDam.csv", _mon_dam_help);
    _text_file("DragonRealms.txt", _dragon_realms_help);

    _show_help("Personalities.txt");
    spoiler_hack = FALSE;
}

#endif
