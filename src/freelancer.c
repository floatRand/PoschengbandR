//#include "angband.h"
//
//typedef struct {
//	int prof;
//	int cost;
//	int lvCost;
//	int lv;
//	int maxLevel;
//	int minPLev;
//	cptr name;
//} fl_proficiency, *fl_proficiency_ptr;
//
//
////PROFICIENCIES
///* Realms, skills, such. */
//#define _FL_LEARN_REALM				 0
//#define _FL_LEARN_WEAPON			 1
//#define _FL_LEARN_DUAL_WIELDING		 2
//#define _FL_LEARN_RIDING			 3
//#define _FL_LEARN_MARTIAL_ARTS		 4
///*Boosts*/
//#define _FL_IDX_BOOST_FIRST			 5 // index of first entry
//#define _FL_BOOST_HP				 5
//#define _FL_BOOST_SP				 6
//#define _FL_BOOST_MELEE				 7
//#define _FL_BOOST_RANGED		     8
//#define _FL_BOOST_MAGIC				 9
//#define _FL_BOOST_SKILL				 10
///*Flags*/
//#define _FL_IDX_FLAG_FIRST			 11 // index of first entry
//#define _FL_F_NO_GLOVE_ENCUMBERANCE  11
//#define _FL_F_ALLOW_DEC_MANA		 12
//#define _FL_F_SNEAK_ATTACKS			 13
//#define _FL_F_HEAVY_ARMOR_CASTING	 14
//#define _FL_F_AUTO_IDENTIFY			 15
///*Activated*/
//#define _FL_IDX_ABILITY_FIRST		 16 // index of first entry
//#define _FL_A_EATMAGIC				 16
//#define _FL_A_HP_TO_SP				 17
//#define _FL_A_IDENTIFY_TRUE			 18
//#define _FL_A_RECALL				 19
//
//#define _FL_NULL				     20
//
//#define _FL_MAX_ACTIVATEABLES		 4 // for listing special abilities
//
//#define _FL_MAX_PROF				 21 // maximum.
//static const int _prof_progression = 1; // 1 point every levels, 1 at first level
//static int _prof_points = 1;
//
//
//static fl_proficiency _proficiencies[_FL_MAX_PROF] = {
////IDX,cost, leveling cost, lv, mxlv, minimum plv
//{ _FL_LEARN_REALM,				1, 1, 0, 1,	1, "Learn a magic realm" },
//{ _FL_LEARN_WEAPON,				1, 1, 0, 1,	1, "Learn a weapon group skill" },
//{ _FL_LEARN_DUAL_WIELDING,		1, 1, 0, 1,	1, "Learn dual wielding skill" },
//{ _FL_LEARN_RIDING,				1, 1, 0, 1,	1, "Learn riding skill" },
//{ _FL_LEARN_MARTIAL_ARTS,		1, 1, 0, 1, 1, "Learn martial arts" },
//
//{ _FL_BOOST_HP,					1, 1, 0, 1,	1, "Boost HP" },
//{ _FL_BOOST_SP,					1, 1, 0, 1,	1, "Boost SP" },
//{ _FL_BOOST_MELEE,				1, 1, 0, 1, 1, "Boost melee attacks" },
//{ _FL_BOOST_RANGED,				1, 1, 0, 1, 1, "Boost ranged attacks" },
//{ _FL_BOOST_MAGIC,				1, 1, 0, 1, 1, "Boost casting" },
//{ _FL_BOOST_SKILL,				1, 1, 0, 1, 1, "Boost invidual skill" },
//
//{ _FL_F_NO_GLOVE_ENCUMBERANCE,	1, 1, 0, 1, 1, "Ignore glove encumbrance" },
//{ _FL_F_ALLOW_DEC_MANA,			1, 1, 0, 1,	1, "Allow dec_mana" },
//{ _FL_F_SNEAK_ATTACKS,			1, 1, 0, 1,	1, "Gain sneak attacking" },
//{ _FL_F_HEAVY_ARMOR_CASTING,	1, 1, 0, 1, 1, "Improve casting in heavy armour" },
//{ _FL_F_AUTO_IDENTIFY,			1, 1, 0, 1, 1, "Identify items automatically" },
//
//{ _FL_A_EATMAGIC,				1, 1, 0, 1, 1, "Eat magical charges for SP" },
//{ _FL_A_HP_TO_SP,				1, 1, 0, 1, 1, "Convert HP to SP" },
//{ _FL_A_IDENTIFY_TRUE,			1, 1, 0, 1, 1, "Learn innate *Identity" },
//{ _FL_A_RECALL,					1, 1, 0, 1, 1, "Learn Recall" },
//
//{ _FL_NULL, 0,0,0,0,0, "" } // for bad returns.
//};
//
//
//typedef struct {
//	int realmID;
//	int lv;
//	int cost;
//	int cast_stat;
//} _fl_realm_info, *_fl_realm_info_ptr;
//
//// specialty realms require the actual casting stat in order.
//static _fl_realm_info _realmInfo[MAX_REALM] = {
//	{ REALM_NATURE,		0,	1, -1 },
//	{ REALM_CRAFT,		0,	1, -1 },
//	{ REALM_DAEMON,		0,	1, -1 },
//	{ REALM_LIFE,		0,	1, -1 },
//	{ REALM_DEATH,		0,	1, -1 },
//	{ REALM_CHAOS,		0,	1, -1 },
//	{ REALM_TRUMP,		0,	1, -1 },
//	{ REALM_CRUSADE,	0,	1, -1 },
//	{ REALM_HISSATSU,	0,	1, A_WIS },
//	{ REALM_MUSIC,		0,	1, A_CHR },
//	{ REALM_HEX,		0,	1, A_INT },
//	{ REALM_NECROMANCY, 0,	1, -1 },
//	{ REALM_RAGE,		0,	1, -1 },
//	{ REALM_ARCANE,		0,	1, -1 },
//	{ REALM_BURGLARY,	0,	1, A_DEX },
//	{ REALM_ARMAGEDDON, 0,	1, -1 }, 
//};
//
//typedef struct {
//	int s_id;
//	int lv;
//} _fl_skill_boost, *_fl_skill_boost_ptr;
//
//#define _FL_SK_DIS 0
//#define _FL_SK_DEV 1
//#define _FL_SK_SVA 2
//#define _FL_SK_STL 3
//#define _FL_SK_SRH 4
//#define _FL_SK_FOS 5
//#define _FL_SK_THN 6
//#define _FL_SK_THB 7
//#define _FL_SK_MAX 8
///* dis, dev, sav, stl, srh, fos, thn, thb */
//
//static _fl_skill_boost _skillBoosts[_FL_SK_MAX] = {
//	{_FL_SK_DIS, 0},
//	{_FL_SK_DEV, 0},
//	{_FL_SK_SVA, 0},
//	{_FL_SK_STL, 0},
//	{_FL_SK_SRH, 0},
//	{_FL_SK_FOS, 0},
//	{_FL_SK_THN, 0},
//	{_FL_SK_THB, 0},
//};
//
//fl_proficiency *_fetch_proficiency(int prof){
//	if (!(prof > 0 || prof < _FL_MAX_PROF)){
//		if (prof == _proficiencies[prof].prof) return _proficiencies + (prof);
//		// something went wrong, use longer vers.
//		for (int i = 0; i < _FL_MAX_PROF; i++){
//			if (prof == _proficiencies[prof].prof) return _proficiencies + (prof);
//		}
//	}
//	return _proficiencies + (_FL_MAX_PROF - 1); // return null prof if something went wrong
//	
//}
//
//static void _birth(void){
//	/* Init all. */
//	_prof_points = 1; // beanie points;
//
//	// reset all of these things.
//	for (int i = 0; i < _FL_MAX_PROF; i++){
//		(_proficiencies + i)->lv = 0;
//	}
//	for (int i = 0; i < MAX_REALM; i++){
//		(_realmInfo + i)->lv = 0;
//	}
//	for (int i = 0; i < _FL_SK_MAX; i++){
//		(_skillBoosts + i)->lv = 0;
//	}
//
//}
//
//
//static int _calc_Upgrade_Cost(int i){
//	int cost = _fetch_proficiency(i)->lvCost * _fetch_proficiency(i)->lv;
//		return cost;
//}
//
//static void _displayProficiencies(rect_t display)
//{
//	char    buf[MAX_NLEN];
//	int     i;
//	point_t pos = rect_topleft(&display);
//	int     padding, max_o_len = 20;
//	doc_ptr doc = NULL;
//	fl_proficiency *list = _proficiencies;
//	int DC = 1;
//
//	padding = 5;   /* leading " a) " + trailing " " */
//	padding += 12; /* " Count " */
//
//	/* Measure */
//	for (i = 0; i < _FL_MAX_PROF; i++)
//	{
//		fl_proficiency *prof_ptr = list + i;
//		if (prof_ptr->prof != _FL_NULL)
//		{
//			int len;
//			strcpy(buf, prof_ptr->name);
//			len = strlen(buf);
//			if (len > max_o_len)
//				max_o_len = len;
//		}
//	}
//
//	if (max_o_len + padding > display.cx)
//		max_o_len = display.cx - padding;
//
//	/* Display */
//	doc = doc_alloc(display.cx);
//	doc_insert(doc, "<style:table>");
//	for (i = 0; i < _FL_MAX_PROF-1; i++)
//	{
//		fl_proficiency *prof_ptr = list + i;
//
//		switch (i){ // categories neatly.
//			case 0: doc_printf(doc, "[ General: ] \n"); break;
//			case _FL_IDX_ABILITY_FIRST:	doc_printf(doc, "[ Abilities: ] \n"); break;
//			case _FL_IDX_BOOST_FIRST:	doc_printf(doc, "[ Boosts: ] \n"); break;
//			case _FL_IDX_FLAG_FIRST:	doc_printf(doc, "[ Traits: ] \n"); break;
//		}
//
//		doc_printf(doc, " %c) ", I2A(i));
//
//		if (prof_ptr->prof != _FL_NULL)
//		{
//			int prlv = prof_ptr->lv;
//			int cost = prof_ptr->cost;
//			if (prlv > 1) cost = _calc_Upgrade_Cost(i);
//
//				if (prof_ptr->minPLev <= p_ptr->lev){
//					if (prlv == 0){ // display unpurchased ones with white 
//						doc_insert_text(doc, TERM_L_WHITE, prof_ptr->name);
//						if (i > _FL_LEARN_WEAPON) doc_printf(doc, "<tab:%d>Unpurchased, Cost:%2d%\n",
//							display.cx - 22, cost);
//						else doc_insert_text(doc, TERM_L_DARK, "\n");
//					}
//					else if (prlv > 0 && prlv < prof_ptr->maxLevel){ // dgreen
//						doc_insert_text(doc, TERM_GREEN, prof_ptr->name);
//						doc_printf(doc, "<tab:%d><color:g>Level: %2d%, Cost:%2d%</color>\n",
//							display.cx - 22, cost);
//					}
//					else { // capped
//						doc_insert_text(doc, TERM_YELLOW, prof_ptr->name);
//						doc_printf(doc, "<tab:%d><color:y>Max level</color>\n",
//							display.cx - 22);
//					}
//				}
//				else{
//					doc_insert_text(doc, TERM_L_DARK, prof_ptr->name);
//					doc_printf(doc, "<color:d>Minimum level requirement: %2d%</color>\n",
//						display.cx - 22, prof_ptr->minPLev);
//				}
//			
//
//			///*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
//		}
//	
//	}
//
//	doc_printf(doc, "[ Proficiency points: %d ]", _prof_points);
//
//	doc_insert(doc, "</style>");
//	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
//	doc_free(doc);
//}
//
//
//
//int _chooseProf(int options)
//{
//	int			 chosen_prof = _FL_NULL;
//	int          cmd;
//	rect_t       display = ui_menu_rect();
//	string_ptr   prompt = NULL;
//	bool         done = FALSE;
//	bool         exchange = FALSE;
//
//	if (display.cx > 80)
//		display.cx = 80;
//
//	prompt = string_alloc();
//	screen_save();
//
//	while (!done)
//	{
//		string_clear(prompt);
//
//		string_printf(prompt, "Learn which proficiency? :");
//
//		prt(string_buffer(prompt), 0, 0);
//		_displayProficiencies(display);
//
//		cmd = inkey_special(FALSE);
//
//		if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q')
//			done = TRUE;
//
//		if ('a' <= cmd && cmd < 'a' + _FL_MAX_PROF)
//		{
//			chosen_prof = A2I(cmd);
//			if (chosen_prof != _FL_NULL) done = TRUE;
//		}
//	}
//
//	screen_load();
//	string_free(prompt);
//	return chosen_prof;
//}
//
//static void _choose_prof_spell(int cmd, variant *res)
//{
//	switch (cmd)
//	{
//	case SPELL_NAME:
//		var_set_string(res, "Learn Proficiency");
//		break;
//	case SPELL_DESC:
//		var_set_string(res, "Learn new skill yo.");
//		break;
//	case SPELL_CAST:
//		_chooseProf(0);
//		var_set_bool(res, TRUE);
//		break;
//	default:
//		default_spell(cmd, res);
//		break;
//	}
//}
//
//
//static int _get_powers(spell_info* spells, int max)
//{
//	int ct = 0;
//	
//	spell_info* spell = &spells[ct++];
//	spell->level = 1;
//	spell->cost = 0;
//	spell->fail = 0;
//	spell->fn = _choose_prof_spell;
//	
//	return ct;
//}
//
//int _castingStat(void){
//	int res = A_INT; // return highest
//			if (p_ptr->stat_use[res] < p_ptr->stat_use[A_CHR]) res = A_CHR;
//			if (p_ptr->stat_use[res] < p_ptr->stat_use[A_WIS]) res = A_WIS;
//	return res;
//}
//
//
//	
//
//
//int _minFail(void){
//	//return MAX(1, _minFailRate);
//}
//
//int _castWeightLimit(void){
//	int lv = _fetch_proficiency(_FL_F_HEAVY_ARMOR_CASTING)->lv;
//	if (lv > 0) return 700 + lv * 100;
//		return 420;
//}
//
//u32b _casterFlags(void){
//
//	u32b castFlags = 0;
//	if (_fetch_proficiency(_FL_F_NO_GLOVE_ENCUMBERANCE)->lv > 0) castFlags |= CASTER_GLOVE_ENCUMBRANCE;
//	if (_fetch_proficiency(_FL_F_ALLOW_DEC_MANA)->lv > 0) castFlags |= CASTER_ALLOW_DEC_MANA;
//	return castFlags;
//}
//
//void freelancer_skill_boost(void){
//	/*boost skills here*/
//
//}
//
//int freelancer_sp_boost(void){
//	if (_fetch_proficiency(_FL_BOOST_SP)->lv > 0){
//		return _fetch_proficiency(_FL_BOOST_SP)->lv * p_ptr->lev; // 1 per level.
//	}
//	return 0;
//}
//
//int freelancer_hp_boost(void){
//	if (_fetch_proficiency(_FL_BOOST_HP)->lv > 0){
//		return _fetch_proficiency(_FL_BOOST_HP)->lv * p_ptr->lev * 2; // 2 per level. Still, it's cheap! 
//	}
//	return 0;
//}
//
//int freelancer_stab_level(){
//	int stabbylv = _fetch_proficiency(_FL_F_SNEAK_ATTACKS)->lv;
//	return stabbylv;
//}
// 
//static void _calc_bonuses(void)
//{
//	if (_fetch_proficiency(_FL_F_AUTO_IDENTIFY)->lv > 0) p_ptr->auto_id = TRUE;
//}
//
//static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
//{
//
//}
//
//static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
//{
//	int meleeBoost = _fetch_proficiency(_FL_BOOST_MELEE)->lv;
//	if (meleeBoost > 0){
//		info_ptr->to_d += p_ptr->lev / (9 - meleeBoost);
//		info_ptr->dis_to_d += p_ptr->lev / (9 - meleeBoost);
//		info_ptr->xtra_blow += py_prorata_level_aux(60+meleeBoost*10, 0, 1, 1);
//	}
//}
//
//static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
//{
//	int rangedBoost = _fetch_proficiency(_FL_BOOST_RANGED)->lv;
//	// we don't really care what we shoot with, unlike rangers. We do worse, though.
//	if (rangedBoost > 0){
//		p_ptr->shooter_info.num_fire += (p_ptr->lev * 50 + rangedBoost * 25) / 50;
//	}
//}
//
//static caster_info* _caster_info(void)
//{
//	static caster_info me = { 0 };
//	static bool init = FALSE;
//	if (!init)
//	{
//		me.magic_desc = "knapsack";
//		me.which_stat = _castingStat;
//		me.weight = _castWeightLimit;
//		me.min_fail = _minFail;
//		me.options = _casterFlags;
//		init = TRUE;
//	}
//	return &me;
//}
//
//class_t *freelancer_get_class(void)
//{
//	static class_t me = { 0 };
//	static bool init = FALSE;
//
//	if (!init)
//	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
//		skills_t bs = { 15, 18, 28, 1, 12, 2, 48, 20 };
//		skills_t xs = { 5, 7, 9, 0, 0, 0, 13, 11 };
//
//		me.name = "Freelancer";
//		me.desc = "Freelancers are wanderers who pick up skills as they "
//			"gain experience. Their flexibility is their greatest strength: "
//			"while weak at beginning, they gain proficiency points that "
//			"they can use to purchase new abilities: skills, realms, etc. "
//			"However, due to not being as focused as others, these abilities "
//			"are somewhat weaker.\n"
//			"Their highest mental ability determines their casting ability.";
//
//		me.stats[A_STR] = 0;
//		me.stats[A_INT] = 0;
//		me.stats[A_WIS] = 0;
//		me.stats[A_DEX] = 0;
//		me.stats[A_CON] = 0;
//		me.stats[A_CHR] = 0;
//		me.base_skills = bs;
//		me.extra_skills = xs;
//		me.life = 100;
//		me.base_hp = 8;
//		me.exp = 100;
//		me.pets = 40;
//
//		me.birth = _birth;
//		me.caster_info = _caster_info;
//		me.calc_bonuses = _calc_bonuses;
//		me.get_flags = _get_flags;
//		me.calc_weapon_bonuses = _calc_weapon_bonuses;
//		me.calc_shooter_bonuses = _calc_shooter_bonuses;
//		/* TODO: This class uses spell books, so we are SOL
//		me.get_spells = _get_spells;*/
//		me.get_powers = _get_powers;
//		me.character_dump = spellbook_character_dump;
//		init = TRUE;
//	}
//
//	return &me;
//} 
