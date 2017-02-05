#include "angband.h"

#include <assert.h>

/************************************************************************
 * Data Types
 ***********************************************************************/
#define _MAX_STOCK  24
#define _MAX_OWNERS 32

struct _owner_s
{
    int  id;
    cptr name;
    int  purse;
    int  max_inflate;
    int  min_inflate;
    int  race_id;
};
typedef struct _owner_s _owner_t, *_owner_ptr;

typedef bool (*_k_idx_p)(int k_idx);
typedef bool (*_create_obj_f)(obj_ptr obj);
struct _type_s
{
    int           id;
    cptr          name;
    obj_p         purchase_p;
    _create_obj_f create_f;
    _owner_t      owners[_MAX_OWNERS];
};
typedef struct _type_s _type_t, *_type_ptr;

struct _last_visit_s
{
    int turn;
    int level;
    int exp;
};
typedef struct _last_visit_s _last_visit_t;

struct shop_s
{
    _type_ptr     type;
    _owner_ptr    owner;
    inv_ptr       inv;
    _last_visit_t last_visit;
};

/************************************************************************
 * Shop Types and Their Owners (originally from CthAngband)
 ***********************************************************************/

static bool _general_will_buy(obj_ptr obj);
static bool _general_create(obj_ptr obj);

static _type_t _types[] = 
{
    { SHOP_GENERAL, "General Store", _general_will_buy, _general_create,
        {{  1, "Bilbo the Friendly",         200,    170, 108, RACE_HOBBIT },
         {  2, "Rincewind the Chicken",      200,    175, 108, RACE_HUMAN },
         {  3, "Sultan the Midget",          300,    170, 107, RACE_GNOME },
         {  4, "Lyar-el the Comely",         300,    165, 107, RACE_DEMIGOD },
         {  5, "Falilmawen the Friendly",    250,    170, 108, RACE_HOBBIT },
         {  6, "Voirin the Cowardly",        500,    175, 108, RACE_HUMAN },
         {  7, "Erashnak the Midget",        750,    170, 107, RACE_BEASTMAN },
         {  8, "Grug the Comely",           1000,    165, 107, RACE_HALF_TITAN },
         {  9, "Forovir the Cheap",          250,    170, 108, RACE_HUMAN },
         { 10, "Ellis the Fool",             500,    175, 108, RACE_HUMAN },
         { 11, "Filbert the Hungry",         750,    170, 107, RACE_VAMPIRE },
         { 12, "Fthnargl Psathiggua",       1000,    165, 107, RACE_MIND_FLAYER },
         { 13, "Eloise Long-Dead",           250,    170, 108, RACE_SPECTRE },
         { 14, "Fundi the Slow",             500,    175, 108, RACE_ZOMBIE },
         { 15, "Granthus",                   750,    170, 107, RACE_SKELETON },
         { 16, "Lorax the Suave",           1000,    165, 107, RACE_VAMPIRE },
         { 17, "Butch",                      250,    170, 108, RACE_SNOTLING },
         { 18, "Elbereth the Beautiful",     500,    175, 108, RACE_HIGH_ELF },
         { 19, "Sarleth the Sneaky",         750,    170, 107, RACE_GNOME },
         { 20, "Narlock",                   1000,    165, 107, RACE_DWARF },
         { 21, "Haneka the Small",           250,    170, 108, RACE_GNOME },
         { 22, "Loirin the Mad",             500,    175, 108, RACE_HALF_GIANT },
         { 23, "Wuto Poisonbreath",          750,    170, 107, RACE_DRACONIAN },
         { 24, "Araaka the Rotund",         1000,    165, 107, RACE_DRACONIAN },
         { 25, "Poogor the Dumb",            250,    170, 108, RACE_BEASTMAN },
         { 26, "Felorfiliand",               500,    175, 108, RACE_DEMIGOD },
         { 27, "Maroka the Aged",            750,    170, 107, RACE_GNOME },
         { 28, "Sasin the Bold",            1000,    165, 107, RACE_HALF_GIANT },
         { 29, "Abiemar the Peasant",        250,    170, 108, RACE_HUMAN },
         { 30, "Hurk the Poor",              500,    175, 108, RACE_SNOTLING },
         { 31, "Soalin the Wretched",        750,    170, 107, RACE_ZOMBIE },
         { 32, "Merulla the Humble",        1000,    165, 107, RACE_DEMIGOD }}},
        
    { SHOP_NONE }
};

static _type_ptr _get_type(int which)
{
    int i;
    for (i = 0;; i++)
    {
        _type_ptr type = &_types[i];
        if (type->id == SHOP_NONE) return NULL;
        if (type->id == which) return type;
    }
}

static _owner_ptr _get_owner(_type_ptr type, int which)
{
    int i;
    for (i = 0; i < _MAX_OWNERS; i++)
    {
        _owner_ptr owner = &type->owners[i];
        if (!owner->name) break;
        if (owner->id == which) return owner;
    }
    return NULL;
}

static int _count_owners(_type_ptr type)
{
    int ct = 0;
    int i;
    for (i = 0; i < _MAX_OWNERS; i++)
    {
        if (!type->owners[i].name) break;
        ct++;
    }
    return ct;
}

static bool _will_buy(obj_ptr obj)
{
    if (obj_value(obj) <= 0) return FALSE;
    return TRUE;
}

static bool _stock_p(int k_idx)
{
    if (k_info[k_idx].gen_flags & OFG_INSTA_ART)
        return FALSE;

    if (p_ptr->town_num != TOWN_ZUL && p_ptr->town_num != TOWN_DUNGEON)
    {
        if (!(k_info[k_idx].gen_flags & OFG_TOWN))
            return FALSE;
    }
    return TRUE;
}

static int _get_k_idx(_k_idx_p p, int lvl)
{
    int k_idx;
    if (p)
    {
        get_obj_num_hook = p;
        get_obj_num_prep();
    }
    k_idx = get_obj_num(lvl);
    if (p)
    {
        get_obj_num_hook = NULL;
        get_obj_num_prep();
    }
    return k_idx;
}

static int _mod_lvl(int lvl)
{
    return dun_level/3 + lvl;
}

static void _discount(obj_ptr obj)
{
    int discount = 0;
    int cost = obj_value(obj);

    if (cost < 5)          discount = 0;
    else if (one_in_(25))  discount = 25;
    else if (one_in_(150)) discount = 50;
    else if (one_in_(300)) discount = 75;
    else if (one_in_(500)) discount = 90;

    if (object_is_artifact(obj))
        discount = 0;

    obj->discount = discount;
}

static bool _create(obj_ptr obj, int k_idx, int lvl)
{
    int mode = AM_NO_FIXED_ART;

    /* TODO:if (p_ptr->town_num != SECRET_TOWN && cur_store_num != STORE_BLACK)
    {
        mode |=  AM_STOCK_TOWN;
    }

    if (cur_store_num == STORE_BLACK)
        mode |= AM_STOCK_BM;*/

    if (!k_idx) return FALSE;

    object_prep(obj, k_idx);
    apply_magic(obj, lvl, mode);
    if (obj->tval == TV_LITE)
    {
        if (obj->sval == SV_LITE_TORCH) obj->xtra4 = FUEL_TORCH / 2;
        if (obj->sval == SV_LITE_LANTERN) obj->xtra4 = FUEL_LAMP / 2;
    }

    if (object_is_cursed(obj)) return FALSE;

    obj->ident |= IDENT_STORE;
    _discount(obj);
    mass_produce(obj);
    return TRUE;
}

/************************************************************************
 * The General Store
 ***********************************************************************/
static bool _general_will_buy(obj_ptr obj)
{
    switch (obj->tval)
    {
    case TV_POTION:
        if (obj->sval != SV_POTION_WATER) return FALSE;
    case TV_WHISTLE:
    case TV_FOOD:
    case TV_LITE:
    case TV_FLASK:
    case TV_SPIKE:
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_DIGGING:
    case TV_CLOAK:
    case TV_BOTTLE: /* 'Green', recycling Angband */
    case TV_FIGURINE:
    case TV_STATUE:
    case TV_CAPTURE:
    case TV_CARD:
        break;
    default:
        return FALSE;
    }
    return _will_buy(obj);
}

static bool _general_stock_p(int k_idx)
{
    if (!_stock_p(k_idx))
        return FALSE;

    switch (k_info[k_idx].tval)
    {
    case TV_FLASK:
    case TV_SPIKE:
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_CAPTURE:
    case TV_FIGURINE:
    case TV_CLOAK:
    case TV_LITE:
    case TV_FOOD:
    case TV_DIGGING:
        return TRUE;
    }
    return FALSE;
}

static bool _general_create(obj_ptr obj)
{
    int k_idx;
    if (one_in_(50))
        k_idx = lookup_kind(TV_CAPTURE, 0);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_FOOD, SV_FOOD_RATION);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_POTION, SV_POTION_WATER);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_FLASK, SV_FLASK_OIL);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_LITE, SV_LITE_LANTERN);
    else if (one_in_(3))
        k_idx = lookup_kind(TV_DIGGING, SV_SHOVEL);
    else if (one_in_(5))
        k_idx = lookup_kind(TV_DIGGING, SV_PICK);
    else
        k_idx = _get_k_idx(_general_stock_p, _mod_lvl(20));
    return _create(obj, k_idx, _mod_lvl(rand_range(1, 5)));
}

/************************************************************************
 * Shops
 ***********************************************************************/
static void _change_owner(shop_ptr shop)
{
    int ct = _count_owners(shop->type);

    for (;;)
    {
        int        idx = randint0(ct);
        _owner_ptr owner = &shop->type->owners[idx];

        if (owner != shop->owner)
        {
            shop->owner = owner;
            break;
        }
    }
}

shop_ptr shop_alloc(int which)
{
    shop_ptr shop = malloc(sizeof(shop_t));
    shop->type = _get_type(which);
    shop->owner = NULL;
    shop->inv = inv_alloc(shop->type->name, INV_SHOP, 0);
    _change_owner(shop);
    shop_reset(shop);
    return shop;
}

shop_ptr shop_load(savefile_ptr file)
{
    shop_ptr shop = malloc(sizeof(shop_t));
    int      tmp;

    tmp = savefile_read_s16b(file);
    shop->type = _get_type(tmp);
    assert(shop->type);

    tmp = savefile_read_s16b(file);
    shop->owner = _get_owner(shop->type, tmp);
    assert(shop->owner);

    shop->inv = inv_alloc(shop->type->name, INV_SHOP, 0);
    inv_load(shop->inv, file);

    shop->last_visit.turn = savefile_read_s32b(file);
    shop->last_visit.level = savefile_read_s16b(file);
    shop->last_visit.exp = savefile_read_s32b(file);

    assert(savefile_read_u32b(file) == 0xFEEDFEED);

    return shop;
}

void shop_free(shop_ptr shop)
{
    if (shop)
    {
        inv_free(shop->inv);
        shop->inv = NULL;
        shop->type = NULL;
        shop->owner = NULL;
        free(shop);
    }
}

void shop_reset(shop_ptr shop)
{
    inv_clear(shop->inv);
    shop->last_visit.turn = 0;
    shop->last_visit.level = 0;
    shop->last_visit.exp = 0;
}

void shop_save(shop_ptr shop, savefile_ptr file)
{
    savefile_write_s16b(file, shop->type->id);
    savefile_write_s16b(file, shop->owner->id);
    inv_save(shop->inv, file);
    savefile_write_s32b(file, shop->last_visit.turn);
    savefile_write_s16b(file, shop->last_visit.level);
    savefile_write_s32b(file, shop->last_visit.exp);
    savefile_write_u32b(file, 0xFEEDFEED);
}

/************************************************************************
 * User Interface
 *
 * Note: All functions take the point of view of the *shop*, not 
 *       the player. So _buy is the shop buying or the player
 *       selling. This is appropriate for a shop module!
 ***********************************************************************/
struct _ui_context_s
{
    shop_ptr shop;
    slot_t   top;
    int      page_size;
    doc_ptr  doc;
};
typedef struct _ui_context_s _ui_context_t, *_ui_context_ptr;

static void _display(_ui_context_ptr context);
static void _buy(_ui_context_ptr context);
static void _examine(_ui_context_ptr context);
static void _sell(_ui_context_ptr context);
static void _loop(_ui_context_ptr context);

static void _maintain(shop_ptr shop);
static int  _cull(shop_ptr shop, int target);
static int  _restock(shop_ptr shop, int target);

void shop_ui(shop_ptr shop)
{
    _ui_context_t context = {0};

    store_hack = TRUE;
    _maintain(shop);

    context.shop = shop;
    context.top = 1;
    _loop(&context);
    store_hack = FALSE;

    shop->last_visit.turn = game_turn;
}

static void _loop(_ui_context_ptr context)
{
    forget_lite(); /* resizing the term would redraw the map ... sigh */
    forget_view();
    character_icky = TRUE;

    msg_line_clear();
    msg_line_init(ui_shop_msg_rect());

    Term_clear();
    context->doc = doc_alloc(MIN(80, ui_shop_rect().cx));
    for (;;)
    {
        int    max = inv_last(context->shop->inv, obj_exists);
        rect_t r = ui_shop_rect(); /* recalculate in case resize */
        int    cmd;

        context->page_size = MIN(26, r.cy - 3 - 2);
        _display(context);

        cmd = inkey_special(TRUE);
        msg_line_clear();
        msg_boundary(); /* turn_count is unchanging while in home/museum */
        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q') break;
        pack_lock();
        if (!shop_common_cmd_handler(cmd))
        {
            switch (cmd) /* cmd is from the player's perspective */
            {
            case 'g': case 'b': _sell(context); break;
            case 'd': case 's': _buy(context); break;
            case 'x': _examine(context); break;
            case '?':
                doc_display_help("context_shop.txt", NULL);
                Term_clear_rect(ui_shop_msg_rect());
                break;
            case SKEY_PGDOWN: case '3':
                if (context->top + context->page_size < max)
                    context->top += context->page_size;
                break;
            case SKEY_PGUP: case '9':
                if (context->top > context->page_size)
                    context->top -= context->page_size;
                break;
            default:
                if (cmd < 256 && isprint(cmd))
                {
                    msg_format("Unrecognized command: <color:R>%c</color>. "
                               "Press <color:keypress>?</color> for help.", cmd);
                }
                else if (KTRL('A') <= cmd && cmd <= KTRL('Z'))
                {
                    cmd |= 0x40;
                    msg_format("Unrecognized command: <color:R>^%c</color>. "
                               "Press <color:keypress>?</color> for help.", cmd);
                }
            }
        }
        pack_unlock();
        notice_stuff(); /* PW_INVEN and PW_PACK ... */
        handle_stuff(); /* Plus 'C' to view character sheet */
    }
    character_icky = FALSE;
    energy_use = 100;
    msg_line_clear();
    msg_line_init(ui_msg_rect());

    Term_clear();
    do_cmd_redraw();

    doc_free(context->doc);
}

void _display(_ui_context_ptr context)
{
    rect_t r = ui_shop_rect();

    doc_clear(context->doc);
    doc_insert(context->doc, "<style:table>");
    doc_printf(context->doc, "%*s<color:G>%s</color>\n\n",
        (r.cx - 10)/2, "", context->shop->type->name);

    shop_display_inv(context->doc, context->shop->inv, context->top, context->page_size);
    
    doc_insert(context->doc,
        "<color:keypress>x</color> to begin examining items.\n"
        "<color:keypress>Esc</color> to exit. "
        "<color:keypress>PageUp/Down</color> to scroll. "
        "<color:keypress>?</color> for help.");
    doc_insert(context->doc, "</style>");

    Term_clear_rect(r);
    doc_sync_term(context->doc,
        doc_range_top_lines(context->doc, r.cy),
        doc_pos_create(r.x, r.y));
}

void _buy(_ui_context_ptr context)
{
}

void _examine(_ui_context_ptr context)
{
}

void _sell(_ui_context_ptr context)
{
}

/************************************************************************
 * Stocking
 ***********************************************************************/
void _maintain(shop_ptr shop)
{
    int  num;
    int  i;
    bool allow_restock = TRUE;

    /* Initialize an empty shop */
    if (!inv_count_slots(shop->inv, obj_exists))
    {
        _restock(shop, 12);
        return;
    }

    /* Shops maintain once per day */
    num = MIN(10, (game_turn - shop->last_visit.turn) / TOWN_DAWN);
    if (!num) return;

    /* Limit shop scumming (ie resting in town or on DL1 for BM wares) */
    if (shop->type->id == SHOP_BLACK_MARKET || shop->type->id == SHOP_JEWELER)
    {
        if (shop->last_visit.turn)
        {
            int xp = shop->last_visit.exp;
            xp += MIN(MAX(xp / 20, 1000), 100000);
            if ( !ironman_downward
              && p_ptr->max_plv <= shop->last_visit.level
              && p_ptr->max_exp <= xp
              && p_ptr->prace != RACE_ANDROID )
            {
                allow_restock = FALSE;
            }
        }
    }

    /* Maintain the shop for each day since last visit */
    for (i = 0; i < num; i++)
    {
        int ct = inv_count_slots(shop->inv, obj_exists);
        if (ct < 6) _restock(shop, 12);
        else if (ct > 24) _cull(shop, 12);
        else
        {
            ct = _cull(shop, MAX(6, ct - randint1(9)));
            if (allow_restock)
                ct = _restock(shop, MIN(24, ct + randint1(9)));
        }
    }
}

int _cull(shop_ptr shop, int target)
{
    int ct = inv_count_slots(shop->inv, obj_exists);
    int attempt;

    assert(ct > 0 && target > 0);
    assert(ct >= target);
    for (attempt = 1; ct > target && attempt < 100; attempt++)
    {
        slot_t  slot = inv_random_slot(shop->inv, obj_exists);
        obj_ptr obj = inv_obj(shop->inv, slot);

        assert(obj);
        assert(obj->number > 0);

        if (obj->marked & OM_RESERVED) continue;

        if (one_in_(2))
            obj->number = (obj->number + 1)/2;
        else if (one_in_(2))
            obj->number--;
        else
            obj->number = 0;

        if (!obj->number)
        {
            inv_remove(shop->inv, slot);
            ct--;
        }
    }
    inv_sort(shop->inv);
    assert(ct == inv_count_slots(shop->inv, obj_exists));
    return ct;
}

int _add_obj(shop_ptr shop, obj_ptr obj) /* return number of new slots used (0 or 1) */
{
    slot_t slot, max = inv_last(shop->inv, obj_exists);
    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr dest = inv_obj(shop->inv, slot);
        if (!dest) continue;
        if (obj_can_combine(dest, obj, INV_SHOP))
        {
            obj_combine(dest, obj, INV_SHOP);
            return 0;
        }
    }
    inv_add(shop->inv, obj);
    return 1;
}

int _restock(shop_ptr shop, int target)
{
    int ct = inv_count_slots(shop->inv, obj_exists);
    int attempt = 0;

    assert(ct <= target);
    for (attempt = 1; ct < target && attempt < 100; attempt++)
    {
        obj_t forge = {0};
        if (shop->type->create_f(&forge))
            ct += _add_obj(shop, &forge);
    }
    inv_sort(shop->inv);
    assert(ct == inv_count_slots(shop->inv, obj_exists));
    shop->last_visit.level = p_ptr->max_plv;
    shop->last_visit.exp = p_ptr->max_exp;
    return ct;
}

/************************************************************************
 * User Interface Helpers
 ***********************************************************************/
bool shop_common_cmd_handler(int cmd)
{
    switch (cmd)
    {
    case ' ':
    case '\r':
        return TRUE;
    case 'w':
        equip_wield_ui();
        return TRUE;
    case 't': case 'T':
        equip_takeoff_ui();
        return TRUE;
    case 'k': case KTRL('D'):
        obj_destroy_ui();
        return TRUE;
    case 'e':
        equip_ui();
        return TRUE;
    case 'i':
        pack_ui();
        return TRUE;
    case 'I':
        obj_inspect_ui();
        return TRUE;
    case KTRL('I'):
        toggle_inven_equip();
        return TRUE;
    case '{':
        obj_inscribe_ui();
        return TRUE;
    case '}':
        obj_uninscribe_ui();
        return TRUE;
    case '/':
        do_cmd_query_symbol();
        return TRUE;
    case 'C':
        py_display();
        return TRUE;
    case KTRL('W'):
        show_weights = !show_weights;
        return TRUE;
    case KTRL('G'):
        show_item_graph = !show_item_graph;
        return TRUE;
    }
    return FALSE;
}

void shop_display_inv(doc_ptr doc, inv_ptr inv, slot_t top, int page_size)
{
    slot_t slot;
    int    xtra = 0;
    char   name[MAX_NLEN];

    if (show_weights)
        xtra = 9;  /* " 123.0 lbs" */

    doc_insert(doc, "    Item Description");
    if (show_weights)
        doc_printf(doc, "<tab:%d>   Weight", doc_width(doc) - xtra);
    doc_newline(doc);

    for (slot = top; slot < top + page_size; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (obj)
        {
            obj_ptr     next = inv_obj(inv, slot + 1);
            doc_style_t style = *doc_current_style(doc);
            char        label_color = 'w';

            if (next && obj_cmp(obj, next) > 0)
                label_color = 'r';

            object_desc(name, obj, OD_COLOR_CODED);

            doc_printf(doc, " <color:%c>%c</color>) ", label_color, slot_label(slot - top + 1));
            if (show_item_graph)
            {
                doc_insert_char(doc, object_attr(obj), object_char(obj));
                doc_insert(doc, " ");
            }
            if (xtra)
            {
                style.right = doc_width(doc) - xtra;
                doc_push_style(doc, &style);
            }
            doc_printf(doc, "%s", name);
            if (xtra) doc_pop_style(doc);

            if (show_weights)
            {
                int wgt = obj->weight * obj->number;
                doc_printf(doc, "<tab:%d> %3d.%d lbs", doc_width(doc) - xtra, wgt/10, wgt%10);
            }
            doc_newline(doc);
        }
        else
            doc_newline(doc);
    }
}

/************************************************************************
 * Town
 ***********************************************************************/
static cptr _names[] = { "", "Outpost", "Telmora", "Morivant", "Angwil", "Zul", "Dungeon" };

struct town_s
{
   int         id;
   cptr        name;
   int_map_ptr shops;
};

static town_ptr _town_alloc(int which, cptr name)
{
    town_ptr town = malloc(sizeof(town_t));
    town->id = which;
    town->name = name;
    town->shops = int_map_alloc((int_map_free_f)shop_free);
    return town;
}

static town_ptr _town_load(savefile_ptr file)
{
    town_ptr town = malloc(sizeof(town_t));
    int      ct, i;

    town->id = savefile_read_s16b(file);
    assert(0 < town->id && town->id <= TOWN_DUNGEON);
    town->name = _names[town->id];
    town->shops = int_map_alloc((int_map_free_f)shop_free);

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        shop_ptr shop = shop_load(file);
        int_map_add(town->shops, shop->type->id, shop);
    }
    return town;
}

static void _town_free(town_ptr town)
{
    if (town)
    {
        int_map_free(town->shops);
        town->shops = NULL;
        free(town);
    }
}

static void _town_save(town_ptr town, savefile_ptr file)
{
    int_map_iter_ptr iter;

    savefile_write_s16b(file, town->id);
    savefile_write_s16b(file, int_map_count(town->shops));

    for (iter = int_map_iter_alloc(town->shops);
            int_map_iter_is_valid(iter);
            int_map_iter_next(iter))
    {
        shop_ptr shop = int_map_iter_current(iter);
        shop_save(shop, file);
    }
    int_map_iter_free(iter);
}

shop_ptr town_get_shop(town_ptr town, int which)
{
    shop_ptr shop = int_map_find(town->shops, which);
    if (!shop)
    {
        shop = shop_alloc(which);
        int_map_add(town->shops, which, shop);
    }
    assert(shop);
    return shop;
}

/************************************************************************
 * Towns
 ***********************************************************************/
static int_map_ptr _towns = NULL;

void towns_init(void)
{
    int_map_free(_towns);
    _towns = int_map_alloc((int_map_free_f) _town_free);
}

town_ptr towns_current_town(void)
{
    town_ptr town = NULL;
    if (p_ptr->town_num)
        town = towns_get_town(p_ptr->town_num);
    return town;
}


town_ptr towns_get_town(int which)
{
    town_ptr town = int_map_find(_towns, which);
    if (!town)
    {
        town = _town_alloc(which, _names[which]);
        int_map_add(_towns, which, town);
    }
    assert(town);
    return town;
}

void towns_save(savefile_ptr file)
{
    int_map_iter_ptr iter;

    savefile_write_s16b(file, int_map_count(_towns));

    for (iter = int_map_iter_alloc(_towns);
            int_map_iter_is_valid(iter);
            int_map_iter_next(iter))
    {
        town_ptr town = int_map_iter_current(iter);
        _town_save(town, file);
    }
    int_map_iter_free(iter);
}

void towns_load(savefile_ptr file)
{
    int i, ct;

    int_map_clear(_towns);

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        town_ptr town = _town_load(file);
        int_map_add(_towns, town->id, town);
    }
}

