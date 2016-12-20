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

static bool _antitele_toggle = FALSE;

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
			var_set_int(res, 0);   
		else
			var_set_int(res, 100);
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

int _calculate_curse_power(void){
	int slot;
	int boost = 0;
	u32b checklist = 0;
	int basePow = 0;

	for (slot = equip_find_first(object_is_cursed); slot; slot = equip_find_next(object_is_cursed, slot))
	{
		object_type *o_ptr = equip_obj(slot);
		checklist |= (o_ptr->curse_flags);
		if (o_ptr->curse_flags & OFC_PERMA_CURSE) basePow += 3;
		else if (o_ptr->curse_flags & OFC_HEAVY_CURSE) basePow += 2;
		else if (o_ptr->curse_flags & OFC_CURSED) basePow++;
	}
	basePow /= 2;
	boost = _count_flags_set(checklist) + basePow;
	//SPECIAL ONES
	if (checklist & OFC_PERMA_CURSE) boost += 5;
	if (checklist & OFC_TY_CURSE) boost += 4;
	if (checklist & OFC_AGGRAVATE) boost += 3;
	//Other nasty things
	if (checklist & (OFC_HEAVY_CURSE | OFC_DRAIN_EXP | OFC_DRAIN_HP | OFC_DRAIN_MANA)) boost++;
	return boost;
}

bool _purge_curse_which(object_type *o_ptr, int itemnum){
	char tmp_str[MAX_NLEN];

	if (o_ptr->curse_flags & OFC_PERMA_CURSE){
		object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
		msg_format("The curse permeating %s is permanent.", tmp_str);
		return FALSE;
	} // do nothing. It's permanent.
	if (p_ptr->lev >= 35 && o_ptr->curse_flags & OFC_PERMA_CURSE){
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
	else if (p_ptr->lev >= 25 && o_ptr->curse_flags & OFC_HEAVY_CURSE){
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

bool _purge_curse(void){

	int item;
	object_type *o_ptr;

	item_tester_hook = object_is_cursed;
	if (!get_item(&item, "Remove curse from which item? ", "There's nothing to uncurse.", (USE_INVEN | USE_EQUIP)))
		return FALSE;
	o_ptr = &inventory[item];
	if (item >= 0) o_ptr = &inventory[item];

	if (!o_ptr) return FALSE;
	_purge_curse_which(o_ptr, item);
	
	return TRUE;
}

void _curse_item_t(object_type *o_ptr, int pow){

	if (pow >= 1) o_ptr->curse_flags |= (OFC_HEAVY_CURSE);
	o_ptr->curse_flags |= (OFC_CURSED);

	// curses are actually not guaranteed... You can try to reroll them.
		if(one_in_(3)) o_ptr->curse_flags |= get_curse(pow, o_ptr);
		if(one_in_(9)) o_ptr->curse_flags |= get_curse(pow, o_ptr);
		if(one_in_(27)) o_ptr->curse_flags |= get_curse(pow, o_ptr);

	o_ptr->known_curse_flags = o_ptr->curse_flags; // make the curses known.
}

bool _curse_item_aux(bool all){

	int item, power = 0;
	char tmp_str[MAX_NLEN];
	object_type *o_ptr;
	item_tester_hook = object_is_equipment;

	if (!all){
		if (!get_item(&item, "Curse which item? ", "There's nothing to curse.", (USE_INVEN | USE_EQUIP)))
			return FALSE;

		o_ptr = &inventory[item];
		if (item >= 0) o_ptr = &inventory[item];
		if (!o_ptr) return FALSE;

		if (p_ptr->lev >= 35) power = 1;
		else power = 0;

		object_desc(tmp_str, o_ptr, OD_COLOR_CODED);
		if (power == 1) msg_format("A terrible black aura blasts your %s!", tmp_str);
		else msg_format("A black aura surrounds your %s!", tmp_str);

		_curse_item_t(o_ptr, power);
	}
	else {
		if (p_ptr->lev >= 40){ msg_format("A terrible black aura blasts through your equipment!", tmp_str); power = 1; }
		else { msg_format("Black aura washes over your equipment!", tmp_str); power = 0; }

		int slot = 0;
		for (slot = equip_find_first(object_is_wearable); slot; slot = equip_find_next(object_is_wearable, slot))
		{
			_curse_item_t(equip_obj(slot), power);
		}

	}

	p_ptr->update |= (PU_BONUS);
	update_stuff();
	return TRUE;
}

static void _purge_curse_spell(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Purge Curse");break;
	case SPELL_DESC: var_set_string(res, "Removes a curse from a single item. At high levels you can dispel even more powerful curses."); break;
	case SPELL_CAST: var_set_bool(res, _purge_curse()); break;
	default: default_spell(cmd, res);break;
	}
}

static void _curse_item_spell(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Curse Item"); break;
	case SPELL_DESC: var_set_string(res, "Curses a single item."); break;
	case SPELL_CAST: var_set_bool(res,_curse_item_aux(FALSE)); break;
	default: default_spell(cmd, res);break;}
}

static void _curse_item_all_spell(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Curse Equipment"); break;
	case SPELL_DESC: var_set_string(res, "Curses all equipment. This takes bit longer, but not as long as cursing items invidually."); break;
	case SPELL_CAST: var_set_bool(res, _curse_item_aux(TRUE)); break;
	case SPELL_ENERGY: var_set_int(res, 300);
	default: default_spell(cmd, res); break;
	}
}

static int _get_curse_rolls(int pow){

	if (pow == 2) return 4 + (p_ptr->lev / 10) + (_curse_boost_capped / 3); // max 4 + 5 + 5 = 14 
	else if (pow == 1) return 2 + (p_ptr->lev / 10) + (_curse_boost_capped / 3); // max 1 + 5 + 5 = 11
	return 2 + (p_ptr->lev + 10) / 20 + (_curse_boost_capped / 5); // max 1 + 3 + 3 = 7

}

static bool _inflict_curse_aux(int pow, monster_type *m_ptr, int m_idx, bool DoDamage){
	int ct = 0; int p = 0;
	int highest_power = 0;
	int dType = -1;
	int refunds = 0;
	int dmg = 0;
	int rolls = 1;
	int plev = p_ptr->lev;

	char m_name[MAX_NLEN];
	monster_desc(m_name, m_ptr, 0);

	rolls = _get_curse_rolls(pow);
	if (one_in_(3)) rolls++;

	while (ct <= rolls){
		ct++;
		dmg = plev + _curse_boost_capped;
		dType = -1;
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
			if (p == 6 || p == 3 || p == 7){ refunds++; continue; }
		}
		if (dType > 0){
			project(0, 0, m_ptr->fy, m_ptr->fx, dmg, dType, PROJECT_KILL | PROJECT_HIDE | PROJECT_NO_PAIN | PROJECT_SHORT_MON_NAME | PROJECT_JUMP, -1);
		}
	}
	// In addition to these things, we also have some damage. Depends a lot on highest power.
	if (DoDamage){
		dmg += (plev / 2) * refunds + plev * pow;
		project(0, 0, m_ptr->fy, m_ptr->fx, dmg, GF_NETHER, PROJECT_KILL | PROJECT_HIDE | PROJECT_NO_PAIN | PROJECT_SHORT_MON_NAME | PROJECT_JUMP, -1);
	}

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

	return TRUE;
}

static bool _inflict_curse(int pow){ 
	int m_idx = 0;
	monster_type *m_ptr;
	char m_name[MAX_NLEN];
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
			_inflict_curse_aux(pow, m_ptr, m_idx, TRUE);
			energy_use = 100;
			return TRUE;
		}

		return FALSE;
}

static bool _blasphemy(void){

	int afflicted = 0;
	monster_type *m_ptr;
	msg_print("You utter an ancient and terrible word.");

	for (int i = 1; i < m_max; i++)
	{
		m_ptr = &m_list[i];
		if (player_has_los_bold(m_ptr->fy, m_ptr->fx)){ // Not seen
			_inflict_curse_aux(1, m_ptr, i, FALSE);
			afflicted++;
		}

	}
	if (afflicted == 0){ msg_print("... But there's no one to listen."); return FALSE; }
	energy_use = 100;
	return TRUE;
}

/* BLASHPEMY */
static void _blasphemy_spell(int cmd, variant *res){
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Blasphemy"); break;
	case SPELL_INFO: var_set_string(res, info_power(_get_curse_rolls(1) * 10)); break;
	case SPELL_DESC: var_set_string(res, "Utter an accursed word, inflicting curse on all monsters."); break;
	case SPELL_CAST: 
		if (_blasphemy()){
			project(0, 4, py, px, damroll(p_ptr->lev, 8), GF_NETHER, (PROJECT_FULL_DAM | PROJECT_KILL | PROJECT_SHORT_MON_NAME), -1);
			var_set_bool(res, TRUE);
		} else var_set_bool(res, FALSE);
		break;
	default:default_spell(cmd, res); break;
	}
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
	case SPELL_DESC: var_set_string(res, "Senses monsters and traps in range. At high level, also maps the area."); break;
	case SPELL_INFO: var_set_string(res, info_radius(26 + _curse_boost_capped)); break;
	case SPELL_CAST: 
		msg_print("You attempt to sense misfortune... \n");
		detect_monsters_evil( 26 + _curse_boost_capped );
		detect_traps(26 + _curse_boost_capped, FALSE);
		if (p_ptr->lev > 40){ map_area(26 + _curse_boost_capped); }
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
	case SPELL_CAST:{
		_toggle_spell(MALEDICT_TOGGLE_ANTITELE, cmd, res);
		if (_get_toggle() == MALEDICT_TOGGLE_ANTITELE) msg_print("Everything is locked down in space.");
		else msg_print("Dimensional anchoring vanishes.");
		var_set_bool(res, TRUE);
		break;
	}
	default:default_spell(cmd, res); break;
	}
}

static void _karmic_balance(int cmd, variant *res)
{
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Karmic Balance"); break;
	case SPELL_DESC: var_set_string(res, "Purge all curses from equipment to heal self."); break;
	case SPELL_INFO: var_set_string(res, info_heal(0, 0, _curse_boost_capped * 70)); break;
	case SPELL_CAST:{
		int old_cursepow = _curse_boost_capped;
		if (old_cursepow == 0){ msg_print("You are not carrying any dispellable curses. "); var_set_bool(res, FALSE); break; }
		msg_print("You are enveloped in good karma. ");
		hp_player(spell_power(old_cursepow *100));
		remove_all_curse();
		if (old_cursepow >= 3){
			set_stun(0, TRUE);
			set_cut(0, TRUE);
			set_blind(0, TRUE);
			energy_use = 10; // super-quick too.
		}
		var_set_bool(res, TRUE);
		break;
	}
	default:default_spell(cmd, res); break;
	}
}

static void _drain_curse_pow(int cmd, variant *res){
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Drain Curse Power"); break;
	case SPELL_DESC: var_set_string(res, "Drains curse equipment to replenish mana."); break;
	case SPELL_INFO: var_set_string(res, info_power(p_ptr->lev / 2 + 15)); break;
	case SPELL_CAST:
	{
		int item;
		char *s, *q;
		u32b f[OF_ARRAY_SIZE];
		object_type *o_ptr;
		item_tester_hook = object_is_cursed;
		q = "Which cursed equipment do you drain mana from?";
		s = "You have no cursed equipment.";

		if (!get_item(&item, q, s, (USE_EQUIP))){
			var_set_bool(res, FALSE); break;
		}

		o_ptr = &inventory[item];
		obj_flags(o_ptr, f);

			if (_purge_curse_which(o_ptr,item)){
				p_ptr->csp += (p_ptr->lev / 2) + randint1(p_ptr->lev / 2);
				if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;
				var_set_bool(res, TRUE);
			}
			else{ 
				msg_print("Failed to drain curse.");  
				var_set_bool(res, FALSE);
			}
		 break;
	}
	default: default_spell(cmd, res); break;
	}
}

static void _umbra_spell(int cmd, variant *res){
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Umbra"); break;
	case SPELL_DESC: var_set_string(res, "Shroud yourself in shadows, making you more difficult to detect."); break;
	case SPELL_INFO: var_set_string(res, info_duration(p_ptr->lev / 3 + _curse_boost_capped * 3, p_ptr->lev / 3 + _curse_boost_capped * 3)); break;
	case SPELL_CAST:
		set_tim_dark_stalker(spell_power(p_ptr->lev / 3 + _curse_boost_capped * 3 + randint0(p_ptr->lev / 3 + _curse_boost_capped * 3)), FALSE);
		var_set_bool(res, TRUE);
		break;
	default:default_spell(cmd, res); break;
	}
}

static bool _unleash(void){
	
	msg_print("All malice is released!");

	if(_curse_boost_capped >= 6) cast_destruction();
	if(_curse_boost_capped >= 5) project_hack(GF_BLOOD_CURSE, _curse_boost_capped * 25);
	project(0, 10, py, px, damroll(_curse_boost_capped, 25), GF_MANA, (PROJECT_FULL_DAM | PROJECT_KILL | PROJECT_SHORT_MON_NAME), -1);
	hp_player(spell_power(_curse_boost_capped * 50));
	set_stun(0, TRUE);
	set_cut(0, TRUE);
	set_blind(0, TRUE);

	remove_all_curse();
	energy_use = 10;
	return TRUE;

}

static void _unleash_spell(int cmd, variant *res){
	switch (cmd){
	case SPELL_NAME: var_set_string(res, "Unleash Malice"); break;
	case SPELL_DESC: var_set_string(res, "Unleash all the misery and malice in your equipment."); break;
	case SPELL_INFO: {
		if (_curse_boost_capped > 2){
			var_set_string(res, info_power(p_ptr->lev * 2 + _curse_boost_capped * 20));
		}
		else var_set_string(res, info_power(0));
		break;
	}
	case SPELL_CAST:{ 
		if (_curse_boost_capped > 2) var_set_bool(res, _unleash()); 
		else msg_print("There isn't enough malice in you... "); var_set_bool(res, FALSE); 
		break;
	}
	default:default_spell(cmd, res); break;
	}
}
static int _get_powers(spell_info* spells, int max)
{
	int ct = 0;

	spell_info* spell = &spells[ct++];
	spell->level = 3;
	spell->cost = 0;
	spell->fail = calculate_fail_rate(spell->level, 20, p_ptr->stat_ind[A_CHR]);
	spell->fn = _purge_curse_spell;

	spell = &spells[ct++];
	spell->level = 7;
	spell->cost = 0;
	spell->fail = calculate_fail_rate(spell->level, 35, p_ptr->stat_ind[A_CHR]);
	spell->fn = _curse_item_spell;

	spell = &spells[ct++];
	spell->level = 30;
	spell->cost = 30;
	spell->fail = calculate_fail_rate(spell->level, 70, p_ptr->stat_ind[A_CHR]);
	spell->fn = _curse_item_all_spell;


	return ct;
}

bool maledict_ty_protection(void){
	if (p_ptr->pclass != CLASS_MALEDICT) return FALSE;
	if (p_ptr->lev > 30){
		if (one_in_(2)) return TRUE;
	}
	return FALSE;
}

int maledict_get_toggle(void)
{
	int result = TOGGLE_NONE;
	if (p_ptr->pclass == CLASS_MALEDICT)
		result = _get_toggle();
	return result;
}

static spell_info _spells[] =
{
	/*lvl cst fail spell */
	{ 1, 10, 30, _minor_curse }, // debuff
	{ 5, 5, 40, _sense_misfortune }, // detect traps / monsters
	{ 10, 15, 40, _curse_of_impotence}, // no breeding
	{ 15, 8, 45,  _drain_curse_pow }, // attempts to remove curse from one equipment, restores mana based on curses.
	{ 20, 20, 45,  _umbra_spell}, // Stealth buff, cancels out aggravation
	{ 25, 30, 45, _curse_spell}, // stronger debuff + damage
	{ 30, 0, 0, _dimensional_anchor }, // -TELE on self and everyone on sight
	{ 35, 30, 45, _karmic_balance  }, // Remove curse & heal
	{ 40, 110, 45, _blasphemy_spell },
	{ 45, 60, 45, _major_curse }, // crippling debuff
	{ 50, 100, 50, _unleash_spell }, // *remove curse, heal, LOS effects depending on the curse_power. Requires curses to be present.
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

	boost = _calculate_curse_power();

	_curse_boost = boost; // save up the uncapped boost.
	// 22 is true max number of curse flags that you can have, totaling to score of 38. 
	// To gain full power, The boost itself scales up to 15.
	boost = (boost*2) / 3;
	int boost_cap = 5 + p_ptr->lev / 5;
	if (boost > boost_cap) boost = boost_cap;

	_curse_boost_capped = boost;
	p_ptr->pspeed += boost / 3;

	p_ptr->weapon_info[0].xtra_blow += py_prorata_level_aux(boost*10, 0, 1, 1);
	p_ptr->weapon_info[1].xtra_blow += py_prorata_level_aux(boost*10, 0, 1, 1);

	p_ptr->weapon_info[0].to_h += boost; p_ptr->weapon_info[1].to_h += boost;
	p_ptr->to_h_m += boost;
	p_ptr->weapon_info[0].dis_to_h += boost; p_ptr->weapon_info[1].dis_to_h += boost;

	p_ptr->weapon_info[0].to_d += boost; p_ptr->weapon_info[1].to_d += boost;
	p_ptr->to_d_m += boost;
	p_ptr->weapon_info[0].dis_to_d += boost; p_ptr->weapon_info[1].dis_to_d += boost;

	if (maledict_get_toggle() == MALEDICT_TOGGLE_ANTITELE) p_ptr->anti_tele = TRUE;
}

static caster_info * _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "malison";
		me.which_stat = A_CHR;
		me.weight = 4000;
		me.min_fail = 5;
		init = TRUE;
	}
	return &me;
}

static void _birth(void)
{
	py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
	py_birth_obj_aux(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL, 1);
	py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

class_t *maledict_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	if (!init)
	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 20, 24, 28, 1, 12, 2, 68, 51 };
		skills_t xs = { 7, 10, 10, 0, 0, 0, 21, 21 };

		me.name = "Maledict";
		me.desc = "A maledict draws their powers from the misfortunes, bad omens and ill luck. "
			"They become more powerful the more cursed their equipment, and can direct debilating "
			"malisons at their foes. Notably, they have very easy time uncursing equipment, and can "
			"curse it again if they feel like it. They even gain some protection against ancient foul "
			"curses. At high levels, they receive abilities to uncurse all their equipment for a great "
			"benefit.\n"
			"{ EXPERIMENTAL CLASS }";
			

		me.stats[A_STR] = 2;
		me.stats[A_INT] = -1;
		me.stats[A_WIS] = -2;
		me.stats[A_DEX] = 1;
		me.stats[A_CON] = 1;
		me.stats[A_CHR] = 3;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 105;
		me.base_hp = 12;
		me.exp = 135;
		me.pets = 30;

		me.calc_bonuses = _calc_bonuses;
		me.caster_info = _caster_info;
		me.get_spells = _get_spells;
		me.get_powers = _get_powers;
		me.birth = _birth;
		init = TRUE;
	}

	return &me;
}

