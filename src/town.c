#include "angband.h"

#include <assert.h>

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

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        shop_ptr shop = shop_load(file);
        int_map_add(town->shops, shop->type->id, shop);
    }
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

static void _town_save(town_ptr town, safefile_ptr file)
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



