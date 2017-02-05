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
    int  greed;
    int  race_id;
};
typedef struct _owner_s _owner_t, *_owner_ptr;

typedef bool (*_k_idx_p)(int k_idx);
typedef bool (*_create_obj_f)(obj_ptr obj);
struct _type_s
{
    int           id;
    cptr          name;
    obj_p         buy_p;
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
        {{  1, "Bilbo the Friendly",         200, 108, RACE_HOBBIT },
         {  2, "Rincewind the Chicken",      200, 108, RACE_HUMAN },
         {  3, "Sultan the Midget",          300, 107, RACE_GNOME },
         {  4, "Lyar-el the Comely",         300, 107, RACE_DEMIGOD },
         {  5, "Falilmawen the Friendly",    250, 108, RACE_HOBBIT },
         {  6, "Voirin the Cowardly",        500, 108, RACE_HUMAN },
         {  7, "Erashnak the Midget",        750, 107, RACE_BEASTMAN },
         {  8, "Grug the Comely",           1000, 107, RACE_HALF_TITAN },
         {  9, "Forovir the Cheap",          250, 108, RACE_HUMAN },
         { 10, "Ellis the Fool",             500, 108, RACE_HUMAN },
         { 11, "Filbert the Hungry",         750, 107, RACE_VAMPIRE },
         { 12, "Fthnargl Psathiggua",       1000, 107, RACE_MIND_FLAYER },
         { 13, "Eloise Long-Dead",           250, 108, RACE_SPECTRE },
         { 14, "Fundi the Slow",             500, 108, RACE_ZOMBIE },
         { 15, "Granthus",                   750, 107, RACE_SKELETON },
         { 16, "Lorax the Suave",           1000, 107, RACE_VAMPIRE },
         { 17, "Butch",                      250, 108, RACE_SNOTLING },
         { 18, "Elbereth the Beautiful",     500, 108, RACE_HIGH_ELF },
         { 19, "Sarleth the Sneaky",         750, 107, RACE_GNOME },
         { 20, "Narlock",                   1000, 107, RACE_DWARF },
         { 21, "Haneka the Small",           250, 108, RACE_GNOME },
         { 22, "Loirin the Mad",             500, 108, RACE_HALF_GIANT },
         { 23, "Wuto Poisonbreath",          750, 107, RACE_DRACONIAN },
         { 24, "Araaka the Rotund",         1000, 107, RACE_DRACONIAN },
         { 25, "Poogor the Dumb",            250, 108, RACE_BEASTMAN },
         { 26, "Felorfiliand",               500, 108, RACE_DEMIGOD },
         { 27, "Maroka the Aged",            750, 107, RACE_GNOME },
         { 28, "Sasin the Bold",            1000, 107, RACE_HALF_GIANT },
         { 29, "Abiemar the Peasant",        250, 108, RACE_HUMAN },
         { 30, "Hurk the Poor",              500, 108, RACE_SNOTLING },
         { 31, "Soalin the Wretched",        750, 107, RACE_ZOMBIE },
         { 32, "Merulla the Humble",        1000, 107, RACE_DEMIGOD }}},
        
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
 * Pricing
 * Note: All functions take the point of view of the *shop*, not 
 *       the player. So _buy is the shop buying or the player
 *       selling. This is appropriate for a shop module!
 ***********************************************************************/
static int _price_factor_aux(int greed)
{
    int factor;

    factor = get_race()->shop_adjust;
    if (factor == 0)
        factor = 110;

    factor = (factor * adj_gold[p_ptr->stat_ind[A_CHR]] + 50) / 100;
    factor = (factor * (135 - MIN(200, p_ptr->fame)/4) + 50) / 100;
    factor = (factor * greed + 50) / 100;

    return factor;
}

int _price_factor(shop_ptr shop)
{
    int factor = _price_factor_aux(shop->owner->greed);

    if (prace_is_(shop->owner->race_id))
        factor = factor * 90 / 100;

    return factor;
}

static int _sell_price_aux(int price, int factor)
{
    if (factor < 100)
        factor = 100;

    if (price > 1000*1000)
        price = (price / 100) * factor;
    else
        price = (price * factor + 50) / 100;

    if (price > 1000)
        price = big_num_round(price, 3);

    return price;
}

static int _sell_price(shop_ptr shop, int price)
{
    int factor = _price_factor(shop);

    price = _sell_price_aux(price, factor);
    if (shop->type->id == SHOP_BLACK_MARKET)
    {
        if (p_ptr->realm1 != REALM_BURGLARY && !mut_present(MUT_BLACK_MARKETEER))
            price = price * 2;

        price = price * (625 + virtue_current(VIRTUE_JUSTICE)) / 625;
    }
    else if (shop->type->id == SHOP_JEWELER)
        price = price * 2;

    return price;
}

static int _buy_price_aux(int price, int factor)
{
    if (factor < 105)
        factor = 105;

    if (price > 1000*1000)
        price = (price / factor) * 100;
    else
        price = (price * 100) / factor;

    if (price > 1000)
        price = big_num_round(price, 3);

    return price;
}

static int _buy_price(shop_ptr shop, int price)
{
    int factor = _price_factor(shop);

    price = _buy_price_aux(price, factor);
    if (shop->type->id == SHOP_BLACK_MARKET)
    {
        if (p_ptr->realm1 != REALM_BURGLARY && !mut_present(MUT_BLACK_MARKETEER))
            price = price / 2;

        price = price * (625 - virtue_current(VIRTUE_JUSTICE)) / 625;
    }
    else if (shop->type->id == SHOP_JEWELER)
        price = price / 2;

    if (price > shop->owner->purse)
        price = shop->owner->purse;

    return MAX(1, price);
}

int town_service_price(int price)
{
    int factor = _price_factor_aux(100);
    return _sell_price_aux(price, factor);
}

/************************************************************************
 * User Interface
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
static void _sellout(_ui_context_ptr context);
static void _reserve(_ui_context_ptr context);
static void _loop(_ui_context_ptr context);

static void _maintain(shop_ptr shop);
static int  _cull(shop_ptr shop, int target);
static int  _restock(shop_ptr shop, int target);
static void _shuffle_stock(shop_ptr shop);

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

        context->page_size = MIN(26, r.cy - 4 - 4);
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
            case 'g': case 'b': case 'p': _sell(context); break;
            case 'B': _sellout(context); break;
            case 'd': case 's': _buy(context); break;
            case 'x': _examine(context); break;
            case 'S': _shuffle_stock(context->shop); break;
            case 'R': _reserve(context); break;
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
            if (pack_overflow_count())
            {
                msg_print("<color:v>Your pack is overflowing!</color> It's time for you to leave!");
                break;
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

static void _display_inv(doc_ptr doc, shop_ptr shop, slot_t top, int page_size);
static void _display(_ui_context_ptr context)
{
    rect_t   r = ui_shop_rect();
    doc_ptr  doc = context->doc;
    shop_ptr shop = context->shop;
    int      ct = strlen(shop->type->name) + 10; /* " (20000gp)" */
    char     buf[10];

    doc_clear(doc);
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "    <color:U>%s (%s)</color>",
        shop->owner->name, get_race_aux(shop->owner->race_id, 0)->name);
    doc_printf(doc, "<tab:%d><color:G>%s</color> (<color:r>%dgp</color>)\n\n",
        doc_width(doc) - ct,
        shop->type->name, shop->owner->purse);

    _display_inv(doc, shop, context->top, context->page_size);
    
    big_num_display(p_ptr->au, buf);
    doc_printf(doc, "Gold Remaining: <color:y>%s</color>\n\n", buf);
    doc_insert(doc,
        "<color:keypress>b</color> to buy. "
        "<color:keypress>s</color> to sell. "
        "<color:keypress>x</color> to begin examining items.\n"
        "<color:keypress>B</color> to buyout inventory. ");

    if (mut_present(MUT_MERCHANTS_FRIEND))
    {
        doc_insert(doc,
            "<color:keypress>S</color> to shuffle stock. "
            "<color:keypress>R</color> to reserve an item.");
    }
    doc_newline(doc);

    doc_insert(doc,
        "<color:keypress>Esc</color> to exit. "
        "<color:keypress>PageUp/Down</color> to scroll. "
        "<color:keypress>?</color> for help.");
    doc_insert(doc, "</style>");

    Term_clear_rect(r);
    doc_sync_term(doc,
        doc_range_top_lines(doc, r.cy),
        doc_pos_create(r.x, r.y));
}

static int _add_obj(shop_ptr shop, obj_ptr obj);
static void _buy_aux(shop_ptr shop, obj_ptr obj)
{
    char       name[MAX_NLEN];
    string_ptr s = string_alloc();
    char       c;
    int        price = obj_value(obj);

    if (!price)
    {
        msg_print("I have no interest in your junk!");
        return;
    }
    price = _buy_price(shop, price);
    price *= obj->number;

    object_desc(name, obj, OD_COLOR_CODED);
    string_printf(s, "Really sell %s for <color:R>%d</color> gp? <color:y>[y/n]</color>", name, price);
    c = msg_prompt(string_buffer(s), "ny", PROMPT_DEFAULT);
    string_free(s);
    if (c == 'n') return;

    p_ptr->au += price;
    stats_on_gold_selling(price);

    p_ptr->redraw |= PR_GOLD;
    if (prace_is_(RACE_MON_LEPRECHAUN))
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    obj->inscription = 0;
    obj->feeling = FEEL_NONE;
    obj->marked &= ~OM_WORN;
    obj->timeout = 0;

    obj_identify_fully(obj);
    stats_on_purchase(obj);

    object_desc(name, obj, OD_COLOR_CODED); /* again...in case *id* */
    msg_format("You sold %s for <color:R>%d</color> gold.", name, price);

    if (shop->type->id == SHOP_BLACK_MARKET)
        virtue_add(VIRTUE_JUSTICE, -1);

    if (obj->tval == TV_BOTTLE)
        virtue_add(VIRTUE_NATURE, 1);

    if (_add_obj(shop, obj))
        inv_sort(shop->inv);
}

static void _buy(_ui_context_ptr context)
{
    obj_prompt_t prompt = {0};
    int          amt = 1;

    prompt.prompt = "Sell which item?";
    prompt.error = "You have nothing to sell.";
    prompt.filter = context->shop->type->buy_p;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_QUIVER;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    if (prompt.obj->number > 1)
    {
        amt = get_quantity(NULL, prompt.obj->number);
        if (amt <= 0) return;
    }

    if (amt < prompt.obj->number)
    {
        obj_t copy = *prompt.obj;
        copy.number = amt;
        _buy_aux(context->shop, &copy);
    }
    else
        _buy_aux(context->shop, prompt.obj);

    obj_release(prompt.obj, OBJ_RELEASE_QUIET);
}

static void _examine(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;

        if (!msg_command("<color:y>Examine which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot - context->top + 1;
        obj = inv_obj(context->shop->inv, slot);
        if (!obj) continue;

        obj_display(obj);
    }
}

static void _reserve_aux(shop_ptr shop, obj_ptr obj)
{
    int        cost = _sell_price(shop, 10000);
    string_ptr s;
    char       c;
    char       name[MAX_NLEN];

    object_desc(name, obj, OD_COLOR_CODED);
    s = string_alloc_format("Reserve %s for <color:R>%d</color> gp? <color:y>[y/n]</color>", name, cost);
    c = msg_prompt(string_buffer(s), "ny", PROMPT_DEFAULT);
    string_free(s);
    if (c == 'n') return;
    if (cost > p_ptr->au)
    {
        msg_print("You don't have enough gold.");
        return;
    }
    p_ptr->au -= cost;
    stats_on_gold_services(cost);

    p_ptr->redraw |= PR_GOLD;
    if (prace_is_(RACE_MON_LEPRECHAUN))
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    obj->marked |= OM_RESERVED;
    msg_format("Done! I'll hold on to %s for you. You may come back at any time to purchase it.", name);
}

static void _reserve(_ui_context_ptr context)
{
    if (p_ptr->wizard || mut_present(MUT_MERCHANTS_FRIEND))
    {
        for (;;)
        {
            char    cmd;
            slot_t  slot;
            obj_ptr obj;

            if (!msg_command("<color:y>Reserve which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
            if (cmd < 'a' || cmd > 'z') continue;
            slot = label_slot(cmd);
            slot = slot - context->top + 1;
            obj = inv_obj(context->shop->inv, slot);
            if (!obj) continue;

            if (obj->marked & OM_RESERVED)
            {
                msg_print("You have already reserved that item. Choose another.");
                continue;
            }
            _reserve_aux(context->shop, obj);
            break;
        }
    }
    else
        msg_print("I will only reserve items in my stock for wizards or true friends of the merchant's guild.");
}

static void _sell_aux(shop_ptr shop, obj_ptr obj)
{
    char       name[MAX_NLEN];
    string_ptr s = string_alloc();
    char       c;
    int        price = obj_value(obj);

    price = _sell_price(shop, price);
    price *= obj->number;

    object_desc(name, obj, OD_COLOR_CODED);
    string_printf(s, "Really buy %s for <color:R>%d</color> gp? <color:y>[y/n]</color>", name, price);
    c = msg_prompt(string_buffer(s), "ny", PROMPT_DEFAULT);
    string_free(s);
    if (c == 'n') return;

    if (price > p_ptr->au)
    {
        msg_print("You do not have enough gold.");
        return;
    }
    p_ptr->au -= price;
    stats_on_gold_buying(price);

    p_ptr->redraw |= PR_GOLD;
    if (prace_is_(RACE_MON_LEPRECHAUN))
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);

    obj->ident &= ~IDENT_STORE;
    obj->inscription = 0;
    obj->feeling = FEEL_NONE;
    obj->marked &= ~OM_RESERVED;

    obj_identify_fully(obj);
    stats_on_purchase(obj);

    if (shop->type->id == SHOP_BLACK_MARKET)
        virtue_add(VIRTUE_JUSTICE, -1);

    if (obj->tval == TV_BOTTLE)
        virtue_add(VIRTUE_NATURE, -1);

    pack_carry(obj);
    msg_format("You have %s.", name);
}

static void _sell(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        int     amt = 1;

        if (!msg_command("<color:y>Buy which item <color:w>(<color:keypress>Esc</color> "
                         "to cancel)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot - context->top + 1;
        obj = inv_obj(context->shop->inv, slot);
        if (!obj) continue;

        if (obj->number > 1)
        {
            amt = get_quantity(NULL, obj->number);
            if (amt <= 0) break;
        }

        if (amt < obj->number)
        {
            obj_t copy = *obj;
            copy.number = amt;
            obj->number -= amt;
            _sell_aux(context->shop, &copy);
        }
        else
        {
            _sell_aux(context->shop, obj);
            if (!obj->number)
            {
                inv_remove(context->shop->inv, slot);
                inv_sort(context->shop->inv);
            }
        }
        break;
    }
}

static void _sellout(_ui_context_ptr context)
{
}

/************************************************************************
 * Stocking
 ***********************************************************************/
static void _maintain(shop_ptr shop)
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

static bool _can_cull(obj_ptr obj)
{
    if (obj && !(obj->marked & OM_RESERVED)) return TRUE;
    return FALSE;
}
static int _cull(shop_ptr shop, int target)
{
    int ct = inv_count_slots(shop->inv, obj_exists);
    int attempt;

    assert(ct >= target);
    for (attempt = 1; ct > target && attempt < 100; attempt++)
    {
        slot_t  slot = inv_random_slot(shop->inv, _can_cull);
        obj_ptr obj;

        if (!slot) break; /* nothing but 'Reserved' objects remain */

        obj = inv_obj(shop->inv, slot);
        assert(obj->number > 0);
        assert (!(obj->marked & OM_RESERVED));

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

static int _add_obj(shop_ptr shop, obj_ptr obj) /* return number of new slots used (0 or 1) */
{
    slot_t slot, max = inv_last(shop->inv, obj_exists);
    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr dest = inv_obj(shop->inv, slot);
        if (!dest) continue;
        if (obj_can_combine(dest, obj, INV_SHOP))
        {
            obj_combine(dest, obj, INV_SHOP);
            obj->number = 0; /* forget spillover */
            return 0;
        }
    }
    inv_add(shop->inv, obj);
    return 1;
}

static int _restock(shop_ptr shop, int target)
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

static void _shuffle_stock(shop_ptr shop)
{
    if (p_ptr->wizard || mut_present(MUT_MERCHANTS_FRIEND))
    {
        if (!p_ptr->wizard)
        {
            int        cost = _sell_price(shop, 5000);
            string_ptr s;
            char       c;
            s = string_alloc_format("Shuffle stock for <color:R>%d</color> gp? <color:y>[y/n]</color>", cost);
            c = msg_prompt(string_buffer(s), "ny", PROMPT_DEFAULT);
            string_free(s);
            if (c == 'n') return;
            if (cost > p_ptr->au)
            {
                msg_print("You don't have enough gold.");
                return;
            }
            p_ptr->au -= cost;
            stats_on_gold_services(cost);

            p_ptr->redraw |= PR_GOLD;
            if (prace_is_(RACE_MON_LEPRECHAUN))
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA);
        }
        _cull(shop, 0);
        _restock(shop, 12 + randint0(5));
    }
    else
        msg_print("I will only shuffle my stock for wizards or true friends of the merchant's guild.");
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

static void _display_inv(doc_ptr doc, shop_ptr shop, slot_t top, int page_size)
{
    slot_t  slot;
    int     xtra = 0;
    char    name[MAX_NLEN];
    inv_ptr inv = shop->inv;
    bool    show_prices = inv_loc(inv) == INV_SHOP;
    bool    show_values = inv_loc(inv) != INV_SHOP || p_ptr-> wizard;

    if (show_weights)
        xtra += 10;  /* " 123.0 lbs" */
    if (show_prices)
        xtra += 7;
    if (show_values)
        xtra += 7;

    doc_insert(doc, "    Item Description");
    if (xtra)
    {
        doc_printf(doc, "<tab:%d>", doc_width(doc) - xtra);
        if (show_weights)
            doc_printf(doc, " %9.9s", "Weight");
        if (show_prices)
            doc_printf(doc, " %6.6s", "Price");
        if (show_values)
            doc_printf(doc, " %6.6s", "Score");
    }
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
            if (xtra)
            {
                doc_pop_style(doc);
                doc_printf(doc, "<tab:%d>", doc_width(doc) - xtra);

                if (show_weights)
                {
                    int wgt = obj->weight; /* single object only for home/shops */
                    doc_printf(doc, " %3d.%d lbs", wgt/10, wgt%10);
                }
                if (show_prices || show_values)
                {
                    int value = obj_value(obj);

                    if (show_prices)
                    {
                        int price = _sell_price(shop, value);
                        doc_printf(doc, " <color:%c>%6d</color>", price <= p_ptr->au ? 'w' : 'D', price);
                    }
                    if (show_values)
                        doc_printf(doc, " %6d", value);
                }
            }
            doc_newline(doc);
        }
        else
            doc_newline(doc);
    }
}
void shop_display_inv(doc_ptr doc, inv_ptr inv, slot_t top, int page_size)
{
    shop_t hack = {0};
    hack.inv = inv;
    _display_inv(doc, &hack, top, page_size);
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

