/**********************************************************************
 * The Weaponsmith
 *
 * This is a complete rewrite of the Hengband classic, with a few
 * gameplay changes. I've kept Hengband's implementation strategy
 * for modifying the object_type after smithing, but had to make
 * changes since object_type.xtra3 is just a byte.
 *
 **********************************************************************/
#include "angband.h"

#include <assert.h>

/**********************************************************************
 * Essence Classification
 **********************************************************************/
#define _MAX_ESSENCE 255
#define _MIN_SPECIAL 240  /* cf TR_FLAG_MAX */
#define _ESSENCE_NONE -1  /* 0 is actually a valid essence: TR_STR */

/* Essence IDs: These are generally an object flag (TR_*) but can be something
   special. The id is stored as object_type.xtra3 = id + 1 */
enum {
    _ESSENCE_AC = _MIN_SPECIAL,
    _ESSENCE_TO_HIT,
    _ESSENCE_TO_DAM,
    _ESSENCE_XTRA_DICE,
    _ESSENCE_XTRA_MIGHT,
    _ESSENCE_TO_HIT_A,  /* Slaying Gloves no longer use weapon enchantments! */
    _ESSENCE_TO_DAM_A,
    _ESSENCE_SPECIAL    /* 247 We are about to overflow the byte object.xtra3! Use xtra1 from more codes. */
};

enum {                  /* stored in object.xtra1 when object.xtra3 - 1 = _ESSENCE_SPECIAL */
    _ESSENCE_RES_BASE = 1,
    _ESSENCE_SUST_ALL,
    _ESSENCE_SLAYING,   /* object.xtra4 packs (+h,+d) in s16b */
};

/* Essences are grouped by type for display to the user */
enum {
    ESSENCE_TYPE_ENCHANT,
    ESSENCE_TYPE_STATS,
    ESSENCE_TYPE_BONUSES,
    ESSENCE_TYPE_SLAYS,
    ESSENCE_TYPE_BRANDS,
    ESSENCE_TYPE_RESISTS,
    ESSENCE_TYPE_SUSTAINS,
    ESSENCE_TYPE_ABILITIES,
    ESSENCE_TYPE_TELEPATHY,
    ESSENCE_TYPE_MAX
};

typedef struct {
    int  id;
    cptr name;
    int  cost;
    int  info; /* ammo div or pval max, etc */
} _essence_info_t, *_essence_info_ptr;

#define _MAX_INFO_PER_TYPE 24
typedef struct {
    int             type;
    cptr            name;
    _essence_info_t entries[_MAX_INFO_PER_TYPE];
} _essence_group_t, *_essence_group_ptr;

#define _COST_TO_HIT    10
#define _COST_TO_DAM    20
#define _COST_TO_AC     30
#define _COST_TO_HIT_A  15
#define _COST_TO_DAM_A  30
#define _COST_RUSTPROOF 30
#define _ART_ENCH_MULT   3

/* Essence Table: Indexed by ESSENCE_TYPE_* */
static _essence_group_t _essence_groups[ESSENCE_TYPE_MAX] = {
    { ESSENCE_TYPE_ENCHANT, "Enchantments", {
        { _ESSENCE_TO_HIT,   "Weapon Accuracy", _COST_TO_HIT, 10 },
        { _ESSENCE_TO_DAM,   "Weapon Damage", _COST_TO_DAM, 10 },
        { _ESSENCE_TO_HIT_A, "Slaying Accuracy", _COST_TO_HIT_A, 0 },
        { _ESSENCE_TO_DAM_A, "Slaying Damage", _COST_TO_DAM_A, 0 },
        { _ESSENCE_AC,       "Armor Class", _COST_TO_AC, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_STATS, "Stats", {
        { TR_STR, "Strength", 20, 0 },
        { TR_INT, "Intelligence", 20, 0 },
        { TR_WIS, "Wisdom", 20, 0 },
        { TR_DEX, "Dexterity", 20, 0 },
        { TR_CON, "Constitution", 20, 0 },
        { TR_CHR, "Charisma", 20, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_BONUSES, "Bonuses", {
        { TR_SPEED, "Speed", 12, 0 },
        { TR_STEALTH, "Stealth", 15, 0 },
        { TR_LIFE, "Life", 50, 5 },
        { TR_BLOWS, "Extra Attacks", 20, 3 },
        { _ESSENCE_XTRA_DICE, "Extra Dice", 250, 4 },
        { _ESSENCE_XTRA_MIGHT, "Extra Might", 250, 4 },
        { TR_XTRA_SHOTS, "Extra Shots", 50, 4 },
        { TR_MAGIC_MASTERY, "Magic Mastery", 12, 0 },
        { TR_TUNNEL, "Digging", 10, 0 },
        { TR_INFRA, "Infravision", 10, 0 },
        { TR_SEARCH, "Searching", 10, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_SLAYS, "Slays", {
        { TR_SLAY_EVIL, "Slay Evil", 100, 10 },
        { TR_SLAY_GOOD, "Slay Good", 90, 10 },
        { TR_SLAY_LIVING, "Slay Living", 80, 10 },
        { TR_SLAY_UNDEAD, "Slay Undead", 20, 10 },
        { TR_SLAY_DEMON, "Slay Demon", 20, 10 },
        { TR_SLAY_DRAGON, "Slay Dragon", 20, 10 },
        { TR_SLAY_HUMAN, "Slay Human", 20, 10 },
        { TR_SLAY_ANIMAL, "Slay Animal", 20, 10 },
        { TR_SLAY_ORC, "Slay Orc", 15, 10 },
        { TR_SLAY_TROLL, "Slay Troll", 15, 10 },
        { TR_SLAY_GIANT, "Slay Giant", 20, 10 },
        { TR_KILL_EVIL, "Kill Evil", 100, 10 },
        { TR_KILL_UNDEAD, "Kill Undead", 30, 10 },
        { TR_KILL_DEMON, "Kill Demon", 30, 10 },
        { TR_KILL_DRAGON, "Kill Dragon", 30, 10 },
        { TR_KILL_HUMAN, "Kill Human", 30, 10 },
        { TR_KILL_ANIMAL, "Kill Animal", 30, 10 },
        { TR_KILL_ORC, "Kill Orc", 20, 10 },
        { TR_KILL_TROLL, "Kill Troll", 20, 10 },
        { TR_KILL_GIANT, "Kill Giant", 30, 10 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_BRANDS, "Brands", {
        { TR_BRAND_ACID, "Brand Acid", 20, 10 },
        { TR_BRAND_ELEC, "Brand Elec", 20, 10 },
        { TR_BRAND_FIRE, "Brand Fire", 20, 10 },
        { TR_BRAND_COLD, "Brand Cold", 20, 10 },
        { TR_BRAND_POIS, "Brand Poison", 20, 10 },
        { TR_CHAOTIC, "Chaotic", 20, 0 },
        { TR_VAMPIRIC, "Vampiric", 60, 0 },
        { TR_IMPACT, "Impact", 20, 0 },
        { TR_STUN, "Stun", 50, 0 },
        { TR_VORPAL, "Vorpal", 100, 0 },
        { TR_VORPAL2, "*Vorpal*", 100, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_RESISTS, "Resists", {
        { TR_RES_ACID, "Resist Acid", 15, 0 },
        { TR_RES_ELEC, "Resist Elec", 15, 0 },
        { TR_RES_FIRE, "Resist Fire", 15, 0 },
        { TR_RES_COLD, "Resist Cold", 15, 0 },
        { _ESSENCE_SPECIAL, "Resist Base", 50, _ESSENCE_RES_BASE },
        { TR_RES_POIS, "Resist Poison", 30, 0 },
        { TR_RES_LITE, "Resist Light", 30, 0 },
        { TR_RES_DARK, "Resist Dark", 30, 0 },
        { TR_RES_CONF, "Resist Conf", 20, 0 },
        { TR_RES_NETHER, "Resist Nether", 30, 0 },
        { TR_RES_NEXUS, "Resist Nexus", 30, 0 },
        { TR_RES_SOUND, "Resist Sound", 40, 0 },
        { TR_RES_SHARDS, "Resist Shards", 40, 0 },
        { TR_RES_CHAOS, "Resist Chaos", 40, 0 },
        { TR_RES_DISEN, "Resist Disench", 30, 0 },
        { TR_RES_TIME, "Resist Time", 20, 0 },
        { TR_RES_BLIND, "Resist Blind", 20, 0 },
        { TR_RES_FEAR, "Resist Fear", 20, 0 },
        { TR_NO_TELE, "Resist Tele", 20, 0 },
        { TR_IM_ACID, "Immune Acid", 20, 0 },
        { TR_IM_ELEC, "Immune Elec", 20, 0 },
        { TR_IM_FIRE, "Immune Fire", 20, 0 },
        { TR_IM_COLD, "Immune Cold", 20, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_SUSTAINS, "Sustains", {
        { TR_SUST_STR, "Sust Str", 15, 0 },
        { TR_SUST_INT, "Sust Int", 15, 0 },
        { TR_SUST_WIS, "Sust Wis", 15, 0 },
        { TR_SUST_DEX, "Sust Dex", 15, 0 },
        { TR_SUST_CON, "Sust Con", 15, 0 },
        { TR_SUST_CHR, "Sust Chr", 15, 0 },
        { _ESSENCE_SPECIAL, "Sustaining", 50, _ESSENCE_SUST_ALL },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_ABILITIES, "Abilities", {
        { TR_FREE_ACT, "Free Action", 20, 0 },
        { TR_SEE_INVIS, "See Invisible", 30, 0 },
        { TR_HOLD_LIFE, "Hold Life", 20, 0 },
        { TR_SLOW_DIGEST, "Slow Digestion", 15, 0 },
        { TR_REGEN, "Regeneration", 50, 0 },
        { TR_DUAL_WIELDING, "Dual Wielding", 50, 0 },
        { TR_NO_MAGIC, "Antimagic", 15, 0 },
        { TR_WARNING, "Warning", 20, 0 },
        { TR_LEVITATION, "Levitation", 20, 0 },
        { TR_REFLECT, "Reflection", 20, 0 },
        { TR_SH_FIRE, "Aura Fire", 20, 0 },
        { TR_SH_ELEC, "Aura Elec", 20, 0 },
        { TR_SH_COLD, "Aura Cold", 20, 0 },
        { TR_SH_SHARDS, "Aura Shards", 30, 0 },
        { TR_SH_REVENGE, "Revenge", 40, 0 },
        { TR_LITE, "Extra Light", 15, 0 },
        { TR_IGNORE_ACID, "Rustproof", _COST_RUSTPROOF, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },

    { ESSENCE_TYPE_TELEPATHY, "Telepathy", {
        { TR_TELEPATHY, "Telepathy", 40, 0 },
        { TR_ESP_ANIMAL, "Sense Animals", 30, 0 },
        { TR_ESP_UNDEAD, "Sense Undead", 40, 0 },
        { TR_ESP_DEMON, "Sense Demon", 40, 0 },
        { TR_ESP_ORC, "Sense Orc", 20, 0 },
        { TR_ESP_TROLL, "Sense Troll", 20, 0 },
        { TR_ESP_GIANT, "Sense Giant", 20, 0 },
        { TR_ESP_DRAGON, "Sense Dragon", 30, 0 },
        { TR_ESP_HUMAN, "Sense Human", 20, 0 },
        { TR_ESP_EVIL, "Sense Evil", 40, 0 },
        { TR_ESP_GOOD, "Sense Good", 20, 0 },
        { TR_ESP_NONLIVING, "Sense Nonliving", 40, 0 },
        { TR_ESP_UNIQUE, "Sense Unique", 20, 0 },
        { _ESSENCE_NONE, NULL, 0, 0 } } },
};

static _essence_info_ptr _find_essence_info(int id)
{
    int i, j;
    for (i = 0; i < ESSENCE_TYPE_MAX; i++)
    {
        _essence_group_ptr group_ptr = &_essence_groups[i];
        for (j = 0; j < _MAX_INFO_PER_TYPE; j++)
        {
            _essence_info_ptr info_ptr = &group_ptr->entries[j];
            if (info_ptr->id == _ESSENCE_NONE) break;
            if (info_ptr->id == id) return info_ptr;
        }
    }
    return NULL;
}

/**********************************************************************
 * Essence Absorption
 **********************************************************************/

/* Storage of acquired essences. For now, this is a flat table with many
   unused slots. We could switch to an int_map for more efficiency if desired.
   Note the restriction of direct access to _essences[].
   We no longer use p_ptr->magic_num[108] and all the weird mappings that implied.
   Savefiles broke for 4.0.2 with no effort to upgrade :( */
static int _essences[_MAX_ESSENCE] = {0};

static int _get_essence(int which)
{
    assert(0 <= which && which < _MAX_ESSENCE);
    return _essences[which];
}

static bool _set_essence(int which, int n)
{
    int old;

    assert(0 <= which && which < _MAX_ESSENCE);
    old = _essences[which];

    if (n < 0) n = 0;
    if (n > 30000) n = 30000;

    if (n != old)
    {
        _essences[which] = n;
        return TRUE;
    }

    return FALSE;
}

static bool _add_essence(int which, int amount)
{
    return _set_essence(which, _get_essence(which) + amount);
}

static void _clear_essences(void)
{
    int i;

    for (i = 0; i < _MAX_ESSENCE; i++)
        _set_essence(i, 0);
}

static int _count_essences_aux(int type)
{
    _essence_group_ptr group_ptr = &_essence_groups[type];
    int                i;
    int                ct = 0;
    for (i = 0; i < _MAX_INFO_PER_TYPE; i++)
    {
        _essence_info_ptr info_ptr = &group_ptr->entries[i];
        if (info_ptr->id == _ESSENCE_NONE) break;
        ct += _get_essence(info_ptr->id);
    }
    return ct;
}

static int _count_essences(void)
{
    int i;
    int ct = 0;
    for (i = 0; i < ESSENCE_TYPE_MAX; i++)
        ct += _count_essences_aux(i);
    return ct;
}

/* Savefile persistence */
static void _load(savefile_ptr file)
{
    int ct, i;

    _clear_essences();

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        int j = savefile_read_s16b(file);
        int n = savefile_read_s32b(file);

        if (0 <= j && j < _MAX_ESSENCE)
            _add_essence(j, n);
    }
}

static void _save(savefile_ptr file)
{
    int ct = 0, i;

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        if (_get_essence(i))
            ct++;
    }

    savefile_write_s16b(file, ct);

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        int n = _get_essence(i);
        if (n)
        {
            savefile_write_s16b(file, i);
            savefile_write_s32b(file, n);
        }
    }
}

/* Absorption */
static void _absorb_one_aux(int which, cptr name, int amt)
{
    if (amt > 0 && _add_essence(which, amt))
        msg_format("\nYou gain <color:B>%s</color>: %d", name, amt);
}

static void _absorb_one(_essence_info_ptr info, int amt)
{
    assert(info);
    _absorb_one_aux(info->id, info->name, amt);
}

static void _absorb_one_quiet(_essence_info_ptr info, int amt)
{
    assert(info);
    if (amt > 0)
        _add_essence(info->id, amt);
}

typedef void (*_absorb_essence_f)(_essence_info_ptr info, int amt);

static void _absorb_all(object_type *o_ptr, _absorb_essence_f absorb_f)
{
    int          i,j;
    int          div = 1;
    int          mult = o_ptr->number;
    u32b         old_flgs[TR_FLAG_SIZE], new_flgs[TR_FLAG_SIZE];
    object_type  old_obj = *o_ptr;
    object_type  new_obj = {0};

    object_flags(&old_obj, old_flgs);

    /* Mundanity */
    object_prep(&new_obj, o_ptr->k_idx);
    new_obj.iy = old_obj.iy;
    new_obj.ix = old_obj.ix;
    new_obj.next_o_idx = old_obj.next_o_idx;
    new_obj.marked = old_obj.marked;
    new_obj.number = old_obj.number;
    if (old_obj.tval == TV_DRAG_ARMOR) new_obj.timeout = old_obj.timeout;
    new_obj.ident |= (IDENT_FULL);
    object_aware(&new_obj);
    object_known(&new_obj);
    object_flags(&new_obj, new_flgs);

    /* Ammo and Curses */
    if (o_ptr->curse_flags & (TRC_CURSED | TRC_HEAVY_CURSE | TRC_PERMA_CURSE)) div++;
    if (have_flag(old_flgs, TR_AGGRAVATE)) div++;
    if (have_flag(old_flgs, TR_NO_TELE)) div++;
    if (have_flag(old_flgs, TR_DRAIN_EXP)) div++;
    if (have_flag(old_flgs, TR_TY_CURSE)) div++;

    if (object_is_ammo(&old_obj))
        div *= 10;

    /* Normal Handling */
    for (i = ESSENCE_TYPE_STATS; i < ESSENCE_TYPE_MAX; i++)
    {
        _essence_group_ptr group_ptr = &_essence_groups[i];
        assert(i == group_ptr->type);

        for (j = 0; j < _MAX_INFO_PER_TYPE; j++)
        {
            _essence_info_ptr info_ptr = &group_ptr->entries[j];

            if (info_ptr->id == _ESSENCE_NONE) break;

            if (info_ptr->id == _ESSENCE_XTRA_DICE) continue;
            if (info_ptr->id == _ESSENCE_XTRA_MIGHT) continue;
            if (info_ptr->id == _ESSENCE_SPECIAL) continue;

            assert(info_ptr->id < TR_FLAG_COUNT);
            if (have_flag(old_flgs, info_ptr->id))
            {
                if (i == ESSENCE_TYPE_STATS || i == ESSENCE_TYPE_BONUSES)
                {
                    int pval = old_obj.pval;

                    if (have_flag(new_flgs, info_ptr->id))
                        pval -= new_obj.pval;

                    if (pval > 0)
                        absorb_f(info_ptr, pval*10*mult/div);
                }
                else if (!have_flag(new_flgs, info_ptr->id))
                    absorb_f(info_ptr, 10*mult/div);
            }
        }
    }

    /* Special Handling */
    if (object_is_weapon_ammo(&old_obj) && !have_flag(old_flgs, TR_ORDER) && !have_flag(old_flgs, TR_WILD))
    {
        if (old_obj.ds > new_obj.ds)
            absorb_f(_find_essence_info(_ESSENCE_XTRA_DICE), (old_obj.ds - new_obj.ds)*10*mult/div);
        if (old_obj.dd > new_obj.dd)
            absorb_f(_find_essence_info(_ESSENCE_XTRA_DICE), (old_obj.dd - new_obj.dd)*10*mult/div);
    }

    if (old_obj.tval == TV_BOW && old_obj.mult > new_obj.mult)
        absorb_f(_find_essence_info(_ESSENCE_XTRA_MIGHT), (old_obj.mult - new_obj.mult)*mult/div);

    if (old_obj.to_a > new_obj.to_a)
        absorb_f(_find_essence_info(_ESSENCE_AC), (old_obj.to_a - new_obj.to_a)*10*mult/div);

    if (object_is_weapon_ammo(&old_obj))
    {
        if (old_obj.to_h > new_obj.to_h)
            absorb_f(_find_essence_info(_ESSENCE_TO_HIT), (old_obj.to_h - new_obj.to_h)*10*mult/div);
        if (old_obj.to_d > new_obj.to_d)
            absorb_f(_find_essence_info(_ESSENCE_TO_DAM), (old_obj.to_d - new_obj.to_d)*10*mult/div);
    }
    else if (object_is_armour(&old_obj))
    {
        if (old_obj.to_h > new_obj.to_h)
            absorb_f(_find_essence_info(_ESSENCE_TO_HIT_A), (old_obj.to_h - new_obj.to_h)*10*mult/div);
        if (old_obj.to_d > new_obj.to_d)
            absorb_f(_find_essence_info(_ESSENCE_TO_DAM_A), (old_obj.to_d - new_obj.to_d)*10*mult/div);
    }

    *o_ptr = new_obj;
}

static void _remove(object_type *o_ptr)
{
    u32b flgs[TR_FLAG_SIZE];

    if (o_ptr->xtra3 == 1+_ESSENCE_SPECIAL)
    {
        if (o_ptr->xtra1 == _ESSENCE_SLAYING )
        {
            o_ptr->to_h -= (o_ptr->xtra4>>8);
            o_ptr->to_d -= (o_ptr->xtra4 & 0x000f);
            o_ptr->xtra4 = 0;
            if (o_ptr->to_h < 0 && o_ptr->name2 != EGO_GLOVES_BERSERKER) o_ptr->to_h = 0;
            if (o_ptr->to_d < 0) o_ptr->to_d = 0;
        }
        o_ptr->xtra1 = 0;
    }
    else if (o_ptr->xtra3 == 1+_ESSENCE_XTRA_DICE)
    {
        o_ptr->dd -= o_ptr->xtra4;
        o_ptr->xtra4 = 0;
        if (o_ptr->dd < 1) o_ptr->dd = 1;
    }
    else if (o_ptr->xtra3 == 1+_ESSENCE_XTRA_MIGHT)
    {
        o_ptr->mult -= o_ptr->xtra4*25;
        o_ptr->xtra4 = 0;
        if (o_ptr->mult < 100) o_ptr->dd = 100;
    }
    o_ptr->xtra3 = 0;
    object_flags(o_ptr, flgs);
    if (!have_pval_flags(flgs))
        o_ptr->pval = 0;
}

static bool _on_destroy_object(object_type *o_ptr)
{
    if (object_is_weapon_armour_ammo(o_ptr))
    {
        char o_name[MAX_NLEN];
        object_desc(o_name, o_ptr, OD_COLOR_CODED);
        msg_format("You attempt to drain power from %s.", o_name);
        _absorb_all(o_ptr, _absorb_one);
        return TRUE;
    }
    return FALSE;
}

/**********************************************************************
 * Smithing
 **********************************************************************/

/* Cost Calculations */
#define _MAX_PVAL 15  /* Boots of Feanor can reach +15 */
const int _pval_factor[_MAX_PVAL + 1] = {
      0,
    100,  225,  375,  550,  750, /* +5 */
   1000, 1300, 1700, 2200, 2800, /* +6 */
   3500, 4300, 5200, 6200, 7300  /* +15 */
};

static int _calc_pval_cost(int pval, int cost)
{
    if (pval < 0) pval = 0;
    if (pval > _MAX_PVAL) pval = _MAX_PVAL;
    return cost * _pval_factor[pval] / 100;
}

#define _MAX_ENCH 20
const int _ench_factor[_MAX_ENCH + 1] = {
       0,
     100,  200,  300,  400,  500,  /* +5 */
     600,  700,  800,  900, 1000,  /* +10 */
    1200, 1500, 2000, 2700, 3600,  /* +15 */
    5000, 7000,10000,15000,25000   /* +20 */
};
static int _enchant_limit(void)
{
    return 5 + py_prorata_level_aux(150, 1, 0, 2) / 10;
}

static int _calc_enchant_cost(int bonus, int cost)
{
    if (bonus < 0)
        return cost * _ench_factor[1] * bonus / 100;
    if (bonus > _MAX_ENCH) bonus = _MAX_ENCH;
    return cost * _ench_factor[bonus] / 100;
}

/* User Interface (This is a lot of work!)
   [1] We'll use a document to build menus for the user. This is static
       for convenience, but it is also needed as an absorption hook parameter
       so the user can view the essences they will gain.*/
static doc_ptr _doc = NULL;

/* [2] Smithing is a class power, now. First we pick an object.*/
static bool _smithing(void);

/* [3] Then we enter a top level menu loop. The user can perform
       multiple actions on the object, such as removing an existing
       essence and adding a new one, as well as enchanting the object's
       bonuses to hit and damage. After each action, the changed object
       is redisplayed to the user so they can view the progress of
       their work. */
static void _smith_object(object_type *o_ptr);

/* [4] The top level menu will then dispatch to second level menus. Here
       the user may ESC to return to the main menu, or Q to quit smithing
       altogether. */
#define _UNWIND 1
#define _OK 0
static int _smith_absorb(object_type *o_ptr);
static int _smith_remove(object_type *o_ptr);
static int _smith_enchant(object_type *o_ptr);
static int _smith_enchant_armor(object_type *o_ptr);
static int _smith_enchant_weapon(object_type *o_ptr);
static int _smith_add_essence(object_type *o_ptr, int type);
static int _smith_add_pval(object_type *o_ptr, int type);
static int _smith_add_slaying(object_type *o_ptr);

/* Absorption */
static void _absorb_one_spy(_essence_info_ptr info, int amt)
{
    assert(info);
    assert(_doc);
    doc_printf(_doc, "      You will gain <color:B>%s</color>: %d\n", info->name, amt);
}

static int _smith_absorb(object_type *o_ptr)
{
    rect_t      r = ui_map_rect();
    object_type copy = *o_ptr;

    doc_clear(_doc);
    obj_display_smith(o_ptr, _doc);
    doc_insert(_doc, " <color:y>  A</color>) Absorb all essences from this object\n");
    _absorb_all(&copy, _absorb_one_spy);
    doc_insert(_doc, "\n <color:y>ESC</color>) Return to main menu\n");
    doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
    doc_newline(_doc);

    Term_load();
    doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

    for (;;)
    {
        switch (inkey_special(TRUE))
        {
        case ESCAPE:
            return _OK;
        case 'Q': case 'q':
            return _UNWIND;
        case 'A': case 'a':
            _absorb_all(o_ptr, _absorb_one_quiet);
            return _OK;
        }
    }
}

/* Remove added essence */
static int _smith_remove(object_type *o_ptr)
{
    rect_t r = ui_map_rect();
    cptr   name = "Unknown";
    int    id = o_ptr->xtra3 - 1;

    if (id != _ESSENCE_SPECIAL)
    {
        _essence_info_ptr info_ptr = _find_essence_info(id);
        assert(info_ptr);
        name = info_ptr->name;
    }
    else
    {
        switch (o_ptr->xtra1)
        {
        case _ESSENCE_SLAYING:
            name = "Slaying";
            break;
        case _ESSENCE_RES_BASE:
            name = "Resist Base";
            break;
        case _ESSENCE_SUST_ALL:
            name = "Sustaining";
            break;
        }
    }

    doc_clear(_doc);
    obj_display_smith(o_ptr, _doc);

    doc_printf(_doc, " <color:y>  R</color>) Remove added <color:B>%s</color> essence from this object\n", name);

    doc_insert(_doc, "\n <color:y>ESC</color>) Return to main menu\n");
    doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
    doc_newline(_doc);

    Term_load();
    doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

    for (;;)
    {
        switch (inkey_special(TRUE))
        {
        case ESCAPE:
            return _OK;
        case 'Q': case 'q':
            return _UNWIND;
        case 'R': case 'r':
            _remove(o_ptr);
            return _OK;
        }
    }
}

/* Enchantments */
static int _calc_enchant_to_a(object_type *o_ptr, int to_a)
{
    int    mult = o_ptr->number;
    int    cost;

    if (object_is_artifact(o_ptr))
        mult *= _ART_ENCH_MULT;

    cost  = _calc_enchant_cost(to_a, _COST_TO_AC);
    cost -= _calc_enchant_cost(o_ptr->to_a, _COST_TO_AC);
    cost *= mult;

    return cost;
}
static int _smith_enchant_armor(object_type *o_ptr)
{
    rect_t r = ui_map_rect();
    int    max = _enchant_limit();
    int    to_a = o_ptr->to_a;
    int    avail_a = _get_essence(_ESSENCE_AC);
    int    avail_rustproof = 0;
    bool   can_rustproof = FALSE;
    int    cost_a = 0;

    {
        u32b   flgs[TR_FLAG_SIZE];
        object_flags(o_ptr, flgs);
        if (!have_flag(flgs, TR_IGNORE_ACID))
        {
            can_rustproof = TRUE;
            avail_rustproof = _get_essence(TR_IGNORE_ACID);
        }
    }

    if (to_a < max)
    {
        to_a = max;
        cost_a = _calc_enchant_to_a(o_ptr, to_a);
        while (to_a > o_ptr->to_a && cost_a > avail_a)
        {
            to_a--;
            cost_a = _calc_enchant_to_a(o_ptr, to_a);
        }
    }

    for (;;)
    {
        int  cmd;
        char color;

        cost_a = _calc_enchant_to_a(o_ptr, to_a);

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        doc_printf(_doc, " <color:%c>  E</color>) Enchant to ",
            (cost_a > avail_a) ? 'D' : 'y');

        if (to_a == o_ptr->to_a) color = 'w';
        else if (to_a == max) color = 'r';
        else color = 'R';
        doc_printf(_doc, "[%d,<color:%c>%+d</color>]\n", o_ptr->ac, color, to_a);


        doc_insert(_doc, "      Use a/A to adust the amount of armor class to add.\n");

        if (cost_a > avail_a) color = 'r';
        else color = 'G';
        doc_printf(_doc, "      This will cost <color:%c>%d</color> out of %d essences of <color:B>Armor Class</color>.\n",
            color, cost_a, avail_a);

        if (can_rustproof)
        {
            if (_COST_RUSTPROOF > avail_rustproof) color = 'D';
            else color = 'y';
            doc_printf(_doc, "\n <color:%c>  R</color>) Rustproof this armor\n", color);
            if (_COST_RUSTPROOF > avail_rustproof) color = 'r';
            else color = 'G';
            doc_printf(_doc, "      This will cost <color:%c>%d</color> out of %d essences of <color:B>Rustproof</color>.\n",
                color, _COST_RUSTPROOF, avail_rustproof);
        }

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ESC</color>) Return to main menu\n");
        doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        switch (cmd)
        {
        case ESCAPE:
            return _OK;
        case 'Q': case 'q':
            return _UNWIND;
        case 'a':
            if (to_a > o_ptr->to_a)
                to_a--;
            break;
        case 'A':
            if (to_a < max)
                to_a++;
            break;
        case 'E': case 'e':
            if (cost_a > avail_a)
                break;
            o_ptr->to_a = to_a;
            _add_essence(_ESSENCE_AC, -cost_a);
            return _OK;
        case 'R': case 'r':
            if (can_rustproof && avail_rustproof >= _COST_RUSTPROOF)
            {
                add_flag(o_ptr->art_flags, TR_IGNORE_ACID);
                can_rustproof = FALSE;
                _add_essence(TR_IGNORE_ACID, -_COST_RUSTPROOF);
            }
            break;
        }
    }
}

static int _calc_enchant_to_h(object_type *o_ptr, int to_h)
{
    int    mult = o_ptr->number;
    int    div = 1;
    int    cost;

    if (object_is_ammo(o_ptr))
        div = 10;

    if (object_is_artifact(o_ptr))
        mult *= _ART_ENCH_MULT;

    cost  = _calc_enchant_cost(to_h, _COST_TO_HIT);
    cost -= _calc_enchant_cost(o_ptr->to_h, _COST_TO_HIT);
    cost = cost * mult / div;

    return cost;
}
static int _calc_enchant_to_d(object_type *o_ptr, int to_d)
{
    int    mult = o_ptr->number;
    int    div = 1;
    int    cost;

    if (object_is_ammo(o_ptr))
        div = 10;

    if (object_is_artifact(o_ptr))
        mult *= _ART_ENCH_MULT;

    cost  = _calc_enchant_cost(to_d, _COST_TO_DAM);
    cost -= _calc_enchant_cost(o_ptr->to_d, _COST_TO_DAM);
    cost = cost * mult / div;

    return cost;
}
static int _smith_enchant_weapon(object_type *o_ptr)
{
    rect_t r = ui_map_rect();
    int    max = _enchant_limit();
    int    to_h = o_ptr->to_h;
    int    to_d = o_ptr->to_d;
    int    avail_h = _get_essence(_ESSENCE_TO_HIT);
    int    avail_d = _get_essence(_ESSENCE_TO_DAM);
    int    cost_h = 0, cost_d = 0;

    if (to_h < max)
    {
        to_h = max;
        cost_h = _calc_enchant_to_h(o_ptr, to_h);
        while (to_h > o_ptr->to_h && cost_h > avail_h)
        {
            to_h--;
            cost_h = _calc_enchant_to_h(o_ptr, to_h);
        }
    }
    if (to_d < max)
    {
        to_d = max;
        cost_d = _calc_enchant_to_d(o_ptr, to_d);
        while (to_d > o_ptr->to_d && cost_d > avail_d)
        {
            to_d--;
            cost_d = _calc_enchant_to_d(o_ptr, to_d);
        }
    }

    for (;;)
    {
        int  cmd;
        char color;

        cost_h = _calc_enchant_to_h(o_ptr, to_h);
        cost_d = _calc_enchant_to_d(o_ptr, to_d);

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        doc_printf(_doc, " <color:%c>  E</color>) Enchant to ",
            (cost_h > avail_h || cost_d > avail_d) ? 'D' : 'y');

        if (to_h == o_ptr->to_h) color = 'w';
        else if (to_h == max) color = 'r';
        else color = 'R';
        doc_printf(_doc, "(<color:%c>%+d</color>", color, to_h);

        if (to_d == o_ptr->to_d) color = 'w';
        else if (to_d == max) color = 'r';
        else color = 'R';
        doc_printf(_doc, ",<color:%c>%+d</color>)\n", color, to_d);

        doc_insert(_doc, "      Use h/H to adust the amount of accuracy to add.\n");
        doc_insert(_doc, "      Use d/D to adust the amount of damage to add.\n");

        if (cost_h > avail_h) color = 'r';
        else color = 'G';
        doc_printf(_doc, "      This will cost <color:%c>%d</color> out of %d essences of <color:B>Weapon Accuracy</color>.\n",
            color, cost_h, avail_h);

        if (cost_d > avail_d) color = 'r';
        else color = 'G';
        doc_printf(_doc, "      This will cost <color:%c>%d</color> out of %d essences of <color:B>Weapon Damage</color>.\n",
            color, cost_d, avail_d);

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ESC</color>) Return to main menu\n");
        doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        switch (cmd)
        {
        case ESCAPE:
            return _OK;
        case 'Q': case 'q':
            return _UNWIND;
        case 'h':
            if (to_h > o_ptr->to_h)
                to_h--;
            break;
        case 'H':
            if (to_h < max)
                to_h++;
            break;
        case 'd':
            if (to_d > o_ptr->to_d)
                to_d--;
            break;
        case 'D':
            if (to_d < max)
                to_d++;
            break;
        case 'E': case 'e':
            if (cost_h > avail_h || cost_d > avail_d)
                break;
            o_ptr->to_h = to_h;
            o_ptr->to_d = to_d;
            _add_essence(_ESSENCE_TO_HIT, -cost_h);
            _add_essence(_ESSENCE_TO_DAM, -cost_d);
            return _OK;
        }
    }
}

static int _smith_enchant(object_type *o_ptr)
{
    if (object_is_weapon_ammo(o_ptr))
        return _smith_enchant_weapon(o_ptr);
    return _smith_enchant_armor(o_ptr);
}

/* Gauntlets of Slaying ... but now, any armor will do :) */
static int _calc_enchant_to_h_a(object_type *o_ptr, int to_h)
{
    int mult = o_ptr->number;

    if (object_is_artifact(o_ptr))
        mult *= _ART_ENCH_MULT;

    return _calc_enchant_cost(to_h, _COST_TO_HIT_A) * mult;
}
static int _calc_enchant_to_d_a(object_type *o_ptr, int to_d)
{
    int mult = o_ptr->number;

    if (object_is_artifact(o_ptr))
        mult *= _ART_ENCH_MULT;

    return _calc_enchant_cost(to_d, _COST_TO_DAM_A) * mult;
}
static int _smith_add_slaying(object_type *o_ptr)
{
    rect_t r = ui_map_rect();
    int    max = _enchant_limit();
    int    to_h = max;
    int    to_d = max;
    int    avail_h = _get_essence(_ESSENCE_TO_HIT_A);
    int    avail_d = _get_essence(_ESSENCE_TO_DAM_A);
    int    cost_h = 0, cost_d = 0;

    cost_h = _calc_enchant_to_h_a(o_ptr, to_h);
    while (to_h > o_ptr->to_h && cost_h > avail_h)
    {
        to_h--;
        cost_h = _calc_enchant_to_h_a(o_ptr, to_h);
    }

    cost_d = _calc_enchant_to_d_a(o_ptr, to_d);
    while (to_d > o_ptr->to_d && cost_d > avail_d)
    {
        to_d--;
        cost_d = _calc_enchant_to_d_a(o_ptr, to_d);
    }

    for (;;)
    {
        int  cmd;
        char color;

        cost_h = _calc_enchant_to_h_a(o_ptr, to_h);
        cost_d = _calc_enchant_to_d_a(o_ptr, to_d);

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        doc_printf(_doc, " <color:%c>  S</color>) Slaying power of ",
            (cost_h > avail_h || cost_d > avail_d) ? 'D' : 'y');

        if (to_h == 0) color = 'w';
        else if (to_h == max) color = 'r';
        else color = 'R';
        doc_printf(_doc, "(<color:%c>%+d</color>", color, to_h);

        if (to_d == 0) color = 'w';
        else if (to_d == max) color = 'r';
        else color = 'R';
        doc_printf(_doc, ",<color:%c>%+d</color>)\n", color, to_d);

        doc_insert(_doc, "      Use h/H to adust the amount of accuracy to add.\n");
        doc_insert(_doc, "      Use d/D to adust the amount of damage to add.\n");

        if (cost_h > avail_h) color = 'r';
        else color = 'G';
        doc_printf(_doc, "      This will cost <color:%c>%d</color> out of %d essences of <color:B>Slaying Accuracy</color>.\n",
            color, cost_h, avail_h);

        if (cost_d > avail_d) color = 'r';
        else color = 'G';
        doc_printf(_doc, "      This will cost <color:%c>%d</color> out of %d essences of <color:B>Slaying Damage</color>.\n",
            color, cost_d, avail_d);

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ESC</color>) Return to main menu\n");
        doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        switch (cmd)
        {
        case ESCAPE:
            return _OK;
        case 'Q': case 'q':
            return _UNWIND;
        case 'h':
            if (to_h > 0)
                to_h--;
            break;
        case 'H':
            if (to_h < max)
                to_h++;
            break;
        case 'd':
            if (to_d > 0)
                to_d--;
            break;
        case 'D':
            if (to_d < max)
                to_d++;
            break;
        case 'S': case 's':
            if (cost_h > avail_h || cost_d > avail_d)
                break;
            if (cost_h == 0 && cost_d == 0)
                break;
            to_h = to_h/2 + randint0((to_h+1)/2 + 1);
            to_d = to_d/2 + randint0((to_d+1)/2 + 1);
            o_ptr->to_h += to_h;
            o_ptr->to_d += to_d;
            o_ptr->xtra3 = _ESSENCE_SPECIAL + 1;
            o_ptr->xtra1 = _ESSENCE_SLAYING;
            o_ptr->xtra4 = (to_h<<8) + to_d;
            _add_essence(_ESSENCE_TO_HIT_A, -cost_h);
            _add_essence(_ESSENCE_TO_DAM_A, -cost_d);
            return _OK;
        }
    }
}

/* Resists, Slays, Brands, Sustains, Abilities, Telepathy */
static int _smith_add_essence(object_type *o_ptr, int type)
{
    _essence_group_ptr  group_ptr = &_essence_groups[type];
    rect_t              r = ui_map_rect();
    vec_ptr             choices = vec_alloc(NULL);
    bool                done = FALSE;
    int                 result = _OK;
    bool                is_ammo = object_is_ammo(o_ptr);

    /* Build list of choices. The player needs some essences of
       the required type, and we avoid adding a redundant ability
       (that the player is aware of) */
    {
        u32b flgs[TR_FLAG_SIZE];
        int  i;

        object_flags_known(o_ptr, flgs);

        for (i = 0; i < _MAX_INFO_PER_TYPE; i++)
        {
            _essence_info_ptr info_ptr = &group_ptr->entries[i];
            if (info_ptr->id == _ESSENCE_NONE) break;

            if (info_ptr->id < TR_FLAG_COUNT)
            {
                if (info_ptr->id == TR_IGNORE_ACID) continue; /* Rustproofing is handled by 'Enchant' */
                if (!_get_essence(info_ptr->id)) continue;
                if (have_flag(flgs, info_ptr->id)) continue;

                /* TODO: Perhaps we should add filters to our tables? */
                if (info_ptr->id == TR_DUAL_WIELDING && !object_is_armour(o_ptr)) continue; /* Boots of Genji! Yes!! :D */
            }
            else
            {
                assert(info_ptr->id == _ESSENCE_SPECIAL);
                if (info_ptr->info == _ESSENCE_SUST_ALL)
                {
                    if ( !_get_essence(TR_SUST_STR)
                      && !_get_essence(TR_SUST_INT)
                      && !_get_essence(TR_SUST_WIS)
                      && !_get_essence(TR_SUST_DEX)
                      && !_get_essence(TR_SUST_CON)
                      && !_get_essence(TR_SUST_CHR) )
                    {
                        continue;
                    }

                    if ( have_flag(flgs, TR_SUST_STR)
                      && have_flag(flgs, TR_SUST_INT)
                      && have_flag(flgs, TR_SUST_WIS)
                      && have_flag(flgs, TR_SUST_DEX)
                      && have_flag(flgs, TR_SUST_CON)
                      && have_flag(flgs, TR_SUST_CHR) )
                    {
                        continue;
                    }
                }
                else if (info_ptr->info == _ESSENCE_RES_BASE)
                {
                    if ( !_get_essence(TR_RES_ACID)
                      && !_get_essence(TR_RES_ELEC)
                      && !_get_essence(TR_RES_FIRE)
                      && !_get_essence(TR_RES_COLD) )
                    {
                        continue;
                    }

                    if ( have_flag(flgs, TR_RES_ACID)
                      && have_flag(flgs, TR_RES_ELEC)
                      && have_flag(flgs, TR_RES_FIRE)
                      && have_flag(flgs, TR_RES_COLD) )
                    {
                        continue;
                    }
                }
            }

            if (is_ammo && !info_ptr->info) continue;

            vec_add(choices, info_ptr);
        }
    }

    while (!done)
    {
        int  cmd;
        int  choice = -1;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        if (vec_length(choices))
        {
            int       ct = vec_length(choices);
            doc_ptr   cols[2] = {0};
            int       i;
            const int max_rows = 15;
            int       wrap_row = 999;
            int       doc_idx = 0;

            assert(ct <= 26); /* I'm using 'a' to 'z' for choices ... */

            if (ct > max_rows) /* 80x27 */
            {
                cols[0] = doc_alloc(35);
                cols[1] = doc_alloc(35);
                wrap_row = (ct + 1) / 2;
            }
            else
            {
                cols[0] = _doc;
                cols[1] = _doc;
            }

            doc_printf(cols[doc_idx], "<color:G>      %-15.15s  Cost  Avail</color>\n", group_ptr->name);
            for (i = 0; i < vec_length(choices); i++)
            {
                _essence_info_ptr info_ptr = vec_get(choices, i);
                int               cost = info_ptr->cost * o_ptr->number;

                if (is_ammo)
                    cost /= info_ptr->info;

                if (i == wrap_row)
                {
                    doc_idx++;
                    doc_printf(cols[doc_idx], "<color:G>      %-15.15s  Cost  Avail</color>\n", group_ptr->name);
                }

                if (info_ptr->id == _ESSENCE_SPECIAL)
                {
                    if (info_ptr->info == _ESSENCE_SUST_ALL)
                    {
                        bool ok = TRUE;
                        if (cost > _get_essence(TR_SUST_STR)) ok = FALSE;
                        else if (cost > _get_essence(TR_SUST_INT)) ok = FALSE;
                        else if (cost > _get_essence(TR_SUST_WIS)) ok = FALSE;
                        else if (cost > _get_essence(TR_SUST_DEX)) ok = FALSE;
                        else if (cost > _get_essence(TR_SUST_CON)) ok = FALSE;
                        else if (cost > _get_essence(TR_SUST_CHR)) ok = FALSE;
                        doc_printf(cols[doc_idx], " <color:%c>  %c</color>) %-15.15s  <color:%c>%4d</color>\n",
                            ok ? 'y' : 'D',
                            'A' + i,
                            info_ptr->name,
                            ok ? 'G' : 'r',
                            cost
                        );
                    }
                    else if (info_ptr->info == _ESSENCE_RES_BASE)
                    {
                        bool ok = TRUE;
                        if (cost > _get_essence(TR_RES_ACID)) ok = FALSE;
                        else if (cost > _get_essence(TR_RES_ELEC)) ok = FALSE;
                        else if (cost > _get_essence(TR_RES_FIRE)) ok = FALSE;
                        else if (cost > _get_essence(TR_RES_COLD)) ok = FALSE;
                        doc_printf(cols[doc_idx], " <color:%c>  %c</color>) %-15.15s  <color:%c>%4d</color>\n",
                            ok ? 'y' : 'D',
                            'A' + i,
                            info_ptr->name,
                            ok ? 'G' : 'r',
                            cost
                        );
                    }
                }
                else
                {
                    int avail = _get_essence(info_ptr->id);
                    doc_printf(cols[doc_idx], " <color:%c>  %c</color>) %-15.15s  <color:%c>%4d</color>  %5d\n",
                        (cost > avail) ? 'D' : 'y',
                        'A' + i,
                        info_ptr->name,
                        (cost > avail) ? 'r' : 'G',
                        cost,
                        avail
                    );
                }
            }
            if (ct > max_rows)
            {
                doc_insert_cols(_doc, cols, 2, 0);
                doc_free(cols[0]);
                doc_free(cols[1]);
            }
        }
        else
            doc_printf(_doc, "      <color:r>You cannot add any further %s to this object.</color>\n", group_ptr->name);

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ESC</color>) Return to main menu\n");

        if (vec_length(choices) < 17)
            doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        if ('a' <= cmd && cmd <= 'z')
            choice = cmd - 'a';
        else if ('A' <= cmd && cmd <= 'Z')
            choice = cmd - 'A';

        if (choice >= 0 && choice < vec_length(choices))
        {
            _essence_info_ptr info_ptr = vec_get(choices, choice);
            int               cost = info_ptr->cost * o_ptr->number;

            if (is_ammo)
                cost /= info_ptr->info;

            if (info_ptr->id == _ESSENCE_SPECIAL)
            {
                if (info_ptr->info == _ESSENCE_SUST_ALL)
                {
                    if ( cost <= _get_essence(TR_SUST_STR)
                      && cost <= _get_essence(TR_SUST_INT)
                      && cost <= _get_essence(TR_SUST_WIS)
                      && cost <= _get_essence(TR_SUST_DEX)
                      && cost <= _get_essence(TR_SUST_CON)
                      && cost <= _get_essence(TR_SUST_CHR) )
                    {
                        o_ptr->xtra3 = _ESSENCE_SPECIAL + 1;
                        o_ptr->xtra1 = _ESSENCE_SUST_ALL;
                        _add_essence(TR_SUST_STR, -cost);
                        _add_essence(TR_SUST_INT, -cost);
                        _add_essence(TR_SUST_WIS, -cost);
                        _add_essence(TR_SUST_DEX, -cost);
                        _add_essence(TR_SUST_CON, -cost);
                        _add_essence(TR_SUST_CHR, -cost);
                        done = TRUE;
                    }
                }
                else if (info_ptr->info == _ESSENCE_RES_BASE)
                {
                    if ( cost <= _get_essence(TR_RES_ACID)
                      && cost <= _get_essence(TR_RES_ELEC)
                      && cost <= _get_essence(TR_RES_FIRE)
                      && cost <= _get_essence(TR_RES_COLD) )
                    {
                        o_ptr->xtra3 = _ESSENCE_SPECIAL + 1;
                        o_ptr->xtra1 = _ESSENCE_RES_BASE;
                        _add_essence(TR_RES_ACID, -cost);
                        _add_essence(TR_RES_ELEC, -cost);
                        _add_essence(TR_RES_FIRE, -cost);
                        _add_essence(TR_RES_COLD, -cost);
                        done = TRUE;
                    }
                }
            }
            else
            {
                int avail = _get_essence(info_ptr->id);

                if (cost <= avail)
                {
                    o_ptr->xtra3 = info_ptr->id + 1;
                    _add_essence(info_ptr->id, -cost);
                    done = TRUE;
                }
            }
        }
        else if (cmd == ESCAPE)
        {
            done = TRUE;
        }
        else if (cmd == 'Q' || cmd == 'q') /* This might be unreachable */
        {
            result = _UNWIND;
            done = TRUE;
        }
    }
    vec_free(choices);
    return result;
}

/* Stats or Bonuses: Logic is a bit complicated by the fact that 2
   of the pval bonuses aren't really using pvals. In general, the pval
   of the object overrides any user entered value, but for these
   two exceptions (Extra Dice and Extra Might), the user needs to
   be able to enter a value to use that differs from the pval in question.
   I think it is working ... */
static int _smith_add_pval(object_type *o_ptr, int type)
{
    _essence_group_ptr  group_ptr = &_essence_groups[type];
    rect_t              r = ui_map_rect();
    vec_ptr             choices = vec_alloc(NULL);
    bool                done = FALSE;
    int                 result = _OK;
    int                 pval = o_ptr->pval; /* Entered by the user, but o_ptr->pval usually overrides */
    int                 max_pval = 5;

    /* Build list of choices. The player needs some essences of
       the required type, and we avoid adding a redundant ability
       (that the player is aware of) */
    {
        u32b flgs[TR_FLAG_SIZE];
        int  i;

        object_flags_known(o_ptr, flgs);

        for (i = 0; i < _MAX_INFO_PER_TYPE; i++)
        {
            _essence_info_ptr info_ptr = &group_ptr->entries[i];
            if (info_ptr->id == _ESSENCE_NONE) break;
            if (!_get_essence(info_ptr->id)) continue;

            if (info_ptr->id < TR_FLAG_COUNT && have_flag(flgs, info_ptr->id)) continue;

            /* TODO: Perhaps we should add filters to our tables? */
            if (info_ptr->id == TR_XTRA_SHOTS && !object_is_bow(o_ptr)) continue;
            if (info_ptr->id == _ESSENCE_XTRA_MIGHT && !object_is_bow(o_ptr)) continue;
            if (info_ptr->id == _ESSENCE_XTRA_DICE && !object_is_melee_weapon(o_ptr)) continue;
            if (info_ptr->id == TR_BLOWS && !object_is_melee_weapon(o_ptr)) continue;

            if (o_ptr->pval && (info_ptr->id == _ESSENCE_XTRA_MIGHT || info_ptr->id == _ESSENCE_XTRA_DICE))
                max_pval = info_ptr->info;

            vec_add(choices, info_ptr);
        }
    }

    if (!pval)
        pval = max_pval;

    if (pval < 0) /* paranoia ... we shouldn't be called in this case! Also, there shouldn't *be* any negative pvals! */
        return _OK;

    if (pval > max_pval)
        pval = max_pval;

    while (!done)
    {
        int  cmd;
        int  choice = -1;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        if (vec_length(choices))
        {
            int i;
            bool do_pval_warning = FALSE;

            assert(vec_length(choices) <= 26);

            doc_printf(_doc, "<color:G>      %-15.15s  Plus  Cost  Avail</color>\n", group_ptr->name);
            for (i = 0; i < vec_length(choices); i++)
            {
                _essence_info_ptr info_ptr = vec_get(choices, i);
                int               avail = _get_essence(info_ptr->id);
                int               plus;
                char              plus_color = 'y';
                int               cost;
                bool              capped = FALSE;

                if (info_ptr->id >= _MIN_SPECIAL)
                {
                    plus = pval;
                    if (info_ptr->info && plus >= info_ptr->info)
                    {
                        plus = info_ptr->info;
                        plus_color = 'r';
                        capped = TRUE;
                    }
                }
                else if (o_ptr->pval)
                {
                    plus = o_ptr->pval;
                    plus_color = 'D';
                    if (info_ptr->info && plus >= info_ptr->info)
                    {
                        plus = info_ptr->info;
                        plus_color = 'v';
                        capped = TRUE;
                    }
                }
                else
                {
                    plus = pval;
                    if (info_ptr->info && plus >= info_ptr->info)
                    {
                        plus = info_ptr->info;
                        plus_color = 'r';
                        capped = TRUE;
                    }
                }

                cost = _calc_pval_cost(plus, info_ptr->cost * o_ptr->number);

                doc_printf(_doc, " <color:%c>  %c</color>) %-15.15s  <color:%c>%4d</color>  <color:%c>%4d</color>  %5d",
                    (cost > avail) ? 'D' : 'y',
                    'A' + i,
                    info_ptr->name,
                    plus_color,
                    plus,
                    (cost > avail) ? 'r' : 'G',
                    cost,
                    avail
                );


                if (capped)
                {
                    if (plus < o_ptr->pval && info_ptr->id < TR_FLAG_COUNT)
                    {
                        doc_printf(_doc, " <color:v>Bonus is capped at %+d<color:o>*</color></color>", info_ptr->info);
                        do_pval_warning = TRUE;
                    }
                    else
                        doc_printf(_doc, " <color:R>Bonus is capped at %+d</color>", info_ptr->info);
                }
                doc_newline(_doc);
            }
            if (!o_ptr->pval || type == ESSENCE_TYPE_BONUSES)
            {
                if (o_ptr->pval)
                    doc_printf(_doc, "\n      <indent>Use +/- or type '1' to '%d' to adjust the amount of bonus to use. For most bonuses, the plus on the object will override any value you choose.</indent>\n", max_pval);
                else
                    doc_printf(_doc, "\n      Use +/- or type '1' to '%d' to adjust the amount of bonus to use.\n", max_pval);
            }
            if (do_pval_warning)
                doc_insert(_doc, "\n      <color:o>*</color> <indent><color:v>Choosing this option will reduce the bonus of this object affecting other attributes!</color></indent>\n");
        }
        else
            doc_printf(_doc, "      <color:r>You cannot add any further %s to this object.</color>\n", group_ptr->name);

        doc_newline(_doc);
        doc_insert(_doc, " <color:y>ESC</color>) Return to main menu\n");

        if (vec_length(choices) < 17)
            doc_insert(_doc, " <color:y>  Q</color>) Quit work on this object\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        if ('a' <= cmd && cmd <= 'z')
            choice = cmd - 'a';
        else if ('A' <= cmd && cmd <= 'Z')
            choice = cmd - 'A';

        if (choice >= 0 && choice < vec_length(choices))
        {
            _essence_info_ptr info_ptr = vec_get(choices, choice);
            int               avail = _get_essence(info_ptr->id);
            int               plus;
            int               cost;

            if (info_ptr->id >= _MIN_SPECIAL)
                plus = pval;
            else if (o_ptr->pval)
                plus = o_ptr->pval;
            else
                plus = pval;

            if (info_ptr->info && plus >= info_ptr->info)
                plus = info_ptr->info;

            cost = _calc_pval_cost(plus, info_ptr->cost * o_ptr->number);

            if (cost <= avail)
            {
                o_ptr->xtra3 = info_ptr->id + 1;
                _add_essence(info_ptr->id, -cost);
                done = TRUE;
                if (info_ptr->id < TR_FLAG_COUNT)
                {
                    o_ptr->pval = plus;
                }
                else if (info_ptr->id == _ESSENCE_XTRA_DICE)
                {
                    o_ptr->dd += plus;
                    o_ptr->xtra4 = plus;
                }
                else if (info_ptr->id == _ESSENCE_XTRA_MIGHT)
                {
                    o_ptr->mult += 25 * plus;
                    o_ptr->xtra4 = plus;
                }
            }
        }
        else if (cmd == ESCAPE)
        {
            done = TRUE;
        }
        else if (cmd == 'Q' || cmd == 'q') /* This might be unreachable */
        {
            result = _UNWIND;
            done = TRUE;
        }
        else if (cmd == '+')
        {
            if (pval < max_pval) pval++;
        }
        else if (cmd == '-')
        {
            if (pval > 1) pval--;
        }
        else if ('1' <= cmd && cmd <= '9')
        {
            int val = cmd - '0';
            if (1 <= val && val <= max_pval)
                pval = val;
        }
    }
    vec_free(choices);
    return result;
}

/* top level 'menu' processing */
static void _character_dump_aux(doc_ptr doc);
static bool _can_enchant(object_type *o_ptr)
{
    u32b flgs[TR_FLAG_SIZE];
    object_flags(o_ptr, flgs);
    if (have_flag(flgs, TR_NO_ENCHANT)) /* Harps, Guns, Runeswords, Kamikaze Robes, etc. */
        return FALSE;
    return TRUE;
}
static void _list_current_essences(void)
{
    doc_ptr doc = doc_alloc(80);
    doc_insert(doc, "<style:wide>");
    _character_dump_aux(doc);
    doc_insert(doc, "</style>");
    doc_display(doc, "Current Essences", 0);
    doc_free(doc);
}
static void _smith_weapon_armor(object_type *o_ptr)
{
    bool   done = FALSE;
    bool   can_enchant = _can_enchant(o_ptr);
    rect_t r = ui_map_rect();

    while (!done)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        doc_printf(_doc, "   <color:%c>E</color>) Enchant this object\n", can_enchant ? 'y' : 'D');

        doc_newline(_doc);
        doc_insert(_doc, "   <color:y>A</color>) Absorb all essences\n");
        if (object_is_smith(o_ptr))
            doc_insert(_doc, "   <color:y>R</color>) Remove added essence\n");
        else if (object_is_artifact(o_ptr))
        {
        }
        else
        {
            doc_insert(_doc, "\n   <color:y>1</color>) Add Statistic\n");
            doc_insert(_doc, "   <color:y>2</color>) Add Bonus\n");
            doc_insert(_doc, "   <color:y>3</color>) Add Resistance\n");
            doc_insert(_doc, "   <color:y>4</color>) Add Sustain\n");
            doc_insert(_doc, "   <color:y>5</color>) Add Ability\n");
            doc_insert(_doc, "   <color:y>6</color>) Add Telepathy\n");
            if (object_is_melee_weapon(o_ptr))
            {
                doc_insert(_doc, "   <color:y>7</color>) Add Slay\n");
                doc_insert(_doc, "   <color:y>8</color>) Add Brand\n");
            }
            else if (object_is_armour(o_ptr))
            {
                if (_get_essence(_ESSENCE_TO_HIT_A) || _get_essence(_ESSENCE_TO_DAM_A))
                    doc_insert(_doc, "   <color:y>7</color>) Add Slaying\n");
            }
        }

        doc_newline(_doc);
        doc_insert(_doc, "   <color:y>L</color>) List current essences\n");
        doc_insert(_doc, "   <color:y>Q</color>) Quit work on this object\n");
        doc_newline(_doc);
        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        switch (cmd)
        {
        case 'L': case 'l':
            _list_current_essences();
            break;
        case ESCAPE:
        case 'Q': case 'q':
            done = TRUE;
            break;
        case 'A': case 'a':
            if (_smith_absorb(o_ptr) == _UNWIND)
                done = TRUE;
            break;
        case 'R': case 'r':
            if (object_is_smith(o_ptr) && _smith_remove(o_ptr) == _UNWIND)
                done = TRUE;
            break;
        case 'E': case 'e':
            if (can_enchant && _smith_enchant(o_ptr) == _UNWIND)
                done = TRUE;
            break;
        case '1':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_pval(o_ptr, ESSENCE_TYPE_STATS) == _UNWIND)
                done = TRUE;
            break;
        case '2':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_pval(o_ptr, ESSENCE_TYPE_BONUSES) == _UNWIND)
                done = TRUE;
            break;
        case '3':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_essence(o_ptr, ESSENCE_TYPE_RESISTS) == _UNWIND)
                done = TRUE;
            break;
        case '4':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_essence(o_ptr, ESSENCE_TYPE_SUSTAINS) == _UNWIND)
                done = TRUE;
            break;
        case '5':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_essence(o_ptr, ESSENCE_TYPE_ABILITIES) == _UNWIND)
                done = TRUE;
            break;
        case '6':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_essence(o_ptr, ESSENCE_TYPE_TELEPATHY) == _UNWIND)
                done = TRUE;
            break;
        case '7':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (object_is_melee_weapon(o_ptr))
            {
                if (_smith_add_essence(o_ptr, ESSENCE_TYPE_SLAYS) == _UNWIND)
                    done = TRUE;
            }
            else if (object_is_armour(o_ptr))
            {
                if (_smith_add_slaying(o_ptr) == _UNWIND)
                    done = TRUE;
            }
            break;
        case '8':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (object_is_melee_weapon(o_ptr))
            {
                if (_smith_add_essence(o_ptr, ESSENCE_TYPE_BRANDS) == _UNWIND)
                    done = TRUE;
            }
            break;
        }
    }
}
static void _smith_ammo(object_type *o_ptr)
{
    bool   done = FALSE;
    bool   can_enchant = _can_enchant(o_ptr);
    rect_t r = ui_map_rect();

    while (!done)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        doc_printf(_doc, "   <color:%c>E</color>) Enchant this object\n", can_enchant ? 'y' : 'D');

        doc_newline(_doc);
        doc_insert(_doc, "   <color:y>A</color>) Absorb all essences\n");
        if (object_is_smith(o_ptr))
            doc_insert(_doc, "   <color:y>R</color>) Remove added essence\n");
        else if (object_is_artifact(o_ptr))
        {
        }
        else
        {
            doc_insert(_doc, "\n   <color:y>1</color>) Add Slay\n");
            doc_insert(_doc, "   <color:y>2</color>) Add Brand\n");
        }

        doc_newline(_doc);
        doc_insert(_doc, "   <color:y>L</color>) List current essences\n");
        doc_insert(_doc, "   <color:y>Q</color>) Quit work on this object\n");
        doc_newline(_doc);
        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = inkey_special(TRUE);
        switch (cmd)
        {
        case 'L': case 'l':
            _list_current_essences();
            break;
        case ESCAPE:
        case 'Q': case 'q':
            done = TRUE;
            break;
        case 'A': case 'a':
            if (_smith_absorb(o_ptr) == _UNWIND)
                done = TRUE;
            break;
        case 'R': case 'r':
            if (object_is_smith(o_ptr) && _smith_remove(o_ptr) == _UNWIND)
                done = TRUE;
            break;
        case 'E': case 'e':
            if (can_enchant && _smith_enchant(o_ptr) == _UNWIND)
                done = TRUE;
            break;
        case '1':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_essence(o_ptr, ESSENCE_TYPE_SLAYS) == _UNWIND)
                done = TRUE;
            break;
        case '2':
            if (object_is_smith(o_ptr) || object_is_artifact(o_ptr)) break;
            if (_smith_add_essence(o_ptr, ESSENCE_TYPE_BRANDS) == _UNWIND)
                done = TRUE;
            break;
        }
    }
}
static void _smith_object(object_type *o_ptr)
{
    assert(!_doc);
    _doc = doc_alloc(72);
    msg_line_clear();
    Term_save();

    if (object_is_ammo(o_ptr))
        _smith_ammo(o_ptr);
    else
        _smith_weapon_armor(o_ptr);

    Term_load();
    doc_free(_doc);
    _doc = NULL;
}

/* entrypoint for smithing: pick an object and enter the toplevel menu */
static bool _smithing(void)
{
    int          item;
    object_type *o_ptr;
    object_type  old_obj;

    item_tester_hook = object_is_weapon_armour_ammo;
    item_tester_no_ryoute = TRUE;

    if (!get_item(&item, "Smith which object? ", "You have nothing to work with.", (USE_INVEN | USE_FLOOR)))
        return FALSE;
    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];

    /* Smithing now automatically 'Judges' the object for free */
    identify_item(o_ptr);
    if (p_ptr->lev >= 30)
    {
        o_ptr->ident |= IDENT_FULL;
        ego_aware(o_ptr);
    }

    old_obj = *o_ptr;

    _smith_object(o_ptr);

    if (item >= 0)
        p_ptr->total_weight += (o_ptr->weight*o_ptr->number - old_obj.weight*old_obj.number);

    p_ptr->notice |= PN_COMBINE | PN_REORDER;
    p_ptr->window |= PW_INVEN;

    return TRUE;
}

/**********************************************************************
 * Powers
 **********************************************************************/
void _smithing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Smithing");
        break;
    case SPELL_DESC:
        var_set_string(res, "Work on a selected object, either extracting, removing, or adding essences.");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->blind)
        {
            msg_print("Better not work the forge while blind!");
            return;
        }
        if (p_ptr->image)
        {
            msg_print("Better not work the forge while hallucinating!");
            return;
        }
        var_set_bool(res, _smithing());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _powers[] =
{
    { A_INT, { 1,  0,  0, _smithing_spell} },
    { -1, { -1, -1, -1, NULL} }
};

static int _get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _powers);
}

/**********************************************************************
 * Character Dump
 **********************************************************************/
static void _dump_ability_flag(doc_ptr doc, int which, int cost, cptr name)
{
    int n = _get_essence(which);
    if (n > 0)
    {
        doc_printf(doc, "   %-15.15s %5d <color:%c>%5d</color>\n",
            name,
            n,
            (n < cost) ? 'D' : 'w',
            cost
        );
    }
}
static void _dump_slay_flag(doc_ptr doc, int which, int cost, cptr name)
{
    _dump_ability_flag(doc, which, cost, name);
}
static void _dump_slay_flag_ammo(doc_ptr doc, int which, int cost, int ammo_div, cptr name)
{
    int n = _get_essence(which);
    assert(ammo_div > 0);
    if (n > 0)
    {
        int ammo_cost = (cost + ammo_div - 1)/ammo_div;

        doc_printf(doc, "   %-15.15s %5d <color:%c>%5d</color> <color:%c>%5d</color>\n",
            name,
            n,
            (n < cost) ? 'D' : 'w',
            cost,
            (n < ammo_cost) ? 'D' : 'w',
            ammo_cost
        );
    }
}
static void _dump_pval_flag(doc_ptr doc, int which, int cost, int max, cptr name)
{
    int i;
    int n = _get_essence(which);
    if (n > 0)
    {
        doc_printf(doc, "   %-15.15s %5d", name, n);
        for (i = 1; i <= max; i++)
        {
            int c = _calc_pval_cost(i, cost);
            doc_printf(doc, " <color:%c>%5d</color>",
                (n < c) ? 'D' : 'w', c);
        }
        doc_newline(doc);
    }
}

static void _dump_pval_table(doc_ptr doc, int type)
{
    int i;
    _essence_group_ptr group_ptr = &_essence_groups[type];

    doc_printf(doc, "   <color:G>%-15.15s Total    +1    +2    +3    +4    +5</color><color:D>    +6    +7</color>\n", group_ptr->name);
    for (i = 0; i < _MAX_INFO_PER_TYPE; i++)
    {
        _essence_info_ptr info_ptr = &group_ptr->entries[i];
        int               max = 7;
        if (info_ptr->id == _ESSENCE_NONE) break;
        if (info_ptr->info) max = info_ptr->info;
        _dump_pval_flag(doc, info_ptr->id, info_ptr->cost, max, info_ptr->name);
    }
}

static void _dump_ability_table(doc_ptr doc, int type)
{
    int i;
    _essence_group_ptr group_ptr = &_essence_groups[type];
    doc_printf(doc, "   <color:G>%-15.15s Total  Cost</color>\n", group_ptr->name);
    for (i = 0; i < _MAX_INFO_PER_TYPE; i++)
    {
        _essence_info_ptr info_ptr = &group_ptr->entries[i];
        if (info_ptr->id == _ESSENCE_NONE) break;
        if (info_ptr->id == _ESSENCE_SPECIAL) continue; /* TODO */
        _dump_ability_flag(doc, info_ptr->id, info_ptr->cost, info_ptr->name);
    }
}

static void _dump_slay_table(doc_ptr doc, int type)
{
    int i;
    _essence_group_ptr group_ptr = &_essence_groups[type];
    doc_printf(doc, "   <color:G>%-15.15s Total  Cost  </color><color:R>Ammo</color></color>\n", group_ptr->name);
    for (i = 0; i < _MAX_INFO_PER_TYPE; i++)
    {
        _essence_info_ptr info_ptr = &group_ptr->entries[i];
        if (info_ptr->id == _ESSENCE_NONE) break;
        if (info_ptr->info)
            _dump_slay_flag_ammo(doc, info_ptr->id, info_ptr->cost, info_ptr->info, info_ptr->name);
        else
            _dump_slay_flag(doc, info_ptr->id, info_ptr->cost, info_ptr->name);
    }
}

static void _character_dump_aux(doc_ptr doc)
{
    int i;
    {
        int n_h = _get_essence(_ESSENCE_TO_HIT);
        int n_d = _get_essence(_ESSENCE_TO_DAM);
        int n_a = _get_essence(_ESSENCE_AC);
        int n_ha = _get_essence(_ESSENCE_TO_HIT_A);
        int n_da = _get_essence(_ESSENCE_TO_DAM_A);
        int max = _enchant_limit();

        doc_printf(doc, "   <color:G>%-12.12s    Weapons    </color><color:R>Missiles</color>         <color:B>Armor</color>\n", "");
        doc_printf(doc, "   <color:G>%-12.12s   Hit   Dam   </color><color:R>Hit   Dam</color>   <color:B>Hit   Dam    AC</color>\n", "Enchantments");
        doc_printf(doc, "   %12.12s %5d %5d %5d %5d %5d %5d %5d\n", "Total", n_h, n_d, n_h, n_d, n_ha, n_da, n_a);
        for (i = 1; i <= max; i++)
        {
            int c_h = _calc_enchant_cost(i, _COST_TO_HIT);
            int c_d = _calc_enchant_cost(i, _COST_TO_DAM);
            int c_a = _calc_enchant_cost(i, _COST_TO_AC);
            int c_hm = (c_h + 9) / 10;
            int c_dm = (c_d + 9) / 10;
            int c_ha = _calc_enchant_cost(i, _COST_TO_HIT_A);
            int c_da = _calc_enchant_cost(i, _COST_TO_DAM_A);

            doc_printf(doc, "   %-9.9s+%2d <color:%c>%5d</color> <color:%c>%5d</color>",
              "", i,
              (n_h < c_h) ? 'D' : 'w', c_h,
              (n_d < c_d) ? 'D' : 'w', c_d
            );
            doc_printf(doc, " <color:%c>%5d</color> <color:%c>%5d</color>",
              (n_h < c_hm) ? 'D' : 'w', c_hm,
              (n_d < c_dm) ? 'D' : 'w', c_dm
            );
            doc_printf(doc, " <color:%c>%5d</color> <color:%c>%5d</color> <color:%c>%5d</color>",
              (n_ha < c_ha) ? 'D' : 'w', c_ha,
              (n_da < c_da) ? 'D' : 'w', c_da,
              (n_a < c_a) ? 'D' : 'w', c_a
            );
            doc_newline(doc);
        }
        if (_ART_ENCH_MULT > 1)
            doc_printf(doc, "   <color:G>Note</color>: Artifacts cost <color:r>%dx</color> to enchant.\n", _ART_ENCH_MULT);
        doc_newline(doc);
    }

    _dump_pval_table(doc, ESSENCE_TYPE_STATS);
    doc_newline(doc);
    _dump_pval_table(doc, ESSENCE_TYPE_BONUSES);
    doc_newline(doc);

    {
        doc_ptr cols[2];

        cols[0] = doc_alloc(40);
        cols[1] = doc_alloc(40);

        _dump_slay_table(cols[0], ESSENCE_TYPE_SLAYS);
        doc_newline(cols[0]);
        _dump_slay_table(cols[0], ESSENCE_TYPE_BRANDS);

        _dump_ability_table(cols[1], ESSENCE_TYPE_RESISTS);
        doc_newline(cols[1]);
        _dump_ability_table(cols[1], ESSENCE_TYPE_SUSTAINS);

        doc_insert_cols(doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);
    }

    {
        doc_ptr cols[2];

        cols[0] = doc_alloc(40);
        cols[1] = doc_alloc(40);

        _dump_ability_table(cols[0], ESSENCE_TYPE_ABILITIES);
        _dump_ability_table(cols[1], ESSENCE_TYPE_TELEPATHY);

        doc_insert_cols(doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);
    }
}

static void _character_dump(doc_ptr doc)
{
    if (_count_essences())
    {
        doc_printf(doc, "<topic:Essences>=================================== <color:keypress>E</color>ssences ==================================\n\n");
        _character_dump_aux(doc);
    }
}

static void _birth(void)
{
    _clear_essences();
}

/**********************************************************************
 * Public
 **********************************************************************/
class_t *weaponsmith_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  28,  28,   1,  20,  10,  60,  45};
    skills_t xs = { 10,  10,  10,   0,   0,   0,  21,  15};

        me.name = "Weaponsmith";
        me.desc = "A Weaponsmith can improve weapons and armors for him or herself. "
                    "They are good at fighting, and they have potential ability to "
                    "become even better than Warriors using improved equipment. They "
                    "cannot cast spells, and are poor at skills such as stealth or "
                    "magic defense.\n \n"
                    "A Weaponsmith extracts the essences of special effects from weapons "
                    "or armors which have various special abilities, and can add these "
                    "essences to another weapon or armor. Normally, each equipment can "
                    "be improved only once, but they can remove a previously added "
                    "essence from improved equipment to improve it with another "
                    "essence. To-hit, to-damage bonus, and AC can be improved freely "
                    "up to a maximum value depending on level. Weaponsmiths now use class "
                    "powers for Smithing commands.";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 111;
        me.base_hp = 12;
        me.exp = 130;
        me.pets = 40;
        
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;

        me.birth = _birth;
        me.load_player = _load;
        me.save_player = _save;
        me.destroy_object = _on_destroy_object;

        init = TRUE;
    }

    return &me;
}

void weaponsmith_object_flags(object_type *o_ptr, u32b flgs[TR_FLAG_SIZE])
{
    if (object_is_smith(o_ptr))
    {
        int add = o_ptr->xtra3 - 1;

        if (add < TR_FLAG_COUNT)
            add_flag(flgs, add);

        else if (add == _ESSENCE_SPECIAL)
        {
            switch (o_ptr->xtra1)
            {
            case _ESSENCE_RES_BASE:
                add_flag(flgs, TR_RES_ACID);
                add_flag(flgs, TR_RES_ELEC);
                add_flag(flgs, TR_RES_FIRE);
                add_flag(flgs, TR_RES_COLD);
                break;
            case _ESSENCE_SUST_ALL:
                add_flag(flgs, TR_SUST_STR);
                add_flag(flgs, TR_SUST_INT);
                add_flag(flgs, TR_SUST_WIS);
                add_flag(flgs, TR_SUST_DEX);
                add_flag(flgs, TR_SUST_CON);
                add_flag(flgs, TR_SUST_CHR);
                break;
            case _ESSENCE_SLAYING:
                add_flag(flgs, TR_SHOW_MODS);
                break;
            }
        }
    }
}
