#include "angband.h"

static int _curse_boost = 0;
static int _curse_ty_count = 0;

bool _purge_curse(void){

	int slot, item;
	int ct = 0;
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


static spell_info _spells[] =
{
	/*lvl cst fail spell */
	//{ 1, 2, 30, _bolt_spell },
	//{ 3, 3, 40, _regeneration_spell },
	//{ 6, 4, 40, _foretell_spell },
	//{ 8, 8, 50, _quicken_spell },
	//{ 10, 9, 50, _withering_spell },
	//{ 13, 10, 50, _blast_spell },
	//{ 17, 12, 50, _back_to_origins_spell },
	//{ 23, 15, 60, _haste_spell },
	//{ 27, 20, 60, _wave_spell },
	//{ 30, 10, 60, _shield_spell },
	//{ 33, 50, 70, _rewind_time_spell },
	//{ 35, 35, 70, _breath_spell },
	//{ 37, 50, 70, _remember_spell },
	//{ 39, 30, 70, _stasis_spell },
	//{ 41, 20, 80, _travel_spell },
	//{ 45, 80, 80, _double_move_spell },
	//{ 49, 100, 80, _foresee_spell },
	{ -1, -1, -1, NULL }
};




static void _purge_curse_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Purge Curse");
		break;
	case SPELL_DESC:
		var_set_string(res, "Removes a curse from a single item. At high levels you can dispel even more powerful curses.");
		break;
	case SPELL_CAST:
		var_set_bool(res, FALSE);
		if (_purge_curse()) var_set_bool(res, TRUE);
				else var_set_bool(res, FALSE);
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

static void _invoke_curse_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Invoke Curse");
		break;
	case SPELL_DESC:
		var_set_string(res, "Inflicts curse on the enemy, depending on your own curses.");
		break;
	case SPELL_CAST:
		var_set_bool(res, FALSE);
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

static void _curse_item_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Curse Item");
		break;
	case SPELL_DESC:
		var_set_string(res, "Curses a single item.");
		break;
	case SPELL_CAST:
		var_set_bool(res, FALSE);
		if (_curse_item_aux()) var_set_bool(res, TRUE);
		break;
	default:
		default_spell(cmd, res);
		break;
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
	spell->level = 6;
	spell->cost = 20;
	spell->fail = calculate_fail_rate(spell->level, 10, p_ptr->stat_ind[A_CHR]);
	spell->fn = _invoke_curse_spell;

	spell = &spells[ct++];
	spell->level = 12;
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

static void _calc_bonuses(void)
{
	int slot;
	int boost = 0;
	_curse_ty_count = 0;

	u32b checklist = 0; 

	for (slot = equip_find_first(object_is_cursed); slot; slot = equip_find_next(object_is_cursed, slot))
	{
		object_type *o_ptr = equip_obj(slot);
		checklist |= (o_ptr->curse_flags);
	}

	boost = _count_flags_set(checklist);
		//SPECIAL ONES
	if (checklist & OFC_PERMA_CURSE) boost+=5;
	if (checklist & OFC_TY_CURSE) boost+=4;
	if (checklist & OFC_AGGRAVATE) boost+=3;
	//Other nasty things
	if (checklist & (OFC_HEAVY_CURSE | OFC_DRAIN_EXP | OFC_DRAIN_HP | OFC_DRAIN_MANA)) boost++;
	
	_curse_boost = boost; // save up the uncapped boost.
	// 22 is true max number of curse flags that you can have, totaling to score of 38. 
	// To gain full power, The boost itself scales up to 15.
	boost = (boost*2) / 3;
	int boost_cap = 5 + p_ptr->lev / 5;

	if (boost > boost_cap) boost = boost_cap;

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
		/* TODO: This class uses spell books, so we are SOL
		me.get_spells = _get_spells;*/
		me.get_powers = _get_powers;
		me.character_dump = spellbook_character_dump;
		init = TRUE;
	}

	return &me;
}

