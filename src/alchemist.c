#include "angband.h"

#include <assert.h>

/** Based on Magic Eater code, if it wasn't obvious.**/

#define _MAX_INF_SLOTS 8
#define _INVALID_SLOT -1
#define _INFUSION_CAP 30
#define _MAX_CHEM 35000

static object_type _infusions[_MAX_INF_SLOTS];

static int _CHEM = 0;

typedef struct {
	int  sval;
	cptr name;
	int  cost;
	int minLv;
} _formula_info_t, *_formula_info_ptr;


// a single potion is about 1/3 of the cost ( similar to alchemy spell )
// empties for sake of being consistent with the ids, so I can just do quick lookup with _formulas[SV_POTION_WATER] etc. 
// svals might be bit excessive then, but eh... Easier on eye, perhaps. Could be checked for safety, ex (if(i!=sval) search_through ). 

static _formula_info_t _null_formula = { -1, "", -1, FALSE };

static _formula_info_t _formulas[POTION_MAX] = {
{ SV_POTION_WATER,				"water",					10, 1},
{ SV_POTION_APPLE_JUICE,		"apple juice",				10, 1},
{ SV_POTION_SLIME_MOLD,			"slime mold",				10, 1}, 
{ -1, "", -1, 999 }, // ==========================================//
{ SV_POTION_SLOWNESS,			"slowness",					20, 1},
{ SV_POTION_SALT_WATER,			"salt water",				20, 1},
{ SV_POTION_POISON,				"poison",					30, 1},
{ SV_POTION_BLINDNESS,			"blindness",				30, 1},
{ -1, "", -1, 999 }, // ==========================================//
{ SV_POTION_CONFUSION,			"confusion",				30, 1},
{ -1, "", -1, 999 }, // ==========================================//
{ SV_POTION_SLEEP,				"sleep",					60, 1},
{ -1, "", -1, 999 }, // ==========================================//
{ SV_POTION_LOSE_MEMORIES,		"lose memories",			10, 1},
{ -1, "", -1, 999 }, // ==========================================//
{ SV_POTION_RUINATION,			"ruination",				200, 40},
{ SV_POTION_DEC_STR,			"decrease STR",				40, 1},
{ SV_POTION_DEC_INT,			"decrease INT",				40, 1},
{ SV_POTION_DEC_WIS,			"decrease WIS",				40, 1},
{ SV_POTION_DEC_DEX,			"decrease DEX",				40, 1},
{ SV_POTION_DEC_CON,			"decrease CON",				40, 1},
{ SV_POTION_DEC_CHR,			"decrease CHR",				40, 1},
{ SV_POTION_DETONATIONS,		"detonations",				300, 40},
{ SV_POTION_DEATH,				"death",					300, 40},
{ SV_POTION_INFRAVISION,		"infravision",				60, 5},
{ SV_POTION_DETECT_INVIS,		"detect invisibility",		60, 5},
{ SV_POTION_SLOW_POISON,		"slow poison",				20, 1},
{ SV_POTION_CURE_POISON,		"cure poison",				40, 1},
{ SV_POTION_BOLDNESS,			"boldness",					60, 5},
{ SV_POTION_SPEED,				"speed",					120, 10},
{ SV_POTION_RESIST_HEAT,		"resist heat",				60, 5},
{ SV_POTION_RESIST_COLD,		"resist cold",				60, 5},
{ SV_POTION_HEROISM,			"heroism",					90, 5},
{ SV_POTION_BERSERK_STRENGTH,	"berserk strength",			120, 25},
{ SV_POTION_CURE_LIGHT,			"cure light wounds",		10, 1},
{ SV_POTION_CURE_SERIOUS,		"cure serious wounds",		30, 5},
{ SV_POTION_CURE_CRITICAL,		"cure critical wounds",		60, 15},
{ SV_POTION_HEALING,			"healing",					200, 30},
{ SV_POTION_STAR_HEALING,		"*healing",					400, 40},
{ SV_POTION_LIFE,				"life",						500, 60},
{ SV_POTION_RESTORE_MANA,		"restore mana",				180, 30},
{ SV_POTION_RESTORE_EXP,		"restore EXP",				60, 15},
{ SV_POTION_RES_STR,			"restore STR",				60, 15},
{ SV_POTION_RES_INT,			"restore INT",				60, 15},
{ SV_POTION_RES_WIS,			"restore WIS",				60, 15},
{ SV_POTION_RES_DEX,			"restore DEX",				60, 15},
{ SV_POTION_RES_CON,			"restore CON",				60, 15},
{ SV_POTION_RES_CHR,			"restore CHR",				60, 15},
{ SV_POTION_INC_STR,			"increase STR",				900, 999},
{ SV_POTION_INC_INT,			"increase INT",				900, 999},
{ SV_POTION_INC_WIS,			"increase WIS",				900, 999},
{ SV_POTION_INC_DEX,			"increase DEX",				900, 999},
{ SV_POTION_INC_CON,			"increase CON",				900, 999},
{ SV_POTION_INC_CHR,			"increase CHR",				900, 999},
{ -1, "", -1, 999 }, // ==========================================//
{ SV_POTION_AUGMENTATION,		"augmentation",				2400, 999},
{ SV_POTION_ENLIGHTENMENT,		"enlightenment",			120, 30},
{ SV_POTION_STAR_ENLIGHTENMENT,	"*enlightenment",			180, 40},
{ SV_POTION_SELF_KNOWLEDGE,		"self knowledge",			120, 30},
{ SV_POTION_EXPERIENCE,			"experience",				3000, 999},
{ SV_POTION_RESISTANCE,			"resistance",				120, 15},
{ SV_POTION_CURING,				"curing",					90, 20},
{ SV_POTION_INVULNERABILITY,	"invulnerability",			300, 60},
{ SV_POTION_NEW_LIFE,			"new life",					240, 1},
{ SV_POTION_NEO_TSUYOSHI,		"neo-tsuyoshi",				60, 1},
{ SV_POTION_TSUYOSHI,			"tsuyoshi",					30, 1},
{ SV_POTION_POLYMORPH,			"polymorph",				120, 1},
{ SV_POTION_BLOOD,				"blood",					120, 1},
{ SV_POTION_GIANT_STRENGTH,		"giant's strength",			120, 30},
{ SV_POTION_STONE_SKIN,			"stone skin",				120, 30},
{ SV_POTION_CLARITY,			"clarity",					90, 20},
{ SV_POTION_GREAT_CLARITY,		"great clarity",			120, 30},

};


static void _birth(void)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		memset(&_infusions[i], 0, sizeof(object_type));
	}
	_CHEM = 0;
}

static object_type *_which_obj(int tval, int slot)
{
	assert(0 <= slot && slot < _MAX_INF_SLOTS);
	return _infusions + slot;
}

static _formula_info_t _FindFormula(int sval){

	if (sval == _formulas[sval].sval) return _formulas[sval];

	for (int i = 0; i < POTION_MAX; i++){
		if (sval == _formulas[i].sval) return _formulas[i];
	}
	return _null_formula;
}

static void _displayInfusions(rect_t display)
{
	char    buf[MAX_NLEN];
	int     i;
	point_t pos = rect_topleft(&display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	object_type *list = _infusions;
	int alcskil = _AlchemistSkill();
	int DC = 1;

	padding = 5;   /* leading " a) " + trailing " " */
	padding += 12; /* " Count " */

	/* Measure */
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = list + i;
		if (o_ptr->k_idx)
		{
			int len;
			object_desc(buf, o_ptr, 0);
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
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = list + i;

		doc_printf(doc, " %c) ", I2A(i));

		if (o_ptr->k_idx)
		{
			DC = _FindFormula(o_ptr->sval).minLv;
			

			
			object_desc(buf, o_ptr, OD_COLOR_CODED);
			doc_insert(doc, buf);
			if (DC <= alcskil){ // can do
				doc_printf(doc, "<tab:%d>Cost: %4d%, DC:<color:G>%3d%</color>\n",
					display.cx - 22, _FindFormula(o_ptr->sval).cost, DC);
			} 
			else if (DC == 999){ // can't do ever
				doc_printf(doc, "<tab:%d><color:r>Unreproduceable</color>\n",display.cx - 22);
			}
			else {
				doc_printf(doc, "<tab:%d>Cost: %4d%, DC:<color:R>%3d%</color>\n",
					display.cx - 22, _FindFormula(o_ptr->sval).cost, DC);
			}

			/*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
		}
		else
			doc_insert_text(doc, TERM_L_DARK, "(None)\n");
	}
	doc_printf(doc, "Chemical stock: %d \n", _CHEM);

	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);
}

#define _ALLOW_EMPTY    0x01 /* Absorb */
#define _ALLOW_SWITCH   0x02 /* Browse/Use */
#define _ALLOW_EXCHANGE 0x04


int _AlchemistSkill(){
	return (p_ptr->lev + adj_mag_stat[p_ptr->stat_ind[A_INT]]);
}

object_type *_chooseInfusion(cptr verb, int tval, int options)
{
	object_type *result = NULL;
	int          slot = 0;
	int          cmd;
	rect_t       display = ui_menu_rect();
	int          which_tval = tval;
	string_ptr   prompt = NULL;
	bool         done = FALSE;
	bool         exchange = FALSE;
	int          slot1 = _INVALID_SLOT, slot2 = _INVALID_SLOT;

	if (display.cx > 80)
		display.cx = 80;

	which_tval = TV_POTION;
	prompt = string_alloc();
	screen_save();

	while (!done)
	{
		string_clear(prompt);

		if (exchange)
		{
			if (slot1 == _INVALID_SLOT)
				string_printf(prompt, "Select the first %s:", "infusion");
			else
				string_printf(prompt, "Select the second %s:", "infusion");
		}
		else
		{
			string_printf(prompt, "%s which %s", verb, "infusion");
			if (options & _ALLOW_SWITCH)
			{
				if (options & _ALLOW_EXCHANGE){
					string_append_s(prompt, ", ['X' to Exchange");
					string_append_s(prompt, "]:");
				}
			}
			else
				string_append_c(prompt, ':');
		}
		prt(string_buffer(prompt), 0, 0);
		_displayInfusions(display);

		cmd = inkey_special(FALSE);

		if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q')
			done = TRUE;

		if (options & _ALLOW_EXCHANGE)
		{
			if (!exchange && (cmd == 'x' || cmd == 'X'))
			{
				exchange = TRUE;
				slot1 = slot2 = _INVALID_SLOT;
			}
		}

		if ('a' <= cmd && cmd < 'a' + _MAX_INF_SLOTS)
		{
			slot = A2I(cmd);
			if (exchange)
			{
				if (slot1 == _INVALID_SLOT)
					slot1 = slot;
				else
				{
					slot2 = slot;
					if (slot1 != slot2)
					{
						object_type  tmp = *_which_obj(which_tval, slot1);
						object_type *obj1 = _which_obj(which_tval, slot1);
						object_type *obj2 = _which_obj(which_tval, slot2);

						*obj1 = *obj2;
						*obj2 = tmp;
					}
					exchange = FALSE;
					slot1 = slot2 = _INVALID_SLOT;
				}
			}
			else
			{
				object_type *o_ptr = _which_obj(which_tval, slot);
				if (o_ptr->k_idx || (options & _ALLOW_EMPTY))
				{
					result = o_ptr;
					done = TRUE;
				}
			}
		}
	}

	screen_load();
	string_free(prompt);
	return result;
}

int _alchemist_infusion_energyreduction(){

	return 0;
}

void _use_infusion(object_type* o_ptr, int overdose)
{
	int  boost = device_power(100) - 100;
	u32b flgs[OF_ARRAY_SIZE];
	cptr used = NULL;
	int  uses = 1;

	energy_use = 100 - _alchemist_infusion_energyreduction();

	obj_flags(o_ptr, flgs);
	
	if (o_ptr->number < uses || o_ptr->number == 0)
	{
		if (flush_failure) flush();
		msg_print("There are no enough infusions.");
		return;
	}
	
	/*
	if (o_ptr->activation.type == EFFECT_IDENTIFY)
		device_available_charges = device_sp(o_ptr) / o_ptr->activation.cost;
		**/
	sound(SOUND_QUAFF);
	
	used = do_device(o_ptr, SPELL_CAST, boost);
	if (used)
	{
		msg_print(used); // get the message of effect.

		o_ptr->number -= uses;
		stats_on_use(o_ptr, uses);

	}
	else
		energy_use = 0;
}

void alchemist_browse(void)
{
	object_type *o_ptr = _chooseInfusion("Browse", TV_POTION, _ALLOW_SWITCH | _ALLOW_EXCHANGE);
	if (o_ptr)
		obj_display(o_ptr);
}

void alchemist_cast(int tval)
{
	object_type *o_ptr;

	if (!tval)
		tval = TV_POTION;

	o_ptr = _chooseInfusion("Use", tval, _ALLOW_SWITCH);
	if (o_ptr)
	{
		_use_infusion(o_ptr, 1);
	}

}

/* Absorb Magic */
static bool create_infusion(void)
{
	int item;
	object_type *src_ptr;
	object_type *dest_ptr;
	char o_name[MAX_NLEN];

	item_tester_hook = object_is_potion;
	if (!get_item(&item, "Create infusion from which potions? ", "You have nothing to create infusions from.", (USE_INVEN | USE_FLOOR)))
		return FALSE;

	if (item >= 0)
		src_ptr = &inventory[item];
	else
		src_ptr = &o_list[0 - item];

	dest_ptr = _chooseInfusion("Replace", src_ptr->tval, _ALLOW_EMPTY);
	if (!dest_ptr)
		return FALSE;

	if (dest_ptr->number >= _INFUSION_CAP) {
		object_desc(o_name, dest_ptr, OD_COLOR_CODED);
		msg_format("This slot is already full of %s", o_name);
		return FALSE;
	}

	if (dest_ptr->k_idx && dest_ptr->sval != src_ptr->sval)
	{
		char prompt[255];
		object_desc(o_name, dest_ptr, OD_COLOR_CODED);
		sprintf(prompt, "Really replace %s? <color:y>[y/N]</color>", o_name);
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return FALSE;
	}

	int infct = get_quantity(NULL, MIN(src_ptr->number,_INFUSION_CAP));

	if (dest_ptr->sval == src_ptr->sval && dest_ptr->number>0){ 
		// we already got one, so just increment them! And check that there is -something-, because water has sval of 0
		bool capped = FALSE;

		if (dest_ptr->number + infct > _INFUSION_CAP){ infct = _INFUSION_CAP - dest_ptr->number; capped = TRUE; }
		if (infct < 0) infct = 0;
	
			dest_ptr->number += infct;

		if (capped == FALSE){
			msg_format("You create %d additional infusions.", infct);
		}
		else {
			msg_format("You create %d additional infusions, reaching the limit.", infct);
		}
	}
	// limit the infusions
	else {
		object_desc(o_name, src_ptr, OD_COLOR_CODED);
		msg_format("You create infusions out of %s.", o_name);

		*dest_ptr = *src_ptr;
		dest_ptr->discount = 0;
		dest_ptr->number = infct;
		dest_ptr->inscription = 0;
		obj_identify_fully(dest_ptr);
		stats_on_identify(dest_ptr);
	}
	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -infct);
		inven_item_describe(item);
		inven_item_optimize(item);
	}
	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -infct);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
	return TRUE;
}

static void _create_infusion_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Create Infusion");
		break;
	case SPELL_DESC:
		var_set_string(res, "Alchemically process potion to create an infusion - faster to use, and resistant to damage.");
		break;
	case SPELL_CAST:
		var_set_bool(res, create_infusion());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

static bool evaporate(){}

static void _evaporate_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Evaporate");
		break;
	case SPELL_DESC:
		var_set_string(res, "Evaporates an potion, creating a burst that may harm or benefit creatures.");
		break;
	case SPELL_CAST:
		var_set_bool(res, evaporate());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

static bool break_down_potion(void){
	int item;
	object_type *o_ptr;
	char o_name[MAX_NLEN];

	item_tester_hook = object_is_potion;
	if (!get_item(&item, "Break down which potions? ", "You have nothing to break down.", (USE_INVEN | USE_FLOOR)))
		return FALSE;

	if (item >= 0) o_ptr = &inventory[item];
	else o_ptr = &o_list[0 - item];

	int ct = get_quantity(NULL, o_ptr->number);

	if (ct > 0){
			int cost = (ct * _FindFormula(o_ptr->sval).cost)/2;

			if (_CHEM + cost > _MAX_CHEM){ 
				char prompt[255];
				object_desc(o_name, o_ptr, OD_COLOR_CODED);
				sprintf(prompt, "Really break down %s? You will exceed the chemical, and some will be lost. <color:y>[y/N]</color>", o_name);
				if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
					return FALSE;
			}

			_CHEM += cost;
			if (_CHEM > _MAX_CHEM) _CHEM = _MAX_CHEM;

			msg_format("You break down potion(s) and gain %d chemical, totaling at %d.", cost, _CHEM);
	}
	
	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -ct);
		inven_item_describe(item);
		inven_item_optimize(item);
	}
	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -ct);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
	return TRUE;
}

static void _break_down_potion_spell(int cmd, variant *res){
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Break Down Potion");
		break;
	case SPELL_DESC:
		var_set_string(res, "Breaks down a potion to its base chemicals.");
		break;
	case SPELL_CAST:
		var_set_bool(res, break_down_potion());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}

void _reproduceInf(object_type* o_ptr){
	
	u32b flgs[OF_ARRAY_SIZE];
	int limit = _INFUSION_CAP;
	char o_name[MAX_NLEN];

	energy_use = 100;

	obj_flags(o_ptr, flgs);

	if (o_ptr->number >= _INFUSION_CAP)
	{
		if (flush_failure) flush();
		msg_print("This slot is already full."); // perhaps make it sound more flavourful...
		return;
	}
	else if (!o_ptr){
		if (flush_failure) flush();
		msg_print("There's nothing to reproduce."); 
		return;
	}

	sound(SOUND_QUAFF); // what the hell would be the sound anyway?


	limit = _INFUSION_CAP - o_ptr->number;
	if (limit < 0) limit = 0;

	int infct = get_quantity(NULL, MIN(_INFUSION_CAP, limit));

	int cost = infct * _FindFormula(o_ptr->sval).cost;

	if (_FindFormula(o_ptr->sval).minLv > _AlchemistSkill()){
		msg_format("This infusion is beyond your skills to reproduce.");
		return FALSE;
	}
	
	if (o_ptr->level){
		msg_format("This infusion is beyond your skills to reproduce.");
		return FALSE;
	}

	char prompt[255];
	object_desc(o_name, o_ptr, OD_COLOR_CODED);
	sprintf(prompt, "Reproduction would cost %d chemicals. Are you sure?", cost);
	if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
		return FALSE;

	if (cost > _CHEM){ 
		msg_format("You do not have enough chemicals.");
		return FALSE;
	}

	if (infct > 0)
	{
		msg_format("You recreate %d infusions.", infct);
		o_ptr->number += infct;
	}
	else
		energy_use = 0;

}

static bool reproduceInfusion(){
	object_type *o_ptr;
	o_ptr = _chooseInfusion("Reproduce", TV_POTION, 0);
	if (o_ptr){ _reproduceInf(o_ptr, 1); }
}

static void _reproduce_infusion_spell(int cmd, variant *res)
{
	switch (cmd)
	{
	case SPELL_NAME:
		var_set_string(res, "Reproduce Infusion");
		break;
	case SPELL_DESC:
		var_set_string(res, "Uses chemicals to create infusion based on existing infusion.");
		break;
	case SPELL_CAST:
		var_set_bool(res, reproduceInfusion());
		break;
	default:
		default_spell(cmd, res);
		break;
	}
}


void alchemist_gain(void)
{
	if (cast_spell(_create_infusion_spell))
		energy_use = 100;
}

/* Regeneration */
int alchemist_regen_amt(int tval)
{
	return 1;
}

bool alchemist_regen(int pct)
{
	if (p_ptr->pclass != CLASS_ALCHEMIST) return FALSE;
}
/* Auto-ID */
bool alchemist_auto_id(object_type *o_ptr)
{
	/*
	int i;
	if (p_ptr->pclass != CLASS_MAGIC_EATER) return FALSE;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *device_ptr = _which_obj(TV_POTION, i);
		if (device_ptr->activation.type == EFFECT_IDENTIFY && device_sp(device_ptr) > device_ptr->activation.cost)
		{
			identify_item(o_ptr);
			stats_on_use(device_ptr, 1);
			device_decrease_sp(device_ptr, device_ptr->activation.cost);
			return TRUE;
		}
	}
	return FALSE; */
}

/* Character Dump */
static void _dump_list(doc_ptr doc)
{
	int i;
	char o_name[MAX_NLEN];
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = _infusions + i;
		if (o_ptr->k_idx)
		{
			object_desc(o_name, o_ptr, OD_COLOR_CODED);
			doc_printf(doc, "%c) %s\n", I2A(i), o_name);
		}
		else
			doc_printf(doc, "%c) (Empty)\n", I2A(i));
	}
	doc_newline(doc);
}

static void _character_dump(doc_ptr doc)
{
	doc_printf(doc, "<topic:Alchemist>============================= Created <color:keypress>I</color>nfusions ============================\n\n");

	_dump_list(doc);

	doc_newline(doc);
}

static void _load_list(savefile_ptr file)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = _infusions + i;
		memset(o_ptr, 0, sizeof(object_type));
	}

	while (1)
	{
		object_type *o_ptr;
		i = savefile_read_u16b(file);
		if (i == 0xFFFF) break;
		assert(0 <= i && i < _MAX_INF_SLOTS);
		o_ptr = _infusions + i;
		rd_item(file, o_ptr);
		assert(o_ptr->k_idx);
	}

	_CHEM = savefile_read_s32b(file);
}

static void _load_player(savefile_ptr file)
{
	_load_list(file);
}

static void _save_list(savefile_ptr file)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		object_type *o_ptr = _infusions + i;
		if (o_ptr->k_idx)
		{
			savefile_write_u16b(file, (u16b)i);
			wr_item(file, o_ptr);
		}
	}
	savefile_write_u16b(file, 0xFFFF); /* sentinel */

	savefile_write_s32b(file, (s32b)_CHEM); // save chemical count.
}

static void _save_player(savefile_ptr file)
{
	_save_list(file);
}


/* Class Info */
static int _get_powers(spell_info* spells, int max)
{
	int ct = 0;

	spell_info* spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _create_infusion_spell;

	spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _reproduce_infusion_spell;

	spell = &spells[ct++];
	spell->level = 1;
	spell->cost = 0;
	spell->fail = 0;
	spell->fn = _break_down_potion_spell;

				spell = &spells[ct++];
	spell->level = 8;
	spell->cost = 4;
	spell->fail = 20;
	spell->fn = _evaporate_spell;

				spell = &spells[ct++];
	spell->level = 25;
	spell->cost = 5;
	spell->fail = 40;
	spell->fn = alchemy_spell;

	return ct;
}

class_t *alchemist_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	/* static info never changes */
	if (!init)
	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 20, 20, 24, 20, 16, 10, 70, 66 };
		skills_t xs = { 7, 7, 10, 10, 10, 0, 25, 18 };

		me.name = "Alchemist";
		me.desc = "Alchemist is master of tinctures, conconctions and "
			"infusions. They can prepare themselves set infusions from  "
			"that replicate the effect - without consuming space in "
			"inventory or having chance of shattering. They are rather "
			"decent in melee, especially with the right potion - they "
			"cannot rival those specialized in such, though. "
			"Even bad potions are useful in their hands, as weapons or "
			"ingredients. Their other abilities include creating copies "
			"of potions and turning items to gold. They require intelligence "
			"for some of their abilities.";

		me.stats[A_STR] = -1;
		me.stats[A_INT] = 2;
		me.stats[A_WIS] = -1;
		me.stats[A_DEX] = 2;
		me.stats[A_CON] = -2;
		me.stats[A_CHR] = -2;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 102;
		me.base_hp = 12;
		me.exp = 150;
		me.pets = 30;

		me.birth = _birth;
		me.get_powers = _get_powers;
		me.character_dump = _character_dump;
		me.load_player = _load_player;
		me.save_player = _save_player;
		init = TRUE;
	}

	return &me;
}
