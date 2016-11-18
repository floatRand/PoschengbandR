#include "angband.h"


static const int _prof_progression = 1; // 1 point every levels, 1 at first level
static int _prof_points = 1;
static int _p_ct = 0; // number of purchases. Used for iterating.
#define _FL_MAX_BUYS 256 // maximum number of purchases. Shouln't be really reached...

/* On structuring:
This is a hot mess.

Basically, how it works that _proficiencies, _realmInfo and wpn-stuff is static info related to purchasing.
The levels of them are in int-arrays, that are calculated with purchase-history.
Basically, each purchase nets an entry, and entries are totaled for level.
This is so we can handle situations where levels are drained. And reset will be slightly easier.

And hey, only purchases need to be saved!
*/

static skills_t _skill_boost = { 0, 0, 0, 0, 0, 0, 0, 0 };
static skills_t _base_skills = { 0, 0, 0, 0, 0, 0, 0, 0 }; // for sake of displaying base levels.

// the skills are represented in different order. Basically just points into right directions....
#define _SKILL_MAX 8 
#define _SKILL_THN 0
#define _SKILL_THB 1
#define _SKILL_SAV 2
#define _SKILL_STL 3
#define _SKILL_FOS 4
#define _SKILL_SRH 5
#define _SKILL_DIS 6
#define _SKILL_DEV 7

static int _skillLevels[_SKILL_MAX];
static int _skillCosts[_SKILL_MAX] = { 1,1,1,1,1,1,1,1 };
static int _skillSteps[_SKILL_MAX] = { 36, 36, 21, 3, 18, 18, 24, 18 };
static cptr _skillNames[_SKILL_MAX] = {"melee", "ranged", "saving throw", "stealth", "perception", "searching", "disarming", "device" };

typedef struct {
	int prof;
	int cost;
	int lvCost;
	int maxLevel;
	int minPLev;
	cptr name;
} fl_proficiency, *fl_proficiency_ptr;


//PROFICIENCIES
/* Realms, skills, such. */
#define _FL_LEARN_REALM				 0
#define _FL_LEARN_WEAPON			 1
#define _FL_LEARN_DUAL_WIELDING		 2
#define _FL_LEARN_RIDING			 3
#define _FL_LEARN_MARTIAL_ARTS		 4
/*Boosts*/
#define _FL_IDX_BOOST_FIRST			 5 // index of first entry
#define _FL_BOOST_HP				 5
#define _FL_BOOST_SP				 6
#define _FL_BOOST_MELEE				 7
#define _FL_BOOST_BLOWS				 8
#define _FL_BOOST_RANGED		     9
#define _FL_BOOST_MAGIC				 10
#define _FL_BOOST_SKILL				 11
/*Flags*/
#define _FL_IDX_FLAG_FIRST			 12 // index of first entry
#define _FL_F_SNEAK_ATTACKS			 12
#define _FL_F_HEAVY_ARMOR_CASTING	 13
#define _FL_F_AUTO_IDENTIFY			 14

#define _FL_NULL				     15
#define _FL_MAX_PROF				 16 // maximum.

static fl_proficiency _proficiencies[_FL_MAX_PROF] = {
//IDX,cost, leveling cost, mxlv, minimum plv
{ _FL_LEARN_REALM,				1, 1, 2, 1, "Learn a magic realm"},
{ _FL_LEARN_WEAPON,				1, 1, 2, 1, "Learn a weapon group skill" },
{ _FL_LEARN_DUAL_WIELDING,		1, 1, 3, 1, "Learn dual wielding skill" },
{ _FL_LEARN_RIDING,				1, 1, 3, 1, "Learn riding skill" },
{ _FL_LEARN_MARTIAL_ARTS,		1, 1, 3, 1, "Learn martial arts" },

{ _FL_BOOST_HP,					1, 1, 5, 1, "Boost HP" },
{ _FL_BOOST_SP,					1, 1, 5, 1, "Boost SP" },
{ _FL_BOOST_MELEE,				1, 1, 5, 1, "Boost melee attacks" },
{ _FL_BOOST_BLOWS,				1, 1, 5, 1, "Boost number of attacks" },
{ _FL_BOOST_RANGED,				1, 1, 5, 1, "Boost ranged attacks" },
{ _FL_BOOST_MAGIC,				1, 1, 5, 1, "Boost casting" },
{ _FL_BOOST_SKILL,				1, 1, 5, 1, "Boost invidual skill" },

{ _FL_F_SNEAK_ATTACKS,			1, 1, 3, 1, "Gain sneak attacking" },
{ _FL_F_HEAVY_ARMOR_CASTING,	1, 1, 3, 1, "Improve casting in heavy armour" },
{ _FL_F_AUTO_IDENTIFY,			1, 1, 1, 1, "Identify items automatically" },

{ _FL_NULL, 0,0,0,0, "" } // for bad returns.
};

typedef struct {
	int realmID;
	int cost;
	int cast_stat;
} _fl_realm_info, *_fl_realm_info_ptr;

static _fl_realm_info _realmInfo[MAX_REALM] = {
	{ REALM_NONE,		1, -1 },
	{ REALM_LIFE,		1, -1 },
	{ REALM_SORCERY,	1, -1 },
	{ REALM_NATURE,		1, -1 },
	{ REALM_CHAOS,		1, -1 },
	{ REALM_TRUMP,		1, -1 },
	{ REALM_ARCANE,		1, -1 },
	{ REALM_CRAFT, 		1, -1 },
	{ REALM_DAEMON,		1, -1 },
	{ REALM_CRUSADE,	1, -1 },
	{ REALM_NECROMANCY,	1, -1 }, 
	{ REALM_ARMAGEDDON, 1, -1 },
	{ REALM_MUSIC,		1, A_CHR },
	{ REALM_HISSATSU,	1, A_WIS },
	{ REALM_HEX,		-1, A_INT },
	{ REALM_RAGE,		1, A_STR }, 
	{ REALM_BURGLARY,	1, A_DEX },
};
 // +1 gives max lv 3, +2 give max lv 4
typedef struct {
	int tval;
	int cost;
	cptr name;
} _fl_wpn_info, *_fl_wpn_info_ptr;

#define _FL_TVAL_SWORD 0
#define _FL_TVAL_HAFTED 1
#define _FL_TVAL_POLEARM 2
#define _FL_TVAL_BOW 3
#define _FL_TVAL_NULL -1
#define _FL_TVAL_WPNMAX 5

static _fl_wpn_info _wpn_profs[_FL_TVAL_WPNMAX] = {
	{_FL_TVAL_SWORD, 1 ,"Blades"},
	{_FL_TVAL_HAFTED, 1 , "Hafted"},
	{_FL_TVAL_POLEARM, 1, "Polearms" },
	{_FL_TVAL_BOW, 1, "Ranged weapons"},
	{_FL_TVAL_NULL,-1, ""} // sentinel
};

typedef struct 
{
	int lv;
	int profID;
	int subID;
} _fl_purchase, *_fl_purchase_ptr;

static _fl_purchase _fl_purchases[_FL_MAX_BUYS];

static int _profLevels[_FL_MAX_PROF];
static int _realmLevels[MAX_REALM];
static int _wpnLevels[_FL_TVAL_WPNMAX];

static int fl_GetProfLevel(int profId, int subId){

	if (profId < 0 || profId >= _FL_MAX_PROF) return -1;

	if (profId == _FL_LEARN_REALM && subId>=0 && subId<MAX_REALM) return _realmLevels[subId];
	else if (profId == _FL_LEARN_WEAPON && subId >= 0 && subId<_FL_TVAL_WPNMAX) return _wpnLevels[subId];
	return _profLevels[profId];

}

fl_proficiency *_fetch_proficiency(int prof){
	if (!(prof > 0 || prof < _FL_MAX_PROF)){
		if (prof == _proficiencies[prof].prof) return _proficiencies + (prof);
		// something went wrong, use longer vers.
		for (int i = 0; i < _FL_MAX_PROF; i++){
			if (prof == _proficiencies[prof].prof) return _proficiencies + (prof);
		}

	}
	return _proficiencies + (_FL_MAX_PROF - 1); // return null prof if something went wrong
	
}

void _update_skills(void){
	skills_init(&_skill_boost);
	_skill_boost.dev = _get_skill_boost_amt(_SKILL_DEV);
	_skill_boost.dis = _get_skill_boost_amt(_SKILL_DIS);
	_skill_boost.fos = _get_skill_boost_amt(_SKILL_FOS);
	_skill_boost.sav = _get_skill_boost_amt(_SKILL_SAV);
	_skill_boost.srh = _get_skill_boost_amt(_SKILL_SRH);
	_skill_boost.stl = _get_skill_boost_amt(_SKILL_STL);
	_skill_boost.thb = _get_skill_boost_amt(_SKILL_THB);
	_skill_boost.thn = _get_skill_boost_amt(_SKILL_THN);

}

void freelancer_skill_boost(void){
	_update_skills();
	_base_skills = p_ptr->skills; // hopefully the mutability doesn't fuck me over.

	skills_add(&p_ptr->skills, &_skill_boost);
}

int _get_skill_boost_amt(int skillID){
	if (skillID < 0 || skillID>7) return 0;
	else return _skillLevels[skillID] * _skillSteps[skillID];
	return 0;
}

static int _calc_Upgrade_Cost(int i){
	int cost = _fetch_proficiency(i)->lvCost * fl_GetProfLevel(i,0);
		return cost;
}


static int _displayRealms(rect_t display, u16b mode)
{
	char    buf[MAX_NLEN];
	int     i, choices = 0;
	point_t pos = rect_topleft(&display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	_fl_realm_info *list = _realmInfo;

	padding = 5;   /* leading " a) " + trailing " " */
	padding += 12; /* " Count " */
	max_o_len = 12;

	if (max_o_len + padding > display.cx)
		max_o_len = display.cx - padding;

	/* Display */
	doc = doc_alloc(display.cx);
	doc_insert(doc, "<style:table>\n");
	for (i = 0; i < MAX_REALM; i++)
	{
		_fl_realm_info *info_ptr = list + i;
		if (info_ptr->cost > 0)
		{
			doc_printf(doc, "<tab:3> %c) ", I2A(i));
			int prlv = fl_GetProfLevel(_FL_LEARN_REALM, i);
			int cost = info_ptr->cost;
			//if (prlv > 1) cost = _calc_Upgrade_Cost(i);
				if (prlv == 0){ // display unpurchased ones with white 
					doc_insert_text(doc, TERM_L_WHITE, realm_names[info_ptr->realmID]);
					doc_printf(doc, "<tab:%d>Unpurchased, Cost:%2d%\n",display.cx - 28, cost);
				}
				else if (prlv > 0 && prlv < 3){ // dgreen
					doc_insert_text(doc, TERM_GREEN, realm_names[info_ptr->realmID]);
					doc_printf(doc, "<tab:%d><color:g>Level: %2d%, Cost:%2d%</color>\n",
						display.cx - 28, cost);
				}
				else { // capped
					doc_insert_text(doc, TERM_YELLOW, realm_names[info_ptr->realmID]);
					doc_printf(doc, "<tab:%d><color:y>Max level</color>\n",
						display.cx - 28);
				}
				choices++;
		}

	}
	doc_printf(doc, "\nProficiency points: %d\n", _prof_points);
	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);

	return choices;
}

static int _displayWpnGroups(rect_t display, u16b mode)
{
	char    buf[MAX_NLEN];
	int     i;
	int choices = 0;
	point_t pos = rect_topleft(&display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	_fl_wpn_info *list = _wpn_profs;
	

	padding = 5;   /* leading " a) " + trailing " " */
	padding += 12; /* " Count " */
	max_o_len = 12;
	if (max_o_len + padding > display.cx)
		max_o_len = display.cx - padding;

	/* Display */
	doc = doc_alloc(display.cx);
	doc_insert(doc, "<style:table>\n");

	for (i = 0; i < _FL_TVAL_WPNMAX; i++)
	{
		_fl_wpn_info *info_ptr = list + i;
		if (info_ptr->cost > 0)
		{
			doc_printf(doc, "<tab:3> %c) ", I2A(i));
			int prlv = fl_GetProfLevel(_FL_LEARN_WEAPON, i);
			int cost = info_ptr->cost;
			//if (prlv > 1) cost = _calc_Upgrade_Cost(i);
			if (prlv == 0){ // display unpurchased ones with white 
				doc_insert_text(doc, TERM_L_WHITE, info_ptr->name);
				doc_printf(doc, "<tab:%d>Unpurchased, Cost:%2d%\n", display.cx - 28, cost);
			}
			else if (prlv > 0 && prlv < 2){ // dgreen
				doc_insert_text(doc, TERM_GREEN, info_ptr->name);
				doc_printf(doc, "<tab:%d><color:g>Level: %2d%, Cost:%2d%</color>\n",
					display.cx - 28, cost);
			}
			else { // capped
				doc_insert_text(doc, TERM_YELLOW, info_ptr->name);
				doc_printf(doc, "<tab:%d><color:y>Max level</color>\n",
					display.cx - 28);
			}
			///*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
			choices++;
		}

	}

	doc_printf(doc, "\nProficiency points: %d\n", _prof_points);
	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);

	return choices;
}

static int _displaySkills(rect_t display, u16b mode)
{
	char    buf[MAX_NLEN];
	int     i;
	int choices = 0;
	point_t pos = rect_topleft(&display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	skill_desc_t    desc = { 0 }, descB = { 0 };
	skills_t        skills = _base_skills;
	skills_t		skills_b;

	padding = 5;   /* leading " a) " + trailing " " */
	padding += 12; /* " Count " */
	max_o_len = 12;
	if (max_o_len + padding > display.cx)
		max_o_len = display.cx - padding;

	_update_skills();
	skills_b = _skill_boost;
	/* Display */
	doc = doc_alloc(display.cx);
	doc_insert(doc, "<style:table>\n");
	
	doc_printf(doc, "<tab:2> %c) ", I2A(0));
	desc = skills_describe(skills.thn, 12); descB = skills_describe(skills_b.thn, 12);
	doc_printf(doc, " Melee      : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_THN]);

	doc_printf(doc, "<tab:2> %c) ", I2A(1));
	desc = skills_describe(skills.thb, 12); descB = skills_describe(skills_b.thb, 12);
	doc_printf(doc, " Ranged     : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_THB]);

	doc_printf(doc, "<tab:2> %c) ", I2A(2));
	desc = skills_describe(skills.sav, 7); descB = skills_describe(skills_b.sav, 7);
	doc_printf(doc, " SavingThrow: <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_SAV]);

	doc_printf(doc, "<tab:2> %c) ", I2A(3));
	desc = skills_describe(skills.stl, 1); descB = skills_describe(skills_b.stl, 1);
	doc_printf(doc, " Stealth    : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_STL]);

	doc_printf(doc, "<tab:2> %c) ", I2A(4));
	desc = skills_describe(skills.fos, 6); descB = skills_describe(skills_b.fos, 6);
	doc_printf(doc, " Perception : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_FOS]);

	doc_printf(doc, "<tab:2> %c) ", I2A(5));
	desc = skills_describe(skills.srh, 6); descB = skills_describe(skills_b.srh, 6);
	doc_printf(doc, " Searching  : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_SRH]);

	doc_printf(doc, "<tab:2> %c) ", I2A(6));
	desc = skills_describe(skills.dis, 8); descB = skills_describe(skills_b.dis, 8);
	doc_printf(doc, " Disarming  : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_DIS]);

	doc_printf(doc, "<tab:2> %c) ", I2A(7));
	desc = skills_describe(skills.dev, 6); descB = skills_describe(skills_b.dev, 6);
	doc_printf(doc, " Device     : <color:%c>%s</color> (+<color:%c>%s</color>),  lv: %d\n", attr_to_attr_char(desc.color), desc.desc, attr_to_attr_char(descB.color), descB.desc, _skillLevels[_SKILL_DEV]);

	doc_printf(doc, "\nProficiency points: %d\n", _prof_points);
	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);

	return _SKILL_MAX;
}



static void _displayProficiencies(rect_t display, u16b mode)
{
	char    buf[MAX_NLEN];
	int     i;
	point_t pos = rect_topleft(&display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	fl_proficiency *list = _proficiencies;
	int DC = 1;
	bool displayLvs = TRUE;

	padding = 5;   /* leading " a) " + trailing " " */
	padding += 12; /* " Count " */

	/* Measure */
	for (i = 0; i < _FL_MAX_PROF; i++)
	{
		fl_proficiency *prof_ptr = list + i;
		if (prof_ptr->prof != _FL_NULL)
		{
			int len;
			strcpy(buf, prof_ptr->name);
			len = strlen(buf);
			if (len > max_o_len)
				max_o_len = len;
		}
	}

	if (max_o_len + padding > display.cx)
		max_o_len = display.cx - padding;

	/* Display */
	doc = doc_alloc(display.cx);
	doc_insert(doc, "<style:table>");
	for (i = 0; i < _FL_MAX_PROF-1; i++)
	{
		fl_proficiency *prof_ptr = list + i;

		switch (i){ // categories neatly.
		case 0: doc_printf(doc, "\nGeneral: \n"); break;
		case _FL_IDX_BOOST_FIRST:	doc_printf(doc, "\nBoosts: \n"); break;
			case _FL_IDX_FLAG_FIRST:	doc_printf(doc, "\nTraits: \n"); break;
		}

		doc_printf(doc, "<tab:3> %c) ", I2A(i));

		if (prof_ptr->prof != _FL_NULL)
		{
			int prlv = fl_GetProfLevel(prof_ptr->prof, 0);
			int prmaxlv = prof_ptr->maxLevel;
			displayLvs = !(prof_ptr->prof == _FL_LEARN_REALM || prof_ptr->prof == _FL_LEARN_WEAPON || prof_ptr->prof == _FL_BOOST_SKILL);

			int cost = prof_ptr->cost;
			if (prlv > 1) cost = _calc_Upgrade_Cost(i);

				if (prof_ptr->minPLev <= p_ptr->lev){
					if (prlv == 0){ // display unpurchased ones with white 
						doc_insert_text(doc, TERM_L_WHITE, prof_ptr->name);
						if (displayLvs) doc_printf(doc, "<tab:%d>Unpurchased, Cost:%2d%\n",
							display.cx - 28, cost);
						else doc_insert_text(doc, TERM_L_DARK, "\n");
					}
					else if (prlv > 0 && prlv < prof_ptr->maxLevel){ // dgreen
						doc_insert_text(doc, TERM_L_GREEN, prof_ptr->name);
						if (displayLvs) doc_printf(doc, "<tab:%d><color:G>Level: %2d%, Cost:%2d%</color>\n",
							display.cx - 28, cost);
						else doc_insert_text(doc, TERM_L_DARK, "\n");
					}
					else { // capped
						doc_insert_text(doc, TERM_YELLOW, prof_ptr->name);
						if (displayLvs) doc_printf(doc, "<tab:%d><color:y>Max level</color>\n",
							display.cx - 28);
						else doc_insert_text(doc, TERM_L_DARK, "\n");
					}
				}
				else{
					doc_insert_text(doc, TERM_L_DARK, prof_ptr->name);
					doc_printf(doc, "<color:d>Minimum level requirement: %2d%</color>\n",
						display.cx - 28, prof_ptr->minPLev);
				}
			

			///*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
		}
	
	}

	doc_printf(doc, "\nProficiency points: %d\n", _prof_points);

	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);
}

void freelancer_count_buys(void){
	int i = 0;

	// 0 out.
	for (i = 0; i < _FL_MAX_PROF; i++) { _profLevels[i] = 0; }
	for (i = 0; i < MAX_REALM; i++) { _realmLevels[i] = 0; }
	for (i = 0; i < _FL_TVAL_WPNMAX; i++) { _wpnLevels[i] = 0; }
	for (i = 0; i < _SKILL_MAX; i++) { _skillLevels[i] = 0; }

		for (i = 0; i < _FL_MAX_BUYS; i++){
			if (_fl_purchases[i].lv >= p_ptr->lev) {
				switch( _fl_purchases[i].profID ){
					case _FL_LEARN_REALM: _realmLevels[_fl_purchases[i].subID]++; break;
					case _FL_LEARN_WEAPON: _wpnLevels[_fl_purchases[i].subID]++; break;
					case _FL_BOOST_SKILL:_skillLevels[_fl_purchases[i].subID]++; break;
					default: _profLevels[_fl_purchases[i].profID]++; break;
				}	
			} else if (_fl_purchases[i].profID == _FL_NULL) break; // the way they are done, there shouldn't be nulls inbetween...
		}

}

int freelancer_spell_power(int use_realm){

	if (use_realm < 0 || use_realm >= MAX_REALM) return 0;
	if (_realmLevels[use_realm] == 0) return 0;

			return (p_ptr->lev * 10) / (15 - _realmLevels[use_realm]*2);
}

cptr _grantProficiency(int prof, int sub_choice, int *res){
	int getCost = -1;
	int curLv = 0, minPlev = 0;
	bool maxedOut = FALSE;
	bool ok = FALSE;
	cptr profName;

	res = -1;
	// Basically check if you are either:
	// 1. Have enough points for buying
	// 2. Are high level enough.
	// 3. Are already maxed out.
	// Also, a method of tracking purchases is needed so develeveling works properly. 
	bool upgrading = FALSE; // this is if we are not buying

	if (prof == _FL_LEARN_REALM && (sub_choice >= 0 && sub_choice < MAX_REALM)){
		getCost = _realmInfo[sub_choice].cost;
		profName = realm_names[_realmInfo[sub_choice].realmID];
			if (_realmLevels[sub_choice] >= _proficiencies[prof].maxLevel) maxedOut = TRUE;
			ok = TRUE;
	}
	else if (prof == _FL_LEARN_WEAPON && (sub_choice >= 0 && sub_choice < _FL_TVAL_WPNMAX)){
		getCost = _wpn_profs[sub_choice].cost;
		profName = _wpn_profs[sub_choice].name;
			if (_wpnLevels[sub_choice] >= _proficiencies[prof].maxLevel) maxedOut = TRUE;
			ok = TRUE;
	}
	else if (prof == _FL_BOOST_SKILL && (sub_choice >= 0 && sub_choice < _SKILL_MAX)){
		getCost = _skillCosts[sub_choice];
		profName = _skillNames[sub_choice];
			if (_skillLevels[sub_choice] >= _proficiencies[prof].maxLevel) maxedOut = TRUE;
			ok = TRUE;
	}
	else {
			getCost = _proficiencies[prof].cost;
			profName = _proficiencies[prof].name;
			if (_profLevels[prof] >= _proficiencies[prof].maxLevel) maxedOut = TRUE;
			ok = TRUE;
	}
	// check if it is legit choice.
	if (getCost > _prof_points) return "You don't have enough proficiency points.";
	else if (maxedOut) return "The proficiency is already at max level.";
	else if (_p_ct >= _FL_MAX_BUYS - 2) return "You cannot buy more proficiencies. Perhaps buy a potion of new life?";

	if(ok){
		_fl_purchases[_p_ct].lv = p_ptr->lev; // mark down level gained
		_fl_purchases[_p_ct].profID = prof;  // mark profession
		_fl_purchases[_p_ct].subID = sub_choice; // mark possible sub-choice
		_p_ct++;
		res = 1;
	}
	return profName;
}


int _chooseProf(int options)
{
	int			 chosen_prof = _FL_NULL;
	int			 sub_choice = -1;
	int			 max_choises = 0;
	int          cmd;
	rect_t       display = ui_menu_rect();
	string_ptr   prompt = NULL;
	cptr		 failMsg;
	bool         done = FALSE, all_done = FALSE;
	bool         exchange = FALSE;

	if (display.cx > 80)
		display.cx = 80;

	prompt = string_alloc();
	screen_save();
	// oh christ this thing makes my heads spin...
	while (!all_done){ // main loop begin

		freelancer_count_buys(); // update things. 
		chosen_prof = _FL_NULL; sub_choice = -1;
		// first block, getting the proficiency
		while (1)
		{
			string_clear(prompt);
			string_printf(prompt, "Learn which proficiency? :");
			prt(string_buffer(prompt), 0, 0);
			_displayProficiencies(display, 0);
			cmd = inkey_special(FALSE);
			if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q'){ all_done = TRUE; break; }
			if ('a' <= cmd && cmd < 'a' + _FL_MAX_PROF)
			{
				chosen_prof = A2I(cmd);
				if (chosen_prof != _FL_NULL) break;
			}
		} 

		screen_load();
		screen_save();

		//sub-loops. If it is weapon-skill or realm, enter these stupid things. Otherwise, you are free to go.
		if (chosen_prof == _FL_LEARN_REALM){
			while (1){
				string_clear(prompt);
				string_printf(prompt, "Learn which realm? :");
				prt(string_buffer(prompt), 0, 0);
				max_choises = _displayRealms(display, 0);
				cmd = inkey_special(FALSE);
				if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q'){ chosen_prof = _FL_NULL; break; }
				if ('a' <= cmd && cmd < 'a' + max_choises)
				{
					sub_choice = A2I(cmd);
					if (sub_choice != -1) break; 
				}
			}
		}
		else if (chosen_prof == _FL_LEARN_WEAPON){
			while (1){

				string_clear(prompt);
				string_printf(prompt, "Learn which weapon group? :");
				prt(string_buffer(prompt), 0, 0);
				max_choises = _displayWpnGroups(display, 0);
				cmd = inkey_special(FALSE);
				if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q'){ chosen_prof = _FL_NULL; break; }
				if ('a' <= cmd && cmd < 'a' + max_choises)
				{
					sub_choice = A2I(cmd);
					if (sub_choice != -1) break; 
				}
			} 
		}
		else if (chosen_prof == _FL_BOOST_SKILL){
			while (1){
				string_clear(prompt);
				string_printf(prompt, "Boost which skill? :");
				prt(string_buffer(prompt), 0, 0);
				max_choises = _displaySkills(display, 0);
				cmd = inkey_special(FALSE);
				if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q'){ chosen_prof = _FL_NULL; break; }
				if ('a' <= cmd && cmd < 'a' + max_choises)
				{
					sub_choice = A2I(cmd);
					if (sub_choice != -1) break;
				}
			}
		}
		screen_load();
		screen_save();
		// Now we just check if we can actually -purchase- the said ability.
		// It will generate a message if so, a cptr.
		if (chosen_prof != _FL_NULL){
			int res = -1;
			failMsg = _grantProficiency(chosen_prof, sub_choice, &res);
			
			while (1){
					string_clear(prompt);
					string_printf(prompt, failMsg);
					prt(string_buffer(prompt), 0, 0);
					cmd = inkey_special(FALSE);
					if (cmd)  break; 
			}
			if (res == 1) all_done = TRUE;
			else{chosen_prof = _FL_NULL; sub_choice = -1;}
		}
		else all_done = TRUE; // just bugger off...

	} // end of main loop


	screen_load();
	string_free(prompt);

	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
	update_stuff();
	return chosen_prof;
}

static void _choose_prof_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Learn Proficiency");
		break;
	case SPELL_DESC:
		var_set_string(res, "Learn new skill yo.");
		break;
	case SPELL_CAST:
		_chooseProf(0);
		var_set_bool(res, TRUE);
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

static void _birth(void){
	/* Init all. */
	int i = 0;
	_prof_points = 1; // beanie points;

	// initialize, reset, all that jazz.
	for (i = 0; i < _FL_MAX_BUYS; i++){
		_fl_purchases[i].lv = -1;
		_fl_purchases[i].profID = _FL_NULL;
		_fl_purchases[i].subID = -1;
	}
	_p_ct = 0; // SUPER-IMPORTANT

	// 0 out levels
	for (i = 0; i < _FL_MAX_PROF; i++) { _profLevels[i] = 0; }
	for (i = 0; i < MAX_REALM; i++) { _realmLevels[i] = 0; }
	for (i = 0; i < _FL_TVAL_WPNMAX; i++) { _wpnLevels[i] = 0; }

	// and skill boosts as well.
	skills_init(&_skill_boost);
	for (i = 0; i < _SKILL_MAX; i++) { _skillLevels[i] = 0; }
}

static int _get_powers(spell_info* spells, int max)
{
	int ct = 0;
	
	spell_info* spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _choose_prof_spell;
	
	return ct;
}

int _castingStat(void){
	int res = A_INT; // return highest
		//	if (p_ptr->stat_use[res] < p_ptr->stat_use[A_CHR]) res = A_CHR;
		//	if (p_ptr->stat_use[res] < p_ptr->stat_use[A_WIS]) res = A_WIS;
	return res;
}


	


int _minFail(void){
	return 5;
}

int _castWeightLimit(void){
	int lv = fl_GetProfLevel(_FL_F_HEAVY_ARMOR_CASTING,0);
	if (lv > 0) return 700 + lv * 100;
		return 420;
}

u32b _casterFlags(void){

	u32b castFlags = 0;
	return castFlags;
}

int freelancer_sp_boost(void){
	if (fl_GetProfLevel(_FL_BOOST_SP, 0) > 0){
		return fl_GetProfLevel(_FL_BOOST_SP, 0)*p_ptr->lev; // 1 per level.
	}
	return 0;
}

int freelancer_hp_boost(void){
	if (fl_GetProfLevel(_FL_BOOST_HP, 0) > 0){
		return fl_GetProfLevel(_FL_BOOST_HP, 0)*p_ptr->lev*2; // 1 per level.
	}
	return 0;
}

int freelancer_stab_level(){
	int stabbylv = fl_GetProfLevel(_FL_F_SNEAK_ATTACKS, 0);
	return stabbylv;
}

bool freelancer_can_use_realm(int realm){

	if (_realmLevels[realm] > 0) return TRUE;
	return FALSE;
}
 
static void _calc_bonuses(void)
{
	freelancer_count_buys(); // COUNT THEM!
	freelancer_skill_boost();

	if (fl_GetProfLevel(_FL_F_AUTO_IDENTIFY, 0) > 0) p_ptr->auto_id = TRUE;

}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{

}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
	int meleeBoost = fl_GetProfLevel(_FL_BOOST_MELEE, 0);
	int blowBoost = fl_GetProfLevel(_FL_BOOST_BLOWS, 0);
	if (meleeBoost > 0){
		info_ptr->to_d += p_ptr->lev / (10 - meleeBoost);
		info_ptr->dis_to_d += p_ptr->lev / (10 - meleeBoost);
	}

	if(blowBoost>0) info_ptr->xtra_blow += py_prorata_level_aux(( p_ptr->lev * blowBoost)/4, 0, 1, 1);

}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
	int rangedBoost = fl_GetProfLevel(_FL_BOOST_RANGED, 0);
	// we don't really care what we shoot with, unlike rangers. We do worse, though.
	if (rangedBoost > 0){
		// not sure how to handle this yet, actually.
		//info_ptr->num_fire += (p_ptr->lev * 50 + rangedBoost * 10) / 50;
	}
}

static caster_info* _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "knapsack";
		me.which_stat = _castingStat;
		me.weight = _castWeightLimit;
		me.min_fail = _minFail;
		me.options = _casterFlags;
		init = TRUE;
	}
	return &me;
}

class_t *freelancer_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	if (!init)
	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 15, 18, 28, 1, 12, 2, 48, 20 };
		skills_t xs = { 5, 7, 9, 0, 0, 0, 13, 11 };

		me.name = "Freelancer";
		me.desc = "Freelancers are wanderers who pick up skills as they "
			"gain experience. Their flexibility is their greatest strength: "
			"while weak at beginning, they gain proficiency points that "
			"they can use to purchase new abilities: skills, realms, etc. "
			"However, due to not being as focused as others, these abilities "
			"are somewhat weaker.\n"
			"Their highest mental ability determines their casting ability.";

		me.stats[A_STR] = 0;
		me.stats[A_INT] = 0;
		me.stats[A_WIS] = 0;
		me.stats[A_DEX] = 0;
		me.stats[A_CON] = 0;
		me.stats[A_CHR] = 0;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 100;
		me.base_hp = 8;
		me.exp = 130;
		me.pets = 40;

		me.birth = _birth;
		me.caster_info = _caster_info;
		me.calc_bonuses = _calc_bonuses;
		me.get_flags = _get_flags;
		me.calc_weapon_bonuses = _calc_weapon_bonuses;
		me.calc_shooter_bonuses = _calc_shooter_bonuses;
		/* TODO: This class uses spell books, so we are SOL
		me.get_spells = _get_spells;*/
		me.get_powers = _get_powers;
		me.character_dump = spellbook_character_dump;
		init = TRUE;
	}

	return &me;
} 
