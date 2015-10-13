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

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->name, race_ptr->name);
    fprintf(fp, "%s\n\n", race_ptr->desc);
    switch(idx)
    {
    case RACE_DEMIGOD:
        fputs("See <link:Demigods.txt> for more details on demigod parentage.\n\n", fp);
        break;
    case RACE_DRACONIAN:
        fputs("See <link:Draconians.txt> for more details on draconians.\n\n", fp);
        break;
    case RACE_MON_RING:
        fputs("See <link:rings.txt> for more details on rings.\n\n", fp);
        break;
    case RACE_MON_DRAGON:
        fputs("See <link:DragonRealms.txt> for more details on dragons.\n\n", fp);
        break;
    }

    fputs("  <indent><style:table><color:G>Stat Modifiers          Skill Modifiers</color>\n", fp);
    fprintf(fp, "Strength     %+3d        Disarming   %+4d\n", race_ptr->stats[A_STR], race_ptr->skills.dis);
    fprintf(fp, "Intelligence %+3d        Device      %+4d\n", race_ptr->stats[A_INT], race_ptr->skills.dev);
    fprintf(fp, "Wisdom       %+3d        Save        %+4d\n", race_ptr->stats[A_WIS], race_ptr->skills.sav);
    fprintf(fp, "Dexterity    %+3d        Stealth     %+4d\n", race_ptr->stats[A_DEX], race_ptr->skills.stl);
    fprintf(fp, "Constitution %+3d        Searching   %+4d\n", race_ptr->stats[A_CON], race_ptr->skills.srh);
    fprintf(fp, "Charisma     %+3d        Perception  %+4d\n", race_ptr->stats[A_CHR], race_ptr->skills.fos);
    fprintf(fp, "Life Rating  %3d%%       Melee       %+4d\n", race_ptr->life, race_ptr->skills.thn);
    fprintf(fp, "Base HP      %3d        Bows        %+4d\n", race_ptr->base_hp, race_ptr->skills.thb);
    fprintf(fp, "Experience   %3d%%       Infravision %4d'\n", race_ptr->exp, race_ptr->infra*10);
    fputs("</style></indent>\n", fp);
}

/* TODO: This is copied/duplicated in birth.txt ... Spoiler generation is a convenience
   hack, so I'll turn a blind eye for now :) */
#define _MAX_RACES_PER_GROUP 23
#define _MAX_RACE_GROUPS      8
typedef struct _race_group_s {
    cptr name;
    int ids[_MAX_RACES_PER_GROUP];
} _race_group_t;
static _race_group_t _race_groups[_MAX_RACE_GROUPS] = {
    { "Humans",
        {RACE_AMBERITE, RACE_BARBARIAN, RACE_DEMIGOD, RACE_DUNADAN, RACE_HUMAN, -1} },
    { "Elves",
        {RACE_DARK_ELF, RACE_HIGH_ELF, RACE_WOOD_ELF, -1} },
    { "Hobbits and Dwarves",
        {RACE_DWARF, RACE_GNOME, RACE_HOBBIT, RACE_NIBELUNG, -1} },
    { "Fairies",
        {RACE_SHADOW_FAIRY, RACE_SPRITE, -1} },
    { "Angels and Demons",
        {RACE_ARCHON, RACE_BALROG, RACE_IMP, -1} },
    { "Orcs, Trolls and Giants",
        {RACE_CYCLOPS, RACE_HALF_GIANT, RACE_HALF_OGRE,
         RACE_HALF_TITAN, RACE_HALF_TROLL, RACE_KOBOLD, RACE_SNOTLING, -1} },
    { "The Undead",
        {RACE_SKELETON, RACE_SPECTRE, RACE_VAMPIRE, RACE_ZOMBIE, -1} },
    { "Other Races",
        {RACE_ANDROID, RACE_BEASTMAN, RACE_CENTAUR, RACE_DRACONIAN, RACE_DOPPELGANGER, RACE_ENT,
         RACE_GOLEM, RACE_KLACKON, RACE_KUTAR, RACE_MIND_FLAYER, RACE_TONBERRY, RACE_YEEK,-1 } },
};

static void _races_help(FILE* fp)
{
    int i, j;

    fputs("<style:title>The Races</style>\n", fp);
    fputs("There are many races in the world, each, for the most part, with both "
          "strengths and weaknesses. Humans are the base race, and serve as the benchmark "
          "of comparison for all the others. In general, the stronger a race is relative to "
          "humans, the higher the <color:keyword>Experience Penalty</color> and the longer "
          "it will take to gain levels.\n\n"
          "For details on the <color:keyword>Primary Statistics</color>, see "
          "<link:birth.txt#PrimaryStats>. For information about the various <color:keyword>Skills</color>, see "
          "<link:birth.txt#PrimarySkills>. To compare the various races, you might want to take a look "
          "at <link:Races.txt#Tables> the race tables below.\n\n", fp);
    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", _race_groups[i].name);
        for (j = 0; ; j++)
        {
            int race_idx = _race_groups[i].ids[j];
            if (race_idx == -1) break;
            _race_help(fp, race_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Race Statistic Bonus Table</style>\n<style:table>\n", fp);

    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        fprintf(fp, "<color:o>%s</color>\n", _race_groups[i].name);
        fprintf(fp, "<color:G>%-14.14s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "Race");
        for (j = 0; ; j++)
        {
            int     race_idx = _race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_t_aux(race_idx, 0);
            fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
                race_ptr->name,
                race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
                race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
                race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
            );
        }
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Race Skill Bonus Table</style>\n<style:table>\n", fp);

    for (i = 0; i < _MAX_RACE_GROUPS; i++)
    {
        fprintf(fp, "<color:o>%s</color>\n", _race_groups[i].name);
        fprintf(fp, "<color:G>%-14.14s</color> <color:G>Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows  Infra</color>\n", "Race");
        for (j = 0; ; j++)
        {
            int     race_idx = _race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_t_aux(race_idx, 0);
            fprintf(fp, "%-14s %+4d  %+4d  %+4d  %+4d  %+4d  %+4d  %+5d  %+4d  %4d'\n",
                race_ptr->name,
                race_ptr->skills.dis, race_ptr->skills.dev, race_ptr->skills.sav,
                race_ptr->skills.stl, race_ptr->skills.srh, race_ptr->skills.fos,
                race_ptr->skills.thn, race_ptr->skills.thb, race_ptr->infra*10
            );
        }
        fputc('\n', fp);
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
        race_t *race_ptr = get_race_t_aux(RACE_DEMIGOD, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);

        fputs("  <indent><style:table><color:G>Stat Modifiers          Skill Modifiers</color>\n", fp);
        fprintf(fp, "Strength     %+3d        Disarming   %+4d\n", race_ptr->stats[A_STR], race_ptr->skills.dis);
        fprintf(fp, "Intelligence %+3d        Device      %+4d\n", race_ptr->stats[A_INT], race_ptr->skills.dev);
        fprintf(fp, "Wisdom       %+3d        Save        %+4d\n", race_ptr->stats[A_WIS], race_ptr->skills.sav);
        fprintf(fp, "Dexterity    %+3d        Stealth     %+4d\n", race_ptr->stats[A_DEX], race_ptr->skills.stl);
        fprintf(fp, "Constitution %+3d        Searching   %+4d\n", race_ptr->stats[A_CON], race_ptr->skills.srh);
        fprintf(fp, "Charisma     %+3d        Perception  %+4d\n", race_ptr->stats[A_CHR], race_ptr->skills.fos);
        fprintf(fp, "Life Rating  %3d%%       Melee       %+4d\n", race_ptr->life, race_ptr->skills.thn);
        fprintf(fp, "Base HP      %3d        Bows        %+4d\n", race_ptr->base_hp, race_ptr->skills.thb);
        fprintf(fp, "Experience   %3d%%       Infravision %4d'\n", race_ptr->exp, race_ptr->infra*10);
        fputs("</style></indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Demigod Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < MAX_DEMIGOD_TYPES; i++)
    {
        race_t *race_ptr = get_race_t_aux(RACE_DEMIGOD, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS], 
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR], 
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Demigod Skill Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:G>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows  Infra</color>\n", fp);
    for (i = 0; i < MAX_DEMIGOD_TYPES; i++)
    {
        race_t *race_ptr = get_race_t_aux(RACE_DEMIGOD, i);

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
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n",
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
        race_t *race_ptr = get_race_t_aux(RACE_DRACONIAN, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);

        fputs("  <indent><style:table><color:G>Stat Modifiers          Skill Modifiers</color>\n", fp);
        fprintf(fp, "Strength     %+3d        Disarming   %+4d\n", race_ptr->stats[A_STR], race_ptr->skills.dis);
        fprintf(fp, "Intelligence %+3d        Device      %+4d\n", race_ptr->stats[A_INT], race_ptr->skills.dev);
        fprintf(fp, "Wisdom       %+3d        Save        %+4d\n", race_ptr->stats[A_WIS], race_ptr->skills.sav);
        fprintf(fp, "Dexterity    %+3d        Stealth     %+4d\n", race_ptr->stats[A_DEX], race_ptr->skills.stl);
        fprintf(fp, "Constitution %+3d        Searching   %+4d\n", race_ptr->stats[A_CON], race_ptr->skills.srh);
        fprintf(fp, "Charisma     %+3d        Perception  %+4d\n", race_ptr->stats[A_CHR], race_ptr->skills.fos);
        fprintf(fp, "Life Rating  %3d%%       Melee       %+4d\n", race_ptr->life, race_ptr->skills.thn);
        fprintf(fp, "Base HP      %3d        Bows        %+4d\n", race_ptr->base_hp, race_ptr->skills.thb);
        fprintf(fp, "Experience   %3d%%       Infravision %4d'\n", race_ptr->exp, race_ptr->infra*10);
        fputs("</style></indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Draconian Statistic Bonus Table</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

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
    fputs("<style:table><color:G>               Dsrm  Dvce  Save  Stlh  Srch  Prcp  Melee  Bows  Infra</color>\n", fp);

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
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n",
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
    class_t     *class_ptr = get_class_t_aux(idx, 0);
    caster_info *caster_ptr = NULL;

    if (class_ptr->caster_info && idx != CLASS_PSION)
        caster_ptr = class_ptr->caster_info();

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->name, class_ptr->name);
    fputs(class_ptr->desc, fp);
    fputs("\n\n", fp);

    fputs("  <indent><style:table><color:G>Stat Modifiers          Skill Modifiers</color>\n", fp);
    fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %2d+%-2d\n",
        (caster_ptr && caster_ptr->which_stat == A_STR) ? 'v' : 'w',
        class_ptr->stats[A_STR],
        class_ptr->base_skills.dis,
        class_ptr->extra_skills.dis);
    fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %2d+%-2d\n",
        (caster_ptr && caster_ptr->which_stat == A_INT) ? 'v' : 'w',
        class_ptr->stats[A_INT],
        class_ptr->base_skills.dev,
        class_ptr->extra_skills.dev);
    fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %2d+%-2d\n",
        (caster_ptr && caster_ptr->which_stat == A_WIS) ? 'v' : 'w',
        class_ptr->stats[A_WIS],
        class_ptr->base_skills.sav,
        class_ptr->extra_skills.sav);
    fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %2d\n",
        (caster_ptr && caster_ptr->which_stat == A_DEX) ? 'v' : 'w',
        class_ptr->stats[A_DEX],
        class_ptr->base_skills.stl);
    fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %2d\n",
        (caster_ptr && caster_ptr->which_stat == A_CON) ? 'v' : 'w',
        class_ptr->stats[A_CON],
        class_ptr->base_skills.srh);
    fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %2d\n",
        (caster_ptr && caster_ptr->which_stat == A_CHR) ? 'v' : 'w',
        class_ptr->stats[A_CHR],
        class_ptr->base_skills.fos);
    fprintf(fp, "Life Rating  %3d%%       Melee       %2d+%-2d\n",
        class_ptr->life,
        class_ptr->base_skills.thn,
        class_ptr->extra_skills.thn);
    fprintf(fp, "Base HP      %3d        Bows        %2d+%-2d\n",
        class_ptr->base_hp,
        class_ptr->base_skills.thb,
        class_ptr->extra_skills.thb);
    fprintf(fp, "Experience   %3d%%\n", class_ptr->exp);
    fputs("</style></indent>\n", fp);
}

#define _MAX_CLASSES_PER_GROUP 20
#define _MAX_CLASS_GROUPS      11
typedef struct _class_group_s {
    cptr name;
    int ids[_MAX_CLASSES_PER_GROUP];
} _class_group_t;
static _class_group_t _class_groups[_MAX_CLASS_GROUPS] = {
    { "Melee", {CLASS_BERSERKER, CLASS_BLOOD_KNIGHT, CLASS_DUELIST, CLASS_MAULER,
                    CLASS_RUNE_KNIGHT, CLASS_SAMURAI, CLASS_WARRIOR, CLASS_WEAPONMASTER,
                    CLASS_WEAPONSMITH, -1} },
    { "Archery", {CLASS_ARCHER, CLASS_SNIPER, -1} },
    { "Martial Arts", {CLASS_FORCETRAINER, CLASS_MONK, CLASS_MYSTIC, -1} },
    { "Magic", {CLASS_BLOOD_MAGE, CLASS_BLUE_MAGE, CLASS_HIGH_MAGE, CLASS_MAGE,
                    CLASS_NECROMANCER, CLASS_SORCERER, -1} },
    { "Devices", {CLASS_DEVICEMASTER, /*CLASS_MAGIC_EATER,*/ -1} },
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

static void _classes_help(FILE* fp)
{
    int i, j, k;

    fputs("<style:title>The Classes</style>\n\n", fp);

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", _class_groups[i].name);
        for (j = 0; ; j++)
        {
            int class_idx = _class_groups[i].ids[j];
            if (class_idx == -1) break;
            _class_help(fp, class_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>Table 1 - Class Statistic Bonus Table</style>\n<style:table>\n", fp);

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        fprintf(fp, "<color:o>%s</color>\n", _class_groups[i].name);
        fprintf(fp, "<color:G>%-14.14s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "Class");
        for (j = 0; ; j++)
        {
            int          class_idx = _class_groups[i].ids[j];
            class_t     *class_ptr;
            caster_info *caster_ptr = NULL;
            char         line[255];
            char         tmp[255];

            if (class_idx == -1) break;
            class_ptr = get_class_t_aux(class_idx, 0);
            if (class_ptr->caster_info)
                caster_ptr = class_ptr->caster_info();

            sprintf(line, "%-14s", class_ptr->name);
            for (k = 0; k < 6; k++)
            {
                if (caster_ptr && k == caster_ptr->which_stat && class_idx != CLASS_PSION)
                    sprintf(tmp, "<color:v> %+3d </color>", class_ptr->stats[k]);
                else
                    sprintf(tmp, " %+3d ", class_ptr->stats[k]);
                strcat(line, tmp);
            }
            sprintf(tmp, " %3d%%  %+3d  %3d%%", class_ptr->life, class_ptr->base_hp, class_ptr->exp);
            strcat(line, tmp);
            fprintf(fp, "%s\n", line);
        }
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<style:heading>Table 2 - Class Skill Bonus Table</style>\n<style:table>\n", fp);

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        fprintf(fp, "<color:o>%s</color>\n", _class_groups[i].name);
        fprintf(fp, "<color:G>%-14.14s</color> <color:G>Dsrm   Dvce   Save   Stlh  Srch  Prcp  Melee  Bows</color>\n", "Class");
        for (j = 0; ; j++)
        {
            int          class_idx = _class_groups[i].ids[j];
            class_t     *class_ptr;

            if (class_idx == -1) break;
            if (class_idx == CLASS_BERSERKER) continue;
            class_ptr = get_class_t_aux(class_idx, 0);

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
        fputc('\n', fp);
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
    spoiler_hack = FALSE;
}

#endif
