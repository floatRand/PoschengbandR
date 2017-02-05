#ifndef INCLUDED_TOWN_H
#define INCLUDED_TOWN_H

#include "shop.h"

struct town_s;
typedef struct town_s town_t, *town_ptr;

enum
{
    TOWN_OUTPOST = 1,
    TOWN_TELMORA,
    TOWN_MORIVANT,
    TOWN_ANGWIL,
    TOWN_ZUL,
    TOWN_DUNGEON
};

extern void     towns_init(void);

extern town_ptr towns_current_town(void);
extern town_ptr towns_get_town(int which);
extern void     towns_save(savefile_ptr file);
extern void     towns_load(savefile_ptr file);

extern shop_ptr town_get_shop(town_ptr town, int which);

#endif
