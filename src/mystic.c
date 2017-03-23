#include "angband.h"

/****************************************************************
 * Helpers
 ****************************************************************/
cptr mystic_spec_name(int psubclass)
{
	switch (psubclass)
	{
	case MYSTIC_SPEC_FIST: return "Way of the Fist";
	case MYSTIC_SPEC_MIND: return "Way of the Mind";
	}
	return "";
}

cptr mystic_spec_desc(int psubclass)
{
	switch (psubclass)
	{
	case MYSTIC_SPEC_FIST: return "You specialize in physical techniques.";
	case MYSTIC_SPEC_MIND: return "You specialize in mystic abilities.";
	}
	return "";
}

static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;

    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();

    return result;
}

int mystic_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (p_ptr->pclass == CLASS_MYSTIC && !heavy_armor())
        result = _get_toggle();
    return result;
}

static void _on_browse(int which)
{
    bool screen_hack = screen_is_saved();
    if (screen_hack) screen_load();

    display_weapon_mode = which;
    do_cmd_knowledge_weapon();
    display_weapon_mode = 0;

    if (screen_hack) screen_save();
}

/****************************************************************
 * Spells
 ****************************************************************/
static void _toggle_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_get_toggle() == which)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(which);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != which)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _acid_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Corrosive Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with an acid blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_ACID));
        break;
    case SPELL_ON_BROWSE:
        _on_browse(MYSTIC_ACID);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _cold_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Icy Fists");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a freezing blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_COLD));
        break;
    case SPELL_ON_BROWSE:
        _on_browse(MYSTIC_COLD);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/* For the Logrus Master
static void _confusing_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Confusing Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with confusing blows.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_CONFUSE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
} */

static void _crushing_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Crushing Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with crushing blows for extra damage.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_CRITICAL));
        break;
    case SPELL_ON_BROWSE:
        _on_browse(MYSTIC_CRITICAL);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _defense_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Defensive Stance");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you gain increased armor class at the expense of your fighting prowess.");
        break;
    default:
        _toggle_spell(MYSTIC_TOGGLE_DEFENSE, cmd, res);
        break;
    }
}

static void _elec_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Lightning Eagle");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a shocking blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_ELEC));
        break;
    case SPELL_ON_BROWSE:
        _on_browse(MYSTIC_ELEC);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _fast_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Quick Approach");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you will move with great haste.");
        break;
    default:
        _toggle_spell(MYSTIC_TOGGLE_FAST, cmd, res);
        break;
    }
}

static void _fire_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Flaming Strike");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a flaming blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_FIRE));
        break;
    case SPELL_ON_BROWSE:
        _on_browse(MYSTIC_FIRE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _killing_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Touch of Death");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to kill an adjacent opponent with a single blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_KILL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _knockout_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Knockout Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempt to knockout an adjacent opponent.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_KNOCKOUT));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mystic_insights_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Mystic Insights");
        break;
    default:
        probing_spell(cmd, res);
        break;
    }
}

static void _offense_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Death Stance");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you concentrate all your mental efforts on offensive deadliness. As such, you become more exposed to enemy attacks.");
        break;
    default:
        _toggle_spell(MYSTIC_TOGGLE_OFFENSE, cmd, res);
        break;
    }
}

static void _poison_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Serpent's Tongue");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a poisonous blow.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_POIS));
        break;
    case SPELL_ON_BROWSE:
        _on_browse(MYSTIC_POIS);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _retaliate_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Aura of Retaliation");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you will retaliate when struck.");
        break;
    default:
        _toggle_spell(MYSTIC_TOGGLE_RETALIATE, cmd, res);
        break;
    }
}

static void _stealth_toggle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stealthy Approach");
        break;
    case SPELL_DESC:
        var_set_string(res, "When using this technique, you will gain enhanced stealth.");
        break;
    default:
        _toggle_spell(MYSTIC_TOGGLE_STEALTH, cmd, res);
        break;
    }
}

static void _stunning_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stunning Blow");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with stunning blows.");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(MYSTIC_STUN));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_hounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Hounds");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon hounds for assistance.");
        break;
    case SPELL_CAST:
    {
        int num = 1; /* randint0(p_ptr->lev/10); */
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HOUND, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No hounds arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_spiders_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Summon Spiders");
        break;
    case SPELL_DESC:
        var_set_string(res, "Summon spiders for assistance.");
        break;
    case SPELL_CAST:
    {
        int num = 1; /* randint0(p_ptr->lev/10); */
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_SPIDER, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("No spiders arrive.");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/** PHYSICAL ALTERNATIVE **/
int prompt_adjancent_monster(int *res_y, int *res_x){
	int x, y;
	int dir;
	int m_idx;
	if (use_old_target && target_okay())
	{
		y = target_row;
		x = target_col;
		m_idx = cave[y][x].m_idx;
		if (m_idx)
		{
			if (m_list[m_idx].cdis > 1)
				m_idx = 0;
			else
				dir = 5;
			*res_x = x; *res_y = y;
			return m_idx;
		}
		else return -1;
	}
	else{
		if (!get_rep_dir2(&dir)) return -1;
		if (dir == 5) return -1;
		y = py + ddy[dir];
		x = px + ddx[dir];
		m_idx = cave[y][x].m_idx;
		if (!m_idx)
		{
			msg_print("There is no monster there.");
			return -1;
		}
		*res_x = x; *res_y = y;
		return m_idx;
	}
}
void _mystic_jump(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Jump");
		break;
	case SPELL_DESC:
		var_set_string(res, "Jump to a random position.");
		break;
	case SPELL_CAST:
		teleport_player(8, TELEPORT_LINE_OF_SIGHT | TELEPORT_NONMAGICAL);
		var_set_bool(res, TRUE);
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

#define _HFANG_RANGE 10

static bool _cave_is_open(int y, int x)
{
	if (cave_have_flag_bold(y, x, FF_HURT_ROCK)) return FALSE;
	if (cave[y][x].feat == feat_permanent) return FALSE;
	if (cave[y][x].feat == feat_permanent_glass_wall) return FALSE;
	if (cave[y][x].feat == feat_mountain) return FALSE;
	return TRUE;
}

static bool horizontal_fang_aux(){

	int tx, ty, i;
	int range = _HFANG_RANGE;
	bool sq_okay = FALSE, strikeMsg = TRUE;
	u16b path_g[32];
	int path_n;
	int flg = PROJECT_THRU | PROJECT_KILL;

	if (!tgt_pt(&tx, &ty, _HFANG_RANGE)) return FALSE;

	project_length = range;
	path_n = project_path(path_g, project_length, py, px, ty, tx, flg);
	project_length = 0;

	if (!path_n) return FALSE;
	if (!los(ty, tx, py, px)){
		msg_print("You have to see where you are going! ");
		return FALSE;
	}
	else if (!cave_player_teleportable_bold(ty, tx, TELEPORT_LINE_OF_SIGHT | TELEPORT_NONMAGICAL) || !in_bounds(ty, tx)){
		msg_print("Invalid destination! ");
		return FALSE;
	}


	/* No scrolling for you */
	if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
		wilderness_scroll_lock = TRUE;

	(void)move_player_effect(ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);

	for (i = 0; i < path_n; i++)
	{
		cave_type *c_ptr;

		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);
		c_ptr = &cave[ny][nx];
		sq_okay = !c_ptr->m_idx && player_can_enter(c_ptr->feat, 0);

		if (cave[ny][nx].m_idx)
		{
			if (strikeMsg){ msg_print("You strike through your enemies!!"); strikeMsg = FALSE; }
			py_attack(ny, nx, 0);
		}
	}

	if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
	{
		wilderness_scroll_lock = FALSE;
		wilderness_move_player(px, py);
	}

	return TRUE;
}

void _horizontal_fang(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Horizontal Fang");
		break;
	case SPELL_DESC:
		var_set_string(res, "Charge through a straight line, attacking anyone on the way.");
		break;
	case SPELL_CAST:
		var_set_bool(res, horizontal_fang_aux());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

void _quaking_strike(int cmd, variant *res){

	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Earthquaking Fist");
		break;
	case SPELL_DESC:
		var_set_string(res, "Shakes dungeon structure, and results in random swapping of floors and walls.");
		break;
	case SPELL_CAST:
	{
		msg_print("You strike the ground! ");
		if (earthquake(py, px, 10))  msg_print("And the dungeon shakes!! ");
		var_set_bool(res, TRUE);
		break;
	}
	default:
		default_spell(cmd, res);
		break;
	}
}

void _overdrive(int cmd, variant *res){
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Raging Demon");
		break;
	case SPELL_DESC:
		var_set_string(res, "Makes two unavoidable attacks that pierce through invulnerability barriers.");
		break;
	case SPELL_CAST:
	{
		int x, y;
		bool result = FALSE;
		int m_idx = prompt_adjancent_monster(&y, &x);
		if (m_idx){
			msg_print("Shun Goku Satsu!");
			if(one_in_(3)) virtue_add(VIRTUE_COMPASSION, -1);
			py_attack(y, x, MONK_OVERDRIVE);
			if (cave[y][x].m_idx)
			{
				handle_stuff();
				py_attack(y, x, MONK_OVERDRIVE);
			}
			
			if (m_list[m_idx].hp <= 0){ msg_print("Ten!"); }
			result = TRUE;
		}
		var_set_bool(res, result);
		break;
	}
	case SPELL_ENERGY:
		var_set_int(res, 70 + ENERGY_NEED());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

static void _circle_kick_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Circle Kick");
		break;
	case SPELL_DESC:
		var_set_string(res, "Kicks all adjacent opponents, stunning them.");
		break;
	case SPELL_INFO:
	{
		int ds = p_ptr->lev;
		int dd = 0;
		dd = MIN(10, p_ptr->lev / 4);

		var_set_string(res, info_damage(dd, ds, p_ptr->to_d_m));
		break;
	}
	case SPELL_CAST:
	{
		circle_kick();
		var_set_bool(res, TRUE);
		break;
	}
	default:
		default_spell(cmd, res);
		break;
	}
}

static void _break_rock_spell(int cmd, variant *res)
{
	/*Copied over from archeologist.*/
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Break Rock");
		break;
	case SPELL_DESC:
		var_set_string(res, "Break through walls or other obstacles with your fists.");
		break;
	case SPELL_ENERGY:
		var_set_int(res, 200);
		break;
	case SPELL_CAST:
	{
		int dir = 5;
		bool b = FALSE;
		if (get_rep_dir2(&dir)
			&& dir != 5)
		{
			int x, y;
			y = py + ddy[dir];
			x = px + ddx[dir];
			if (!in_bounds(y, x))
			{
				msg_print("This wall can not be broken.");
			}
			else if (cave_have_flag_bold(y, x, FF_WALL)
				|| cave_have_flag_bold(y, x, FF_TREE)
				|| cave_have_flag_bold(y, x, FF_CAN_DIG))
			{
				msg_print("You break through wall!");
				cave_alter_feat(y, x, FF_TUNNEL);
				sound(SOUND_DIG);
				teleport_player_to(y, x, TELEPORT_NONMAGICAL); 
				b = TRUE;
			}
			else
			{
				msg_print("There is nothing to excavate.");
			}
		}
		var_set_bool(res, b);
	}
	break;
	default:
		default_spell(cmd, res);
		break;
	}
}


/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
static spell_info _spells_mind[] =
{
    /*lvl cst fail spell */
    {  1,  0,  0, samurai_concentration_spell},
    {  3,  8,  0, _fire_strike_spell},
    {  5,  8, 30, _summon_spiders_spell},
    {  7,  8,  0, _cold_strike_spell},
    {  9, 10, 30, detect_menace_spell},
    { 11, 10,  0, _poison_strike_spell},
    { 13, 15, 40, sense_surroundings_spell},
    { 15,  0,  0, _stealth_toggle_spell},
    { 17,  0,  0, _fast_toggle_spell},
    { 19,  0,  0, _defense_toggle_spell},
    { 21, 15, 50, _mystic_insights_spell},
    /* For the Logrus Master: { 23, 15,  0, _confusing_strike_spell}, */
    { 25, 17,  0, _acid_strike_spell},
    { 27, 20,  0, _stunning_blow_spell},
    { 29,  0,  0, _retaliate_toggle_spell},
    { 30, 30, 60, haste_self_spell},
    { 32, 30, 60, resistance_spell},
    { 33, 30,  0, _elec_strike_spell},
    { 35, 20, 60, rush_attack_spell},
    { 36, 40, 60, _summon_hounds_spell},
    { 37,  0,  0, _offense_toggle_spell},
    { 39, 40,  0, _knockout_blow_spell},
    { 42, 50,  0, _killing_strike_spell},
    { 45, 70,  0, _crushing_blow_spell},
    { -1, -1, -1, NULL}
};

static spell_info _spells_fist[] =
{
	/*lvl cst fail spell */
	{ 1, 0, 0, samurai_concentration_spell },
	{ 5, 10, 20, _mystic_jump },
	{ 9, 10, 30, detect_menace_spell },
	{ 13, 15, 40, sense_surroundings_spell },
	{ 15, 0, 0, _stealth_toggle_spell },
	{ 17, 0, 0, _fast_toggle_spell },
	{ 19, 0, 0, _defense_toggle_spell },
	{ 21, 10, 20, awesome_blow_spell },
	{ 22, 25, 30, swap_pos_spell },
	{ 24, 20, 0, _stunning_blow_spell },
	{ 25, 25, 30, rush_attack_spell },
	{ 27, 20, 40, _circle_kick_spell },
	{ 28, 2, 0, _break_rock_spell },
	{ 29, 0, 0, _retaliate_toggle_spell },
	{ 30, 30, 60, haste_self_spell },
	{ 32, 30, 60, resistance_spell },
	{ 33, 30, 0, _quaking_strike },
	{ 35, 20, 60, _offense_toggle_spell },
	{ 37, 0, 0, _knockout_blow_spell },
	{ 39, 40, 0, _horizontal_fang },
	{ 42, 50, 0, _killing_strike_spell },
	{ 45, 70, 0, _crushing_blow_spell },
	{ 50, 100, 50, _overdrive},
	{ -1, -1, -1, NULL }
};


static int _get_spells(spell_info* spells, int max)
{
	switch (p_ptr->psubclass){
		case MYSTIC_SPEC_MIND:     return get_spells_aux(spells, max, _spells_mind);
		case MYSTIC_SPEC_FIST:     return get_spells_aux(spells, max, _spells_fist);
		default: return get_spells_aux(spells, max, _spells_mind);
	}
}
static void _calc_bonuses(void)
{
    p_ptr->monk_lvl = p_ptr->lev;
    if (!heavy_armor())
    {
        p_ptr->pspeed += p_ptr->lev/10;
        if  (p_ptr->lev >= 25)
            p_ptr->free_act = TRUE;

        switch (_get_toggle())
        {
        case MYSTIC_TOGGLE_STEALTH:
            p_ptr->skills.stl += 2 + 3 * p_ptr->lev/50;
            break;
        case MYSTIC_TOGGLE_FAST:
            p_ptr->quick_walk = TRUE;
            break;
        case MYSTIC_TOGGLE_DEFENSE:
        {
            int bonus = 10 + 40*p_ptr->lev/50;
            p_ptr->to_a += bonus;
            p_ptr->dis_to_a += bonus;
            break;
        }
        case MYSTIC_TOGGLE_OFFENSE:
        {
            int penalty = 10 + 40*p_ptr->lev/50;
            p_ptr->to_a -= penalty;
            p_ptr->dis_to_a -= penalty;
            break;
        }
        }
    }
    monk_ac_bonus();
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (!heavy_armor())
    {
        if (_get_toggle() == MYSTIC_TOGGLE_RETALIATE)
            add_flag(flgs, OF_AURA_REVENGE);
        if (p_ptr->lev >= 10)
            add_flag(flgs, OF_SPEED);
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_FREE_ACT);
    }
}
static caster_info * _caster_info_mind(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "mystic technique";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 350;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 800;
        me.options = CASTER_SUPERCHARGE_MANA;
        init = TRUE;
    }
    return &me;
}

static void _character_dump(doc_ptr doc)
{
    spell_info spells[MAX_SPELLS];
    int        ct = _get_spells(spells, MAX_SPELLS);

    py_display_spells(doc, spells, ct);
}
class_t *mystic_get_class(int subclass)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  34,  36,   5,  32,  24,  64,  60};
    skills_t xs = { 15,  11,  10,   0,   0,   0,  18,  18};

        me.name = "Mystic";
        me.desc = "Mystics are masters of bare handed fighting, like Monks. However, they "
                  "do not learn normal spells. Instead, they gain mystical powers with experience, "
                  "and these powers directly influence their martial arts. In this respect, "
                  "Mystics are similar to the Samurai. Indeed, they even concentrate to boost "
                  "their mana like the Samurai. Mystics eschew weapons of any kind and require "
                  "the lightest of armors in order to practice their martial arts. The number "
                  "of attacks are influenced by dexterity and experience level while the mystic's "
                  "mana and fail rates are influenced by charisma. Mystics are in tune with the "
                  "natural forces around them and may even call on aid when necessary. It has "
                  "been whispered that mystics have even discovered how to kill an opponent with "
                  "a single touch, though they do not share this knowledge with novices.";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 130;
        me.pets = 35;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_SLOW | CLASS_SENSE2_STRONG;

        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info_mind;
        me.get_spells = _get_spells;
        me.character_dump = _character_dump;
        init = TRUE;
    }
	me.subname = mystic_spec_name(subclass);
	me.subdesc = mystic_spec_desc(subclass);

    return &me;
}
