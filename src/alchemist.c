#include "angband.h"

#include <assert.h>

/** Based on Magic Eater code, if it wasn't obvious.**/

#define _MAX_INF_SLOTS 10
#define _INVALID_SLOT -1
#define _INFUSION_CAP 99
#define _MAX_CHEM 35000

#define _CTIER0 0
#define _CTIER1 1
#define _CTIER2 2
#define _CTIER_MAX 3

static cptr _tiername[_CTIER_MAX] = {
	"tier 0",
	"tier 1",
	"tier 2"
};

static object_type _infusions[_MAX_INF_SLOTS];

static int _CHEM[] = {0,0,0};


int _AlchemistSkill(void){
	return (p_ptr->lev + adj_mag_stat[p_ptr->stat_ind[A_INT]]);
}


typedef struct {
	int  sval;
	int  cost;
	int minLv;
	int ctier;
} _formula_info_t, *_formula_info_ptr;


// a single potion is about 1/3 of the cost ( similar to alchemy spell )
// empties for sake of being consistent with the ids, so I can just do quick lookup with _formulas[SV_POTION_WATER] etc. 
// svals might be bit excessive then, but eh... Easier on eye, perhaps. Could be checked for safety, ex (if(i!=sval) search_through ). 

static _formula_info_t _formulas[POTION_MAX+1] = {
{ SV_POTION_WATER,					10, 1, _CTIER0 },
{ SV_POTION_APPLE_JUICE,			10, 1, _CTIER0 },
{ SV_POTION_SLIME_MOLD,				10, 1, _CTIER0 }, 
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_SLOWNESS,				20, 1, _CTIER0 },
{ SV_POTION_SALT_WATER,				20, 1, _CTIER0 },
{ SV_POTION_POISON,					30, 1, _CTIER0 },
{ SV_POTION_BLINDNESS,				30, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_CONFUSION,				30, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_SLEEP,					60, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_LOSE_MEMORIES,			10, 1, _CTIER0 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_RUINATION,				100, 40, _CTIER1 },
{ SV_POTION_DEC_STR,				40, 1, _CTIER0 },
{ SV_POTION_DEC_INT,				40, 1, _CTIER0 },
{ SV_POTION_DEC_WIS,				40, 1, _CTIER0 },
{ SV_POTION_DEC_DEX,				40, 1, _CTIER0 },
{ SV_POTION_DEC_CON,				40, 1, _CTIER0 },
{ SV_POTION_DEC_CHR,				40, 1, _CTIER0 },
{ SV_POTION_DETONATIONS,			100, 40, _CTIER1 },
{ SV_POTION_DEATH,					160, 40, _CTIER1 },
{ SV_POTION_INFRAVISION,			60, 5, _CTIER0 },
{ SV_POTION_DETECT_INVIS,			60, 5, _CTIER0 },
{ SV_POTION_SLOW_POISON,			20, 1, _CTIER0 },
{ SV_POTION_CURE_POISON,			40, 1, _CTIER0 },
{ SV_POTION_BOLDNESS,				60, 5, _CTIER1 },
{ SV_POTION_SPEED,					120, 10, _CTIER1 },
{ SV_POTION_RESIST_HEAT,			50, 5, _CTIER1 },
{ SV_POTION_RESIST_COLD,			50, 5, _CTIER1 },
{ SV_POTION_HEROISM,				60, 5, _CTIER0 },
{ SV_POTION_BERSERK_STRENGTH,		120, 25, _CTIER1 },
{ SV_POTION_CURE_LIGHT,				10, 1, _CTIER0 },
{ SV_POTION_CURE_SERIOUS,			30, 5, _CTIER0 },
{ SV_POTION_CURE_CRITICAL,			60, 15, _CTIER0 },
{ SV_POTION_HEALING,				200, 30, _CTIER1 },
{ SV_POTION_STAR_HEALING,			200, 40, _CTIER2 },
{ SV_POTION_LIFE,					400, 60, _CTIER2 },
{ SV_POTION_RESTORE_MANA,			180, 30, _CTIER1 },
{ SV_POTION_RESTORE_EXP,			60, 15, _CTIER0 },
{ SV_POTION_RES_STR,				60, 15, _CTIER0 },
{ SV_POTION_RES_INT,				60, 15, _CTIER0 },
{ SV_POTION_RES_WIS,				60, 15, _CTIER0 },
{ SV_POTION_RES_DEX,				60, 15, _CTIER0 },
{ SV_POTION_RES_CON,				60, 15, _CTIER0 },
{ SV_POTION_RES_CHR,				60, 15, _CTIER0 },
{ SV_POTION_INC_STR,				500, 30, _CTIER2 },
{ SV_POTION_INC_INT,				500, 30, _CTIER2 },
{ SV_POTION_INC_WIS,				500, 30, _CTIER2 },
{ SV_POTION_INC_DEX,				500, 30, _CTIER2 },
{ SV_POTION_INC_CON,				500, 30, _CTIER2 },
{ SV_POTION_INC_CHR,				500, 30, _CTIER2 },
{ -1, -1, 999 , _CTIER0 }, // ==========================================//
{ SV_POTION_AUGMENTATION,			1000, 999, _CTIER2 },
{ SV_POTION_ENLIGHTENMENT,			120, 30, _CTIER1 },
{ SV_POTION_STAR_ENLIGHTENMENT,		800, 40, _CTIER2 },
{ SV_POTION_SELF_KNOWLEDGE,			120, 30, _CTIER1 },
{ SV_POTION_EXPERIENCE,				1000, 999, _CTIER2 },
{ SV_POTION_RESISTANCE,				120, 15, _CTIER1 },
{ SV_POTION_CURING,					90, 20, _CTIER1 },
{ SV_POTION_INVULNERABILITY,		450, 60, _CTIER2 },
{ SV_POTION_NEW_LIFE,				240, 1, _CTIER2 },
{ SV_POTION_NEO_TSUYOSHI,			60, 1, _CTIER0 },
{ SV_POTION_TSUYOSHI,				30, 1, _CTIER0 },
{ SV_POTION_POLYMORPH,				120, 1, _CTIER1 },
{ SV_POTION_BLOOD,					120, 1, _CTIER0 },
{ SV_POTION_GIANT_STRENGTH,			120, 30, _CTIER1 },
{ SV_POTION_STONE_SKIN,				120, 30, _CTIER1 },
{ SV_POTION_CLARITY,				90, 20, _CTIER0 },
{ SV_POTION_GREAT_CLARITY,			120, 30, _CTIER1 },

{ -1, -1, 999 , _CTIER0 }, // null entry
};


static void _birth(void)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		memset(&_infusions[i], 0, sizeof(object_type));
	}
	for (i = 0; i <= _CTIER_MAX; i++)
	{
		_CHEM[i] = 0;
	}
	_CHEM[_CTIER0] = 240; 
	_CHEM[_CTIER1] = 120;
}

static object_type *_which_obj(int tval, int slot)
{
	assert(0 <= slot && slot < _MAX_INF_SLOTS);
	return _infusions + slot;
}

static _formula_info_t *_FindFormula(int sval){
	if (sval<0 || sval > POTION_MAX) return _formulas + POTION_MAX;

	if (sval == _formulas[sval].sval) return _formulas + sval;

	for (int i = 0; i < POTION_MAX; i++){
		if (sval == _formulas[i].sval) return _formulas + sval;
	}
	return _formulas + POTION_MAX;
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
	int DC = 1, tier = 0;

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
			DC = _FindFormula(o_ptr->sval)->minLv;
			tier = _FindFormula(o_ptr->sval)->ctier;
			object_desc(buf, o_ptr, OD_COLOR_CODED);
			doc_insert(doc, buf);

			if (DC <= alcskil){ // can do
				 doc_printf(doc, "<tab:%d>Cost: %4d%, %s, DC:<color:G>%3d%</color>\n", display.cx - 32, _FindFormula(o_ptr->sval)->cost, _tiername[tier], DC);
			} 
			else if (DC == 999){ // can't do ever
				doc_printf(doc, "<tab:%d><color:r>Unreproduceable</color>\n",display.cx - 22);
			}
			else {
				doc_printf(doc, "<tab:%d>Cost: %4d%, %s, DC:<color:R>%3d%</color>\n", display.cx - 32, _FindFormula(o_ptr->sval)->cost, _tiername[tier], DC);
			}

			/*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
		}
		else
			doc_insert_text(doc, TERM_L_DARK, "(None)\n");
	}


	doc_printf(doc, "\nAlchemist skill: <color:y>%4d%</color> \n", _AlchemistSkill());
	doc_printf(doc, "Chemical stock: \n");
		doc_printf(doc, "<tab:3><color:U>%s: %4d%</color> \n", _tiername[_CTIER0], _CHEM[_CTIER0]);
		doc_printf(doc, "<tab:3><color:o>%s: %4d%</color> \n", _tiername[_CTIER1], _CHEM[_CTIER1]);
		doc_printf(doc, "<tab:3><color:v>%s: %4d%</color> \n", _tiername[_CTIER2], _CHEM[_CTIER2]);
	doc_printf(doc, "\n");
	doc_insert(doc, "</style>");
	doc_sync_term(doc, doc_range_all(doc), doc_pos_create(pos.x, pos.y));
	doc_free(doc);
}

#define _ALLOW_EMPTY    0x01 /* Absorb */
#define _ALLOW_SWITCH   0x02 /* Browse/Use */
#define _ALLOW_EXCHANGE 0x04

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

void _use_infusion(object_type* o_ptr, int overdose)
{
	int  boost = device_power(100) - 100;
	u32b flgs[OF_ARRAY_SIZE];
	cptr used = NULL;
	int  uses = 1;

	energy_use = 100 - _AlchemistSkill(); // at best 30!

	obj_flags(o_ptr, flgs);
	
	if (o_ptr->number < uses || o_ptr->number == 0)
	{
		if (flush_failure) flush();
		msg_print("There are no enough infusions.");
		return;
	}

	if (o_ptr->sval == SV_POTION_DEATH
		|| o_ptr->sval == SV_POTION_RUINATION
		|| o_ptr->sval == SV_POTION_DETONATIONS){

		char prompt[255];
		char o_name[255];
		object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NO_PLURAL | OD_OMIT_PREFIX);
		sprintf(prompt, "Really use %s? <color:y>[y/N]</color>", o_name);
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return;
		else{
			sprintf(prompt, "Really, REALLY use %s? <color:y>[y/N]</color>", o_name);
			if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n') return;
		}
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

	o_ptr = _chooseInfusion("Use", tval, _ALLOW_SWITCH | _ALLOW_EXCHANGE);
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
	bool already_slotted = FALSE;
	
	for (int i = 0; i < _MAX_INF_SLOTS; i++){ // auto-assign if there is already one.
		if (_infusions[i].sval == src_ptr->sval && (src_ptr->tval == TV_POTION && _infusions[i].tval == TV_POTION) && _infusions[i].number < _INFUSION_CAP){ 
			dest_ptr = &_infusions[i]; already_slotted = TRUE; break; }
	}

	if (!already_slotted){
		dest_ptr = _chooseInfusion("Replace", src_ptr->tval, _ALLOW_EMPTY);
	}
	if (!dest_ptr)
		return FALSE;

	if (dest_ptr->number >= _INFUSION_CAP) {
		object_desc(o_name, dest_ptr, OD_OMIT_PREFIX);
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
	if (infct <= 0) { msg_format("You do nothing.");  return FALSE; }
	

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

bool _evaporate_aux(object_type *o_ptr){

	int blastType = 0;
	int power = 0;
	int dam = -1;
	int minPow = -1; int maxPow = -1;
	int rad = 2;
	int dir = 0;
	int plev = p_ptr->lev;
	cptr desc = "";

	char o_name[MAX_NLEN];
	if (o_ptr->tval == TV_POTION){
		switch (o_ptr->sval){
		case SV_POTION_APPLE_JUICE:
		case SV_POTION_SALT_WATER:		// Not GF_WATER because it is a goddamn beast.
		case SV_POTION_WATER:			blastType = GF_ACID;  dam = 10 + damroll(plev / 5, 10);		    break;
		case SV_POTION_BLINDNESS:		blastType = GF_DARK_WEAK; dam = (5 + plev) * 2;					break;
		case SV_POTION_DETONATIONS:		blastType = GF_ROCKET; dam = plev * 12; rad = 3;				break; // give it bit boost for being that rare.
		case SV_POTION_DEATH:			blastType = GF_DEATH_RAY; dam = plev * 200; rad = 1;			break; // powerful as shit
		case SV_POTION_RUINATION:		blastType = GF_TIME; dam = plev * 5;  minPow = 100;				break;
		case SV_POTION_LIFE:			blastType = GF_DISP_UNDEAD;	dam = plev * 40; maxPow = 2000;	    break; // should be super-powerful. Niche use.
		case SV_POTION_HEALING:			blastType = GF_DISP_UNDEAD;	dam = plev * 10; maxPow = 450;		break;
		case SV_POTION_STAR_HEALING:	blastType = GF_DISP_UNDEAD;	dam = plev * 15; maxPow = 850;	    break;
		case SV_POTION_SLEEP:			blastType = GF_OLD_SLEEP; dam = 25 + 7 * (plev / 10);		    break;

		case SV_POTION_DEC_CHR:
		case SV_POTION_DEC_STR:
		case SV_POTION_DEC_DEX:
		case SV_POTION_DEC_INT:
		case SV_POTION_DEC_WIS:
		case SV_POTION_DEC_CON:
			blastType = GF_TIME; dam = (5 + plev) * 2; 							break;
		case SV_POTION_CONFUSION:		blastType = GF_CONFUSION; dam = plev;							break;
		case SV_POTION_SLOWNESS:		blastType = GF_OLD_SLOW; dam = plev;							break;
		case SV_POTION_LOSE_MEMORIES:	blastType = GF_AMNESIA;	dam = plev;								break;
		case SV_POTION_BLOOD:			blastType = GF_BLOOD; dam = (30 + plev) * 2;					break;
		case SV_POTION_POISON:			blastType = GF_POIS; dam = (30 + plev) * 2; rad = 3;			break;
		case SV_POTION_BERSERK_STRENGTH:blastType = GF_BRAIN_SMASH;	dam = damroll(12, 12);				break;
		case SV_POTION_STONE_SKIN:		blastType = GF_PARALYSIS; dam = 25 + plev;						break; // turns to 'stone'
		case SV_POTION_SLIME_MOLD:		blastType = GF_STUN; dam = (5 + plev) * 2;						break;
		case SV_POTION_RESTORE_MANA:    blastType = GF_MANA; dam = (30 + plev) * 2 + 50;			    break;
		default: blastType = -1; // Other potions cannot be evaporated.
		}

		switch (o_ptr->sval){
		case SV_POTION_APPLE_JUICE:     desc = "It produces a blast of... apple juice?";	break;
		case SV_POTION_SALT_WATER:      desc = "It produces a blast of mist, and some salt.";	break;
		case SV_POTION_WATER:			desc = "It produces a blast of mist!";	break;
		case SV_POTION_BLINDNESS:		desc = "It produces a cloud of darkness";	break;
		case SV_POTION_DETONATIONS:		desc = "It produces an explosion!";	break;
		case SV_POTION_DEATH:			desc = "It produces a cloud of death!";	break;
		case SV_POTION_RUINATION:		desc = "It produces an eerie white mist!";	break;
		case SV_POTION_LIFE:			desc = "It produces a massive pillar of raw life energy!";	break;
		case SV_POTION_HEALING:			desc = "It produces a cloud of raw life energy!";	break;
		case SV_POTION_STAR_HEALING:	desc = "It produces a blast of raw life energy!";	break;
		case SV_POTION_SLEEP:			desc = "It produces a cloud of sleeping gas!";	break;
		case SV_POTION_DEC_CHR:
		case SV_POTION_DEC_STR:
		case SV_POTION_DEC_DEX:
		case SV_POTION_DEC_INT:
		case SV_POTION_DEC_WIS:
		case SV_POTION_DEC_CON:
			desc = "It a cloud of brown mist!";	break;
		case SV_POTION_CONFUSION:		desc = "It produces a scintillating cloud!";	break;
		case SV_POTION_SLOWNESS:		desc = "It produces a slow cloud!";	break;
		case SV_POTION_LOSE_MEMORIES:	desc = "It produces an ominous mist!";	break;
		case SV_POTION_BLOOD:			desc = "It produces a blast of blood!";	break;
		case SV_POTION_POISON:			desc = "It produces a cloud of poison!";	break;
		case SV_POTION_BERSERK_STRENGTH:desc = "It produces a cloud of fury!";	break;
		case SV_POTION_STONE_SKIN:		desc = "It produces a cloud of petrification!";	break;
		case SV_POTION_SLIME_MOLD:		desc = "It produces a blast of slime!";	break;
		case SV_POTION_RESTORE_MANA:    desc = "It produces a blast of raw mana!";	break;
		default: blastType = -1; // no allowing others.
		}
	}
	else if (o_ptr->tval == TV_FLASK){
		if (o_ptr->sval == SV_FLASK_OIL){
			desc = "The flask explodes into a cloud of fire!";
			dam = 12 + damroll(plev / 5, 12);	
			blastType = GF_FIRE; rad = 3;
		}
	}

	// Descriptions for sake of formatting.


	if (blastType < 0) { msg_format("You cannot evaporate this potion."); return FALSE; }
	if (!get_aim_dir(&dir)){ return FALSE; }

	// some special stuff.
	if (p_ptr->lev > 40 && blastType == GF_AMNESIA)  plev += (p_ptr->lev - 40) * 2;
		if (minPow >= 0 && minPow > dam) dam = minPow;
		if (maxPow >= 0 && maxPow < dam) dam = maxPow;
		power = spell_power(dam); 

	object_desc(o_name, o_ptr, OD_OMIT_PREFIX | OD_NO_PLURAL);
	msg_format("You evaporate %s. %s",o_name, desc);

	fire_ball_aux(
		blastType,
		dir,
		power,
		rad,
		PROJECT_FULL_DAM
		);


	return TRUE;
}

bool _object_is_evaporable(object_type *o_ptr)
{
	return (k_info[o_ptr->k_idx].tval == TV_POTION) || (k_info[o_ptr->k_idx].tval == TV_FLASK);
}

static bool evaporate(void){

	int item;
	object_type *o_ptr;
	bool EvapInf = TRUE;
	bool success = FALSE;

	char prompt[255];
	sprintf(prompt, "Evaporate [Q] Potion or [m] Infusion?\n");
	if (msg_prompt(prompt, "qm", PROMPT_DEFAULT) == 'q')
		EvapInf = FALSE;

	if (EvapInf == FALSE){
		item_tester_hook = _object_is_evaporable;
		if (!get_item(&item, "Evaporate which potion? ", "You have nothing to evaporate.", (USE_INVEN | USE_FLOOR)))
			return FALSE;

		if (item >= 0)
			o_ptr = &inventory[item];
		else
			o_ptr = &o_list[0 - item];

		if (o_ptr){
			success = _evaporate_aux(o_ptr);
		}

		if (success == TRUE){
			if (item >= 0)
			{
				inven_item_increase(item, -1);
				inven_item_describe(item);
				inven_item_optimize(item);
			}
			/* Eliminate the item (from the floor) */
			else
			{
				floor_item_increase(0 - item, -1);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
			}
		}
		else return FALSE;
	} 
	else{
		o_ptr = _chooseInfusion("Evaporate", TV_POTION, 0);
		if (!o_ptr){ return FALSE; }

		if (o_ptr->number < 1){
			msg_format("There's nothing to evaporate.");
			return FALSE;
		}

		success = _evaporate_aux(o_ptr);
		if (success == TRUE){
			o_ptr->number--;
		}

	}
	energy_use = 100;
	return TRUE;
}

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

bool alchemist_break_down_aux(object_type *o_ptr, int ct){
	char o_name[MAX_NLEN];
	int formulaCost = _FindFormula(o_ptr->sval)->cost;
	int cost = (ct * formulaCost) / 2;
	int tier = _FindFormula(o_ptr->sval)->ctier;
	if (_CHEM[tier] + cost > _MAX_CHEM){
		char prompt[255];
		object_desc(o_name, o_ptr, OD_OMIT_PREFIX);
		sprintf(prompt, "Really break down %s? You will exceed the chemical, and some will be lost. <color:y>[y/N]</color>", o_name);
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return FALSE;
	}

	_CHEM[tier] += cost;
	if (_CHEM[tier] > _MAX_CHEM) _CHEM[tier] = _MAX_CHEM;

	msg_format("You break down potion(s) and gain %d %s chemical, totaling at %d.", cost, _tiername[tier], _CHEM[tier]);

}

static bool break_down_potion(void){
	int item;
	object_type *o_ptr;

	item_tester_hook = object_is_potion;
	if (!get_item(&item, "Break down which potions? ", "You have nothing to break down.", (USE_INVEN | USE_FLOOR)))
		return FALSE;

	if (item >= 0) o_ptr = &inventory[item];
	else o_ptr = &o_list[0 - item];

	int ct = get_quantity(NULL, o_ptr->number);
	bool success = FALSE;
	if (ct > 0){

			if (o_ptr->sval == SV_POTION_WATER){
				if(randint0(100)<10) msg_print("It's just H2O, funny guy.");
				else msg_print("It's just plain water.");
				
				return FALSE;
			} // 

			success = alchemist_break_down_aux(o_ptr, ct);
	}
	
	if (success){
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

	int cost = infct * _FindFormula(o_ptr->sval)->cost;
	int tier = _FindFormula(o_ptr->sval)->ctier;
	if (_FindFormula(o_ptr->sval)->minLv > _AlchemistSkill()){
		msg_format("This infusion is beyond your skills to reproduce.");
		return;
	}
	
	if (infct < 1 || tier<0 || tier>2){ // do notting
		return;
	}

	char prompt[255];
	object_desc(o_name, o_ptr, OD_COLOR_CODED);
	sprintf(prompt, "Reproduction would cost %d chemicals. Are you sure?", cost);
	if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
		return;

	if (cost > _CHEM[tier]){ 
		msg_format("You do not have enough %s chemicals.", _tiername[tier]);
		return;
	}

	if (infct > 0)
	{
		msg_format("You recreate %d infusions.", infct);
		o_ptr->number += infct;
		_CHEM[tier] -= cost;
	}
	else
		energy_use = 0;

}

static bool reproduceInfusion(void){
	object_type *o_ptr;
	o_ptr = _chooseInfusion("Reproduce", TV_POTION, 0);
	if (o_ptr){ _reproduceInf(o_ptr); return TRUE; }
	return FALSE;
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

void alchemist_super_potion_effect(int sval){

	switch (sval)
	{
		case SV_POTION_ENLIGHTENMENT: set_tim_esp(100 + randint0(p_ptr->lev * 2), FALSE); break;
		case SV_POTION_STAR_ENLIGHTENMENT: set_tim_esp(300 + randint0(p_ptr->lev * 6), FALSE); break;
		case SV_POTION_CLARITY: set_confused(0, FALSE); break;
		case SV_POTION_GREAT_CLARITY: set_confused(0, FALSE); set_stun(0, FALSE); set_image(0, FALSE); break;
	}

}

static void _calc_bonuses(void){
	if (p_ptr->lev >= 10) p_ptr->regen += 100;
	if (p_ptr->lev >= 20) 
	{
		add_flag(p_ptr->weapon_info[0].flags, OF_BRAND_POIS);
		add_flag(p_ptr->weapon_info[1].flags, OF_BRAND_POIS);
		add_flag(p_ptr->shooter_info.flags, OF_BRAND_POIS);
	}
	if (p_ptr->lev >= 40)
	{
		add_flag(p_ptr->weapon_info[0].flags, OF_BRAND_ACID);
		add_flag(p_ptr->weapon_info[1].flags, OF_BRAND_ACID);
		add_flag(p_ptr->shooter_info.flags, OF_BRAND_ACID);
	}
	int boost = 0;

	if (IS_SHERO()){ // extra benefits from things.
		boost = 2 + p_ptr->lev / 5;

		p_ptr->weapon_info[0].xtra_blow += py_prorata_level_aux(100, 0, 1, 1);
		p_ptr->weapon_info[1].xtra_blow += py_prorata_level_aux(100, 0, 1, 1);

		p_ptr->to_h_m += boost;
		p_ptr->to_d_m += boost;

		p_ptr->pspeed += 4;
		p_ptr->shooter_info.num_fire += p_ptr->lev * 150 / 50;
	}
	else if (IS_HERO()){
		boost = 1+p_ptr->lev / 10;
		p_ptr->pspeed += 2;
		p_ptr->to_h_m += boost;
		p_ptr->to_d_m += boost;
		p_ptr->shooter_info.num_fire += p_ptr->lev * 150 / 80;
	}

}

static bool _on_destroy_object(object_type *o_ptr)
{
	if (object_is_potion(o_ptr))
	{
		char o_name[MAX_NLEN];
		object_desc(o_name, o_ptr, OD_COLOR_CODED);
		msg_format("You attempt to break down %s. ", o_name);
		alchemist_break_down_aux(o_ptr, o_ptr->number);
		return TRUE;
	}
	return FALSE;
}



static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
	if (p_ptr->lev >= 20)
		add_flag(flgs, OF_BRAND_POIS);
	if (p_ptr->lev >= 40)
		add_flag(flgs, OF_BRAND_ACID);
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

	for (i = 0; i <= _CTIER_MAX; i++){
		_CHEM[i] = savefile_read_s32b(file);
	}
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
	for (i = 0; i <= _CTIER_MAX; i++){
		savefile_write_s32b(file, (s32b)_CHEM[i]); // save chemical count.
	}
	
}

static void _save_player(savefile_ptr file)
{
	_save_list(file);
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

static caster_info * _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "alchemy";
		me.weight = 1000;
		init = TRUE;
	}
	me.which_stat = A_INT;
	return &me;
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
	spell->level = 5;
	spell->cost = 8;
	spell->fail = calculate_fail_rate(spell->level, 25, p_ptr->stat_ind[A_INT]); 
	spell->fn = _evaporate_spell;

	spell = &spells[ct++];
	spell->level = 20;
	spell->cost = 5;
	spell->fail = calculate_fail_rate(spell->level, 30, p_ptr->stat_ind[A_INT]);
	spell->fn = alchemy_spell;

	return ct;
}

class_t *alchemist_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	/* static info never changes */
	if (!init)
	{           
		/* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 30, 30, 34, 6, 50, 24, 68, 58 };
		skills_t xs = { 15, 9, 10, 0, 0, 0, 22, 22 };

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
			"for some of their abilities.\n"
			"{ EXPERIMENTAL CLASS }";

		me.stats[A_STR] = 0;
		me.stats[A_INT] = 2;
		me.stats[A_WIS] = -1;
		me.stats[A_DEX] = 2;
		me.stats[A_CON] = 0;
		me.stats[A_CHR] = -2;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 105;
		me.base_hp = 12;
		me.exp = 125;
		me.pets = 30;

		me.birth = _birth;
		me.get_powers = _get_powers;
		me.calc_bonuses = _calc_bonuses;
		me.character_dump = _character_dump;
		me.get_flags = _get_flags;
		me.caster_info = _caster_info;
		me.load_player = _load_player;
		me.save_player = _save_player;
		me.destroy_object = _on_destroy_object;

		init = TRUE;
	}

	return &me;
}
