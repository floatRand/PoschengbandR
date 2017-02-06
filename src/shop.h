#ifndef INCLUDED_SHOP_H
#define INCLUDED_SHOP_H

#include "inv.h"

/************************************************************************
 * Shops
 ***********************************************************************/
struct shop_s;
typedef struct shop_s shop_t, *shop_ptr;

enum
{
    SHOP_NONE = -1, /* TODO: f_info.txt */
    SHOP_GENERAL = 0,
    SHOP_ARMORY,
    SHOP_WEAPON,
    SHOP_TEMPLE,
    SHOP_ALCHEMIST,
    SHOP_MAGIC,
    SHOP_BLACK_MARKET,
    SHOP_HOME,
    SHOP_BOOK,
    SHOP_MUSEUM,
    SHOP_JEWELER,
};

extern shop_ptr shop_alloc(int which);
extern shop_ptr shop_load(savefile_ptr file);
extern void     shop_free(shop_ptr shop);

extern void     shop_ui(shop_ptr shop);

extern void     shop_reset(shop_ptr shop); /* for dungeon shops */
extern bool     shop_common_cmd_handler(int cmd); /* shared with home_ui */
extern void     shop_display_inv(doc_ptr doc, inv_ptr inv, slot_t top, int page_size);
extern void     shop_save(shop_ptr shop, savefile_ptr file);


/************************************************************************
 * Towns
 ***********************************************************************/
struct town_s;
typedef struct town_s town_t, *town_ptr;

enum
{
    TOWN_OUTPOST = 1,
    TOWN_TELMORA,
    TOWN_MORIVANT,
    TOWN_ANGWIL,
    TOWN_ZUL,
    TOWN_RANDOM
};

extern void     towns_init(void);

extern town_ptr towns_current_town(void);
extern town_ptr towns_get_town(int which);
extern void     towns_save(savefile_ptr file);
extern void     towns_load(savefile_ptr file);

extern shop_ptr town_get_shop(town_ptr town, int which);

extern int      town_service_price(int price);
#endif
