#include "angband.h"

#include <assert.h>

/** Based on Magic Eater code, if it wasn't obvious.**/

#define _MAX_INF_SLOTS 8
#define _INVALID_SLOT -1
#define _INFUSION_CAP 30

static object_type _infusions[_MAX_INF_SLOTS];

static void _birth(void)
{
	int i;
	for (i = 0; i < _MAX_INF_SLOTS; i++)
	{
		memset(&_infusions[i], 0, sizeof(object_type));
	}
}

static object_type *_which_obj(int tval, int slot)
{
	assert(0 <= slot && slot < _MAX_INF_SLOTS);
	return _infusions + slot;
}

static void _displayInfusions(rect_t display)
{
	char    buf[MAX_NLEN];
	int     i;
	point_t pos = rect_topleft(&display);
	int     padding, max_o_len = 20;
	doc_ptr doc = NULL;
	object_type *list = _infusions;


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

			object_desc(buf, o_ptr, OD_COLOR_CODED);
			doc_insert(doc, buf);
			doc_printf(doc, "\n", display.cx - 12,"");
			/*doc_printf(doc, "<tab:%d>SP: %3d.%2.2d\n", display.cx - 12, o_ptr->xtra5 / 100, o_ptr->xtra5 % 100);*/
		}
		else
			doc_insert_text(doc, TERM_L_DARK, "(None)\n");
	}
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
			string_printf(prompt, "%s which %s", "Inject", "infusion");
			if (options & _ALLOW_SWITCH)
			{
				if (options & _ALLOW_EXCHANGE)
					string_append_s(prompt, ", 'X' to Exchange");
				string_append_s(prompt, "]:");
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

void _use_infusion(object_type* o_ptr, int n, int overdose)
{
	int  boost = device_power(100) - 100;
	u32b flgs[OF_ARRAY_SIZE];
	bool used = FALSE;
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
	
	used = device_use(o_ptr, boost);
	if (used)
	{
		msg_print("There are no enough infusions.");
		o_ptr->number += uses;
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
		_use_object(o_ptr);
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

	
	if (dest_ptr->k_idx && dest_ptr->sval != src_ptr->sval)
	{
		char prompt[255];
		object_desc(o_name, dest_ptr, OD_COLOR_CODED);
		sprintf(prompt, "Really replace %s? <color:y>[y/N]</color>", o_name);
		if (msg_prompt(prompt, "ny", PROMPT_DEFAULT) == 'n')
			return FALSE;
	}

	int infct = get_quantity(NULL, MIN(src_ptr->number,_INFUSION_CAP));

	if (dest_ptr->sval == src_ptr->sval){ // we already got one, so just increment them!
			
		if (dest_ptr->number + infct > _INFUSION_CAP) infct = 30 - dest_ptr->number;
		if (infct < 0) infct = 0;

		dest_ptr->number += infct;
		object_desc(o_name, src_ptr, OD_COLOR_CODED);
		msg_format("You create additional infusions out of %s.", o_name);
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
