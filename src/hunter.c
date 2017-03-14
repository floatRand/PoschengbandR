#include "angband.h"

static int _quarry_m_idx = -1;

static bool _fire(int power)
{
	bool result = FALSE;
	shoot_hack = power;
	shoot_count = 0;
	command_cmd = 'f'; /* Hack for inscriptions (e.g. '@f1') */
	result = do_cmd_fire();
	shoot_hack = SHOOT_NONE;
	shoot_count = 0;

	return result;
}

static void _fire_spell(int which, int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_CAST:
		var_set_bool(res, FALSE);
		if (_fire(which))
			var_set_bool(res, TRUE);
		break;
	case SPELL_ENERGY:
		var_set_int(res, energy_use);    /* already set correctly by do_cmd_fire() */
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

void _mapping_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Mapping");
		break;
	case SPELL_DESC:
		var_set_string(res, "Maps nearby area");
		break;
	default: magic_mapping_spell(cmd, res); break;
	}
}

void _set_minor_trap_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Set Minor Trap");
		break;
	case SPELL_DESC:
		var_set_string(res, "Sets a weak trap under you. This trap will have various weak effects on a passing monster.");
		break;
	case SPELL_CAST:
		var_set_bool(res, set_trap(py, px, feat_rogue_trap1));
		break;
	default: default_spell(cmd, res); break;
	}
}

void _set_major_trap_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Set Major Trap");
		break;
	case SPELL_DESC:
		var_set_string(res, "Sets a major trap under you. This trap will have various  effects on a passing monster.");
		break;
	case SPELL_CAST:
		var_set_bool(res, set_trap(py, px, feat_rogue_trap2));
		break;
	default: default_spell(cmd, res); break;
	}
}

void _poison_shot_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Poison Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "Fires a poisoned projectile that deals additional damage to creatures not resistant to poison.");
		break;
	default:
	_fire_spell(SHOOT_POISON,cmd,res); break;
	}
}

void _detect_monsters(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Detect Monsters");
		break;
	case SPELL_DESC:
		var_set_string(res, "Detects monsters nearby.");
		break;
	case SPELL_CAST:
		var_set_bool(res, detect_monsters_normal(DETECT_RAD_DEFAULT));
		break;
	default: default_spell(cmd, res); break;

	}
}

void _scout_uniques(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Scout Uniques");
		break;
	case SPELL_DESC:
		var_set_string(res, "Lists unique monsters in level.");
		break;
	case SPELL_CAST:{
		int i;
		for (i = m_max - 1; i >= 1; i--)
		{
			if (!m_list[i].r_idx) continue;
			if (r_info[m_list[i].r_idx].flags1 & RF1_UNIQUE)
			{
				msg_format("%s. ", r_name + r_info[m_list[i].r_idx].name);
			}
		}
		var_set_bool(res, TRUE);
	} break;
	default: default_spell(cmd, res); break;
	}
}

#define _FAN_MAX_TARGETS   5
/*Shamelessly copied over from weaponmaster...*/
static bool _check_direct_shot(int tx, int ty)
{
	bool result = FALSE;
	u16b path[512];
	int ct = project_path(path, 50, py, px, ty, tx, PROJECT_PATH); /* We don't know the length ... just project from source to target, please! */
	int x, y, i;

	for (i = 0; i < ct; i++)
	{
		x = GRID_X(path[i]);
		y = GRID_Y(path[i]);

		/* Reached target! Yay! */
		if (x == tx && y == ty)
		{
			result = TRUE;
			break;
		}

		/* Stopped by walls/doors */
		if (!cave_have_flag_bold(y, x, FF_PROJECT) && !cave[y][x].m_idx) break;

		/* Monster in the way of target */
		if (cave[y][x].m_idx) break;
	}

	return result;
}

static bool _fan_shot_targets(int *targets, int max)
{
	int i, i2, i3;
	monster_type *m_ptr = NULL;
	int in_sight[_FAN_MAX_TARGETS];
	int tar_dist[_FAN_MAX_TARGETS];
	int ct = 0, dist = 500;
	int rng = 0;
	int offset = 0;
	bool result = FALSE;

	if (max > _FAN_MAX_TARGETS) max = _FAN_MAX_TARGETS;

	if (p_ptr->shooter_info.slot)
		rng = bow_range(equip_obj(p_ptr->shooter_info.slot));

	int main_m_idx = 0;

	if (target_set(TARGET_KILL)){
		if (main_m_idx != p_ptr->riding){
			main_m_idx = cave[target_row][target_col].m_idx;
		}
	}

	if (!player_has_los_bold(target_row, target_col)) return FALSE;

	for (i2 = 0; i2 < max; i2++){ in_sight[i2] = 0; tar_dist[i2] = 500; } // setup comparison array

	if (main_m_idx){ in_sight[0] = main_m_idx; ct++; offset = 1; }

	/* Get monsters, prioritize proximity and LOS! */
	for (i = m_max - 1; i >= 1; i--)
	{
		m_ptr = &m_list[i];
		if (!m_ptr->r_idx) continue;
		if (m_ptr->smart & SM_FRIENDLY) continue;
		if (m_ptr->smart & SM_PET) continue;
		if (m_ptr->cdis > rng) continue;
		if (!m_ptr->ml) continue;
		if (!_check_direct_shot(m_ptr->fx, m_ptr->fy)) continue; // if obstructed by walls, doors or monsters...
		dist = distance(m_ptr->fy, m_ptr->fx, target_row, target_col);
		if (dist > 12) continue;
		
		for (i2 = offset; i2 < max; i2++){
			if (dist < tar_dist[i2]){ 
					for (i3 = max-1; i3 > i2; i3--)
					{
						if (i3 > offset) tar_dist[i3] = tar_dist[i3 - 1];
					}
					tar_dist[i2] = dist; 
					in_sight[i2] = i;
					break;
			}
		}
	}

	for (i = 0; i < max; i++){
			targets[i] = in_sight[i];
			if (!result) result = TRUE;
	}

	return result;
}


static void _fan_shot(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Fan Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "Shoots a number of projectiles at target and monsters nearby.");
		break;
	case SPELL_CAST:
	{
		var_set_bool(res, FALSE);
		int i, item;
		int tgts[_FAN_MAX_TARGETS];

		if (!_fan_shot_targets(tgts, _FAN_MAX_TARGETS)){ msg_print("No targets selected."); return; }

		object_type *bow = equip_obj(p_ptr->shooter_info.slot);

		item_tester_tval = p_ptr->shooter_info.tval_ammo;
		if (!get_item(&item,
			"Fire which ammo? ",
			"You have nothing to fire.", (USE_INVEN | USE_QUIVER)))
		{
			flush();
			return;
		}

		shoot_hack = SHOOT_FAN;

		int tgt, ty, tx;
		for (i = 0; i < _FAN_MAX_TARGETS; i++)
		{
			tgt = tgts[i];
			if (tgt <= 0 || tgt >= max_m_idx) continue;

			tx = m_list[tgt].fx;
		    ty = m_list[tgt].fy;

			do_cmd_fire_aux2(item, bow, px, py, tx, ty);
		}

		shoot_hack = SHOOT_NONE;
		var_set_bool(res, TRUE);
	} 
	break;
		
	default: default_spell(cmd, res); break;
	}
	

}

static void _volley_shot(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Volleying Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "Fire at target behind enemy.");
		break;
	default:
		_fire_spell(SHOOT_VOLLEY, cmd, res);
		break;
	}
}

static void _penetrating_shot(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Penetrating Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "If an shot hits opponent, it pierces and can also hit next opponent in same direction (requires another attack roll), up to 5 opponents. Each successive pierce suffers a cumulative penalty to hit.");
		break;
	default:
		_fire_spell(SHOOT_PIERCE, cmd, res);
		break;
	}
}

static void _exploding_shot(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Exploding Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "Fires an explosive shot.");
		break;
	default:
		_fire_spell(SHOOT_EXPLODE, cmd, res);
		break;
	}
}

static void _culling_shot(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Culling Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "Attempt to kill a monster a well aimed shot.");
		break;
	default:
		_fire_spell(SHOOT_PIERCE, cmd, res);
		break;
	}
}

static void _mana_shot(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Mana Shot");
		break;
	case SPELL_DESC:
		var_set_string(res, "Fires a shot wreathed in mana, dealing unavoidable extra damage. Never misses.");
		break;
	default:
		_fire_spell(SHOOT_MANA, cmd, res);
		break;
	}
}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
	if (!p_ptr->shooter_info.heavy_shoot
		&& p_ptr->shooter_info.tval_ammo <= TV_BOLT
		&& p_ptr->shooter_info.tval_ammo >= TV_SHOT)
	{
		p_ptr->shooter_info.num_fire += p_ptr->lev * 200 / 50;
	}
}

static bool mark_quarry_aux(void)
{
	bool result = FALSE;
	int m_idx = 0;

	if (target_set(TARGET_MARK))
	{
		if (target_who > 0)
			m_idx = target_who;
		else
			m_idx = cave[target_row][target_col].m_idx;
	}

	if (m_idx)
	{
		monster_type *m_ptr = &m_list[m_idx];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		if (m_idx == _quarry_m_idx)
			msg_format("%^s is already marked as your quarry.", r_name + r_ptr->name);
		else
		{

			if (r_ptr->flags1 & RF1_UNIQUE || r_ptr->flags7 & RF7_UNIQUE2 ||
				r_ptr->flags1 & RF1_QUESTOR || r_ptr->level > p_ptr->lev ){
				_quarry_m_idx = m_idx;
				msg_format("You mark %s as your quarry.", r_name + r_ptr->name);
				result = TRUE;
			}
			else { msg_print("That is not quarry worth of pursuing."); }
		}
	}
	else if (_quarry_m_idx > 0)
	{
		_quarry_m_idx = 0;
		msg_print("You let your quarry go.");
	}

	p_ptr->redraw |= PR_STATUS;
	return result;
}

static void _mark_quarry(int cmd, variant *res)
{

	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Mark Quarry");
		break;
	case SPELL_DESC:
		var_set_string(res, "Marks a strong or unique monster quarry. You gain additional damage against that target, and are always aware where that monster is. However, you do less damage against all other monsters.");
		break;
	case SPELL_CAST:
		var_set_bool(res, mark_quarry_aux());
		break;
	default: default_spell(cmd, res); break;
	}
}

static void _first_aid(int cmd, variant *res)
{

	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "First Aid");
		break;
	case SPELL_DESC:
		if(p_ptr->lev >= 25) var_set_string(res, "Treat cuts, blindness and stunning.");
		else if (p_ptr->lev >= 10) var_set_string(res, "Treat cuts, and blindness.");
		else  var_set_string(res, "Treat cuts.");
		break;
	case SPELL_CAST:
		var_set_bool(res, FALSE);
			hp_player(randint1(3));
			set_cut(0, TRUE);
			if(p_ptr->lev>=25) set_stun(0, TRUE);
			if(p_ptr->lev>=10) set_blind(0, TRUE);
		break;
	default: default_spell(cmd, res); break;
	}
}


static void _smoke_bomb(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Smoke Bomb");
		break;
	case SPELL_DESC:
		var_set_string(res, "Teleport away.");
		break;
	default:
		teleport_spell(cmd, res);
		break;
	}
}

int get_hunter_quarry(void){
	if (p_ptr->pclass != CLASS_HUNTER) return 0;
	return _quarry_m_idx;
}

void set_hunter_quarry(int x){
	if (p_ptr->pclass != CLASS_HUNTER) return;
	if (x < 0) x = 0;
	_quarry_m_idx = x;
}

cptr get_current_quarry_name(void){
	if (_quarry_m_idx < 1) return ""; 
	if (_quarry_m_idx > max_m_idx) return "";
	
	monster_type *m_ptr = &m_list[_quarry_m_idx];
	return r_name + r_info[m_ptr->r_idx].name;
}

static spell_info _spells[] =
{
	/*lvl cst fail spell */
	{ 2, 2, 20, _poison_shot_spell }, // just deal extra-damage on vulnerable monsters
	{ 4, 4, 30, _detect_monsters }, // detect monsters
	{ 6, 6, 40, _first_aid }, // heal cuts
	{ 8, 6, 30, _volley_shot }, // shoot over targets
	{ 12, 8, 40, strafing_spell }, // blink
	{ 15, 10, 40, _set_minor_trap_spell }, // as per spell
	{ 23, 16, 40, _fan_shot }, /* fires a fan of shots. max 5 targets?  */
	{ 25, 20, 40, _mapping_spell }, // map area
	{ 27, 13, 40, _smoke_bomb }, // tele
	{ 30, 30, 40, _penetrating_shot }, // as per 'penetrating arrow'
	{ 32, 36, 40, _exploding_shot }, // rocket, damage based on attack quality
	{ 34, 50, 40, _set_major_trap_spell }, // set a major trap
	{ 36, 0, 0, _scout_uniques }, // list uniques
	{ 40, 50, 40, _culling_shot }, // fuck off with single monster
	{ 50, 70, 40, _mana_shot }, // hurts
	{ -1, -1, -1, NULL }
};

static int _get_powers(spell_info* spells, int max)
{
	int ct = 0;

	spell_info* spell = &spells[ct++];
	spell->level = 15;
	spell->cost = 20;
	spell->fail = calculate_fail_rate(spell->level, 60, p_ptr->stat_ind[A_CHR]);
	spell->fn = probing_spell;

	spell = &spells[ct++];
	spell->level = 20;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _mark_quarry;

	return ct;
}


static int _get_spells(spell_info* spells, int max)
{
	return get_spells_aux(spells, max, _spells);
}

static void _birth(void)
{
	_quarry_m_idx = 0;
	py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
	py_birth_obj_aux(TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR, 1);
	py_birth_obj_aux(TV_BOW, SV_LIGHT_XBOW, 1);
	py_birth_obj_aux(TV_BOLT, SV_AMMO_NORMAL, rand_range(30, 50));
}

static caster_info * _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "hunter trick";
		me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 350;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
		me.min_fail = 0;
		init = TRUE;
	}
	return &me;
}

static void _load_player(savefile_ptr file)
{
	 _quarry_m_idx = savefile_read_s32b(file);
}

static void _save_player(savefile_ptr file)
{
	savefile_write_s32b(file, _quarry_m_idx);
}

void _process_hunter(void){
	if (_quarry_m_idx < 1) return;
	if (_quarry_m_idx > max_m_idx) _quarry_m_idx = 0;

	monster_type *m_ptr;
	if (m_list){
		m_ptr = &m_list[_quarry_m_idx];
		if (!m_ptr->r_idx){ _quarry_m_idx = 0; return; }

		/* Repair visibility later */
		repair_monsters = TRUE;

		/* Hack -- Detect monster */
		m_ptr->mflag2 |= (MFLAG2_MARK | MFLAG2_SHOW);

		/* Update the monster */
		update_mon(_quarry_m_idx, FALSE);
	}
}

class_t *hunter_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;
	if (!init)
	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 38, 12, 29, 6, 24, 16, 45, 72 };
		skills_t xs = { 12, 4, 10, 0, 0, 0, 12, 28 };

		me.name = "Hunter";
		me.desc = "Hunters are trackers and slayers of beast and collectors of bounties. "
			"While their archery does not quite compare to likes of archers', they  "
			"supplement it with their unique abilities and techniques. "
			"Some examples might be shooting showers of arrows, listing unique monsters "
			"and setting down traps. \n \n"
			"Indeed - their dedication to slaying significant beasts awards them with "
			"bonuses against wanted monsters or quest monsters. Later on they also gain "
			"some of this bonus on all unique monsters.\n"
			"{ EXPERIMENTAL CLASS }";

		me.stats[A_STR] = 0;
		me.stats[A_INT] = -1;
		me.stats[A_WIS] = 0;
		me.stats[A_DEX] = 2;
		me.stats[A_CON] = 0;
		me.stats[A_CHR] = 2;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 102;
		me.base_hp = 8;
		me.exp = 140;
		me.pets = 30;

		me.birth = _birth;
		me.calc_shooter_bonuses = _calc_shooter_bonuses;
		me.caster_info = _caster_info;
		me.get_spells = _get_spells;
		me.process_player = _process_hunter;
		me.get_powers = _get_powers;
		me.save_player = _save_player;
		me.load_player = _load_player;

		me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
			CLASS_SENSE2_STRONG;

		init = TRUE;
	}
	return &me;
}
