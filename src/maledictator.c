#include "angband.h"

static int _curse_boost = 0;
static int _curse_boost_capped = 0;
static int _curse_ty_count = 0;

#define _CURSE_TIER_2_PTS  \
    (OFC_TELEPORT_SELF | OFC_ADD_L_CURSE | \
     OFC_CALL_ANIMAL | OFC_HEAVY_CURSE | \
     OFC_TELEPORT | OFC_DRAIN_HP | OFC_DRAIN_MANA)
#define _CURSE_TIER_3_PTS  \
    (OFC_DRAIN_EXP | OFC_DRAIN_HP | OFC_DRAIN_MANA | \
     OFC_CALL_DEMON | OFC_CALL_DRAGON | OFC_COWARDICE | \
     OFC_TELEPORT | OFC_DRAIN_HP | OFC_DRAIN_MANA)
#define _CURSE_TIER_4_PTS  (OFC_AGGRAVATE)

#define _N_CURSES 22

int _count_flags_set(u32b flg)
{
	// Number of flags set.
	flg = flg - ((flg >> 1) & 0x55555555);
	flg = (flg & 0x33333333) + ((flg >> 2) & 0x33333333);
	return (((flg + (flg >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

int _calculate_curse_power(void){
	int slot;
	int boost = 0;
	u32b checklist = 0;

	for (slot = equip_find_first(object_is_cursed); slot; slot = equip_find_next(object_is_cursed, slot))
	{
		object_type *o_ptr = equip_obj(slot);
		checklist |= (o_ptr->curse_flags);
	}

	boost = _count_flags_set(checklist);
	//SPECIAL ONES
	if (checklist & OFC_PERMA_CURSE) boost += 5;
	if (checklist & OFC_TY_CURSE) boost += 4;
	if (checklist & OFC_AGGRAVATE) boost += 3;
	//Other nasty things
	if (checklist & (OFC_HEAVY_CURSE | OFC_DRAIN_EXP | OFC_DRAIN_HP | OFC_DRAIN_MANA)) boost++;
	return boost;
}


bool _purge_curse(void){

	int item;
	char tmp_str[MAX_NLEN];
	object_type *o_ptr;

	item_tester_hook = object_is_cursed;
	if (!get_item(&item, "Remove curse from which item? ", "There's nothing to uncurse.", (USE_INVEN | USE_EQUIP)))
		return FALSE;
	o_ptr = &inventory[item];
	if (item >= 0) o_ptr = &inventory[item];

	if (!o_ptr) return FALSE;

	if (o_ptr->curse_flags & OFC_PERMA_CURSE){ 
			object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
			msg_format("The curse permeating %s is permanent.", tmp_str);
		return FALSE; 
	} // do nothing. It's permanent.
	if (p_ptr->lev > 35 && o_ptr->curse_flags & OFC_PERMA_CURSE){
		o_ptr->curse_flags = 0; o_ptr->curse_flags |= OFC_PERMA_CURSE;
		o_ptr->known_curse_flags = 0; o_ptr->known_curse_flags |= OFC_PERMA_CURSE; /* Forget lore in preparation for next cursing */
		o_ptr->ident |= IDENT_SENSE;
		o_ptr->feeling = FEEL_NONE;
		p_ptr->update |= PU_BONUS;
		p_ptr->window |= PW_EQUIP;
		p_ptr->redraw |= PR_EFFECTS;
		object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
		msg_format("The curse of %s is permanent. Lesser curses are stripped away.", tmp_str);
		return TRUE;
	}
	else if (p_ptr->lev > 25 && o_ptr->curse_flags & OFC_HEAVY_CURSE){
		o_ptr->curse_flags = 0;
		o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
		o_ptr->ident |= IDENT_SENSE;
		o_ptr->feeling = FEEL_NONE;
		p_ptr->update |= PU_BONUS;
		p_ptr->window |= PW_EQUIP;
		p_ptr->redraw |= PR_EFFECTS;
		object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
		msg_format("You feel a heavy curse being lifted from %s", tmp_str);
		return TRUE;
	}
	else if (o_ptr->curse_flags & OFC_CURSED){
		o_ptr->curse_flags = 0;
		o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
		o_ptr->ident |= IDENT_SENSE;
		o_ptr->feeling = FEEL_NONE;
		p_ptr->update |= PU_BONUS;
		p_ptr->window |= PW_EQUIP;
		p_ptr->redraw |= PR_EFFECTS;
		object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
		msg_format("You feel a curse being lifted from %s", tmp_str);
		return TRUE;
	}
	else{
		return FALSE; 
	}
}

bool _curse_item_aux(void){

	int item, power = 0;
	char tmp_str[MAX_NLEN];
	object_type *o_ptr;
	item_tester_hook = object_is_equipment;

	if (!get_item(&item, "Curse which item? ", "There's nothing to uncurse.", (USE_INVEN | USE_EQUIP)))
		return FALSE;

	o_ptr = &inventory[item];
	if (item >= 0) o_ptr = &inventory[item];
	if (!o_ptr) return FALSE;

	if (p_ptr->lev > 35) power = 2;
	else power = 1;

	object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
	if(power==1) msg_format("A terrible black aura blasts your %s!", tmp_str);
	else msg_format("A black aura surrounds your %s!", tmp_str);

	o_ptr->curse_flags |= (OFC_CURSED);
	if(power == 2) o_ptr->curse_flags |= (OFC_HEAVY_CURSE);

	for (int i = 0; i < randint0(power+1) + 1; i++){
		o_ptr->curse_flags |= get_curse(power, o_ptr);
	}

	return TRUE;
}

static void _purge_curse_spell(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Purge Curse");break;
	case SPELL_DESC: var_set_string(res, "Removes a curse from a single item. At high levels you can dispel even more powerful curses."); break;
	case SPELL_CAST: var_set_bool(res, _purge_curse()); break;
	default: default_spell(cmd, res);break;}
}

static void _curse_item_spell(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Curse Item"); break;
	case SPELL_DESC: var_set_string(res, "Curses a single item."); break;
	case SPELL_CAST: var_set_bool(res,_curse_item_aux()); break;
	default: default_spell(cmd, res);break;}
}

static int _get_curse_rolls(int pow){

	if (pow == 2) return 4 + (p_ptr->lev / 10) + (_curse_boost_capped / 3); // max 4 + 5 + 5 = 14 
	else if (pow == 1) return 2 + (p_ptr->lev / 10) + (_curse_boost_capped / 5); // max 2 + 5 + 3 = 10
	return 2 + (p_ptr->lev + 10) / 20 + ((_curse_boost_capped * 2) / 15); // max 2 + 3 + 2 = 5

}

static bool _inflict_curse(int pow){ 
	bool result = FALSE;
	int m_idx = 0;
	monster_type *m_ptr;
	char m_name[MAX_NLEN];
	int plev = p_ptr->lev;

	if (!target_set(TARGET_KILL)) return FALSE;

	m_idx = cave[target_row][target_col].m_idx;
		if (!m_idx) return FALSE;
		if (m_idx == p_ptr->riding) return FALSE;
		if (!player_has_los_bold(target_row, target_col)) return FALSE;
		m_ptr = &m_list[m_idx];
	monster_desc(m_name, m_ptr, 0);

	if(pow==0) msg_format("You invoke a minor curse on %s. ", m_name);
	else if (pow == 1) msg_format("You invoke curse on %s. ", m_name);
	else msg_format("You invoke a great curse on %s. ", m_name);

	if (m_ptr)
	{
	
			int ct = 0; int p = 0;
			int highest_power = 0;
			int dType = -1;
			int refunds = 0;
			int dmg = 0;
			int rolls = 1;

			rolls = _get_curse_rolls(pow);
			if (one_in_(3)) rolls++;

			while (ct <= rolls){
				ct++;
				dmg = plev + _curse_boost_capped;

				if (one_in_(666) && plev > 40){ dType = GF_DEATH_RAY; p = 7; dmg = spell_power(plev * 200); }
				else if (one_in_(66) && plev > 40){ dType = GF_BLOOD_CURSE; p = 6; }
				else if (one_in_(22) && plev > 35){ dType = GF_HAND_DOOM; p = 5; }
				else if (one_in_(18) && plev > 30){ dType = GF_ANTIMAGIC; p = 4; }
				else if (one_in_(15) && plev > 10){ dType = GF_AMNESIA; p = 3; }
				else if (one_in_(12) && plev > 10){ dType = GF_PARALYSIS; p = 3; }
				else if (one_in_(10)){ dType = GF_OLD_CONF; p = 2; }
				else if (one_in_(8)){ dType = GF_STUN; p = 2; }
				else if (one_in_(6)){ dType = GF_TURN_ALL; p = 1; }
				else if (one_in_(4)){ dType = GF_OLD_SLOW; p = 1; }
				if (p > highest_power) highest_power = p;

				if (r_info[m_idx].flags1 & RF1_UNIQUE){ // if it is an unique, give some refund for high-powered ones...
					if (p == 6 || p == 3 || p == 7){refunds++; continue;}
				}

				project(0, 0, m_ptr->fy, m_ptr->fx, dmg, dType, PROJECT_KILL | PROJECT_HIDE | PROJECT_NO_PAIN | PROJECT_SHORT_MON_NAME, -1);
			}
			// In addition to these things, we also have some damage. Depends a lot on highest power.
			dmg += ( plev/2 ) * refunds + plev * pow;
			project(0, 0, m_ptr->fy, m_ptr->fx, dmg, GF_NETHER, PROJECT_KILL | PROJECT_HIDE | PROJECT_NO_PAIN | PROJECT_SHORT_MON_NAME, -1);

			switch (highest_power){
					case 1: msg_format("%^s is subjected to a curse. ", m_name); break;
					case 2: msg_format("The curse reaches out for %s. ", m_name); break;
					case 3: msg_format("%^s is cursed. ", m_name); break;
					case 4: msg_format("%^s takes heavy toll from curse. ", m_name); break;
					case 5: msg_format("%^s is blasted by a mighty curse! ", m_name); break;
					case 6: msg_format("Black Omen! Terrible fate awaits %s! ", m_name); break;
					case 7: msg_format("Hand of death reaches for %s! ", m_name); break;
					default: msg_format("But nothing happens. ", m_name); break;
			}
			energy_use = 100;
			return TRUE;
		}

		return FALSE;
}

/* MINOR CURSE */
static void _minor_curse(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Lesser Curse"); break;
	case SPELL_INFO: var_set_string(res, info_power(_get_curse_rolls(0)*10)); break;
	case SPELL_DESC: var_set_string(res, "Invokes a minor curse on a single monster."); break;
	case SPELL_CAST: var_set_bool(res, _inflict_curse(0)); break;
	default:default_spell(cmd, res);break;}
}

/* CURSE */
static void _curse_spell(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Curse"); break;
	case SPELL_INFO: var_set_string(res, info_power(_get_curse_rolls(1) * 10)); break;
	case SPELL_DESC: var_set_string(res, "Invokes a curse on a single monster."); break;
	case SPELL_CAST: var_set_bool(res, _inflict_curse(1)); break;
	default:default_spell(cmd, res); break;}
}

/* MAJOR CURSE */
static void _major_curse(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Great Curse"); break;
	case SPELL_INFO: var_set_string(res, info_power(_get_curse_rolls(2) * 10)); break;
	case SPELL_DESC: var_set_string(res, "Invokes a terrible curse on a single monster."); break;
	case SPELL_CAST: var_set_bool(res,_inflict_curse(2)); break;
	default:default_spell(cmd, res); break;
	}
}
static void _sense_misfortune(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Sense Misfortune"); break;
	case SPELL_DESC: var_set_string(res, "Senses monsters and traps in range."); break;
	case SPELL_INFO: var_set_string(res, info_radius(20 + _curse_boost_capped)); break;
	case SPELL_CAST: 
		msg_print("You attempt to sense misfortune... \n");
		detect_monsters_evil( 20 + _curse_boost_capped );
		detect_traps(20 + _curse_boost_capped, FALSE);
		var_set_bool(res, TRUE);
		break;
	default:default_spell(cmd, res); break;
	}
}
static void _curse_of_impotence(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Curse of Impotence"); break;
	case SPELL_DESC: var_set_string(res, "Curses all creatures with impotence."); break;
	case SPELL_CAST: 
		num_repro += MAX_REPRO; 
		msg_print("You feel a tangible increase in abstinence...");
		var_set_bool(res, TRUE);
		break;
	default:default_spell(cmd, res); break;
	}
}

static void _dimensional_anchor(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Dimensional Lock"); break;
	case SPELL_DESC: var_set_string(res, "Locks everything in place, slowing them disallowing teleportation of monsters - and you."); break;
	case SPELL_CAST:
		set_tim_no_tele(25 + randint0(25), FALSE);
		msg_print("Everything is chained into space...");
		project_hack(GF_OLD_SLOW, p_ptr->lev * 2 + _curse_boost);
		var_set_bool(res, TRUE);
		break;
	default:default_spell(cmd, res); break;
	}
}


static int _get_powers(spell_info* spells, int max)
{
	int ct = 0;

	spell_info* spell = &spells[ct++];
	spell->level = 3;
	spell->cost = 20;
	spell->fail = calculate_fail_rate(spell->level, 20, p_ptr->stat_ind[A_CHR]);
	spell->fn = _purge_curse_spell;

	spell = &spells[ct++];
	spell->level = 7;
	spell->cost = 20;
	spell->fail = calculate_fail_rate(spell->level, 35, p_ptr->stat_ind[A_CHR]);
	spell->fn = _curse_item_spell;

	return ct;
}

bool maledict_ty_protection(void){
	if (p_ptr->pclass != CLASS_MALEDICT) return FALSE;
	if (p_ptr->lev > 30){
		if (one_in_(2)) return TRUE;
	}
	return FALSE;
}

static spell_info _spells[] =
{
	/*lvl cst fail spell */
	{ 1, 10, 30, _minor_curse }, // debuff
	{ 5, 12, 40, _sense_misfortune }, // detect traps / monsters
	{ 10, 15, 40, _curse_of_impotence}, // no breeding
	//{ 15, 8, 50,  _Drain_Curse_Power }, // attempts to remove curse from one equipment, restores mana based on curses.
	//{ 20, 9, 50,  _Umbra}, // Stealth buff, cancels out aggravation
	{ 25, 40, 50, _curse_spell}, // stronger debuff + damage
	{ 30, 30, 50, _dimensional_anchor }, // -TELE on self and everyone on sight
	//{ 40, 15, 60, _Suppress_Curses }, // Buff, negates effects of curses for some duration ( including AC! )
	{ 45, 80, 60, _major_curse }, // crippling debuff
	//{ 50, 100, 80, _Unleash }, // *remove curse, heal, LOS effects depending on the curse_power. Requires curses to be present.
	{ -1, -1, -1, NULL }
};

static int _get_spells(spell_info* spells, int max)
{
	return get_spells_aux(spells, max, _spells);
}

static void _calc_bonuses(void)
{
	int boost = 0;
	_curse_ty_count = 0;

	_curse_boost = boost; // save up the uncapped boost.
	// 22 is true max number of curse flags that you can have, totaling to score of 38. 
	// To gain full power, The boost itself scales up to 15.
	boost = (boost*2) / 3;
	int boost_cap = 5 + p_ptr->lev / 5;

	if (boost > boost_cap) boost = boost_cap;
	_curse_boost_capped = boost;

	p_ptr->to_d_spell += p_ptr->lev / 10 + boost / 2;

	p_ptr->weapon_info[0].to_h += boost; p_ptr->weapon_info[1].to_h += boost;
	p_ptr->to_h_m += boost;
	p_ptr->weapon_info[0].dis_to_h += boost; p_ptr->weapon_info[1].dis_to_h += boost;

	p_ptr->weapon_info[0].to_d += boost; p_ptr->weapon_info[1].to_d += boost;
	p_ptr->to_d_m += boost;
	p_ptr->weapon_info[0].dis_to_d += boost; p_ptr->weapon_info[1].dis_to_d += boost;

}

static caster_info * _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "malison";
		me.which_stat = A_CHR;
		me.weight = 500;
		me.options = CASTER_ALLOW_DEC_MANA | CASTER_GLOVE_ENCUMBRANCE;
		me.min_fail = 5;
		init = TRUE;
	}
	return &me;
}

class_t *maledict_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	if (!init)
	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 20, 24, 34, 1, 12, 2, 68, 40 };
		skills_t xs = { 7, 10, 11, 0, 0, 0, 21, 18 };

		me.name = "Maledict";
		me.desc = "A maledict draws their powers from the misfortunes, bad omens and ill luck. "
			"Wielding variety depilating and damaging spells, blah blah blah description ";

		me.stats[A_STR] = 2;
		me.stats[A_INT] = -1;
		me.stats[A_WIS] = -2;
		me.stats[A_DEX] = 0;
		me.stats[A_CON] = 2;
		me.stats[A_CHR] = 3;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 110;
		me.base_hp = 14;
		me.exp = 135;
		me.pets = 30;

		me.calc_bonuses = _calc_bonuses;
		me.caster_info = _caster_info;
		me.get_spells = _get_spells;
		me.get_powers = _get_powers;
		me.character_dump = spellbook_character_dump;
		init = TRUE;
	}

	return &me;
}

