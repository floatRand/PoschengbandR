#include "angband.h"

#include <assert.h>

static inv_ptr _inv = NULL;

void home_init(void)
{
    inv_free(_inv);
    _inv = inv_alloc("Home", INV_HOME, 0);
}

inv_ptr home_filter(obj_p p)
{
    return inv_filter(_inv, p);
}

static void home_carry(obj_ptr obj)
{
    if (obj->number)
        inv_combine_ex(_inv, obj);
    if (obj->number)
        inv_add(_inv, obj);
}

static void _display(doc_ptr doc, slot_t top, int page_size)
{
    rect_t r = ui_shop_rect();

    doc_clear(doc);
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "%*s<color:G>%s</color>\n\n", (r.cx - 10)/2, "", "Your Home");
    shop_display_inv(doc, _inv, top, page_size);
    doc_insert(doc, "<color:keypress>g</color> to get an item. "
                    "<color:keypress>d</color> to drop an item. "
                    "<color:keypress>x</color> to begin examining items.\n"
                    "<color:keypress>Esc</color> to exit home. "
                    "<color:keypress>PageUp/Down</color> to scroll. "
                    "<color:keypress>?</color> for help.");
    doc_insert(doc, "</style>");


    Term_clear_rect(r);
    doc_sync_term(doc, doc_range_top_lines(doc, r.cy), doc_pos_create(r.x, r.y));
}

static void _get_aux(obj_ptr obj)
{
    char name[MAX_NLEN];
    object_desc(name, obj, OD_COLOR_CODED);
    msg_format("You get %s.", name);
    pack_carry(obj);
}

static void _get(int top)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        int     amt = 1;

        if (!msg_command("<color:y>Get which item <color:w>(<color:keypress>Esc</color> to cancel)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot - top + 1;
        obj = inv_obj(_inv, slot);
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
            _get_aux(&copy);
        }
        else
        {
            _get_aux(obj);
            if (!obj->number)
            {
                inv_remove(_inv, slot);
                inv_sort(_inv);
            }
        }
        break;
    }
}

static void _drop_aux(obj_ptr obj)
{
    char name[MAX_NLEN];
    object_desc(name, obj, OD_COLOR_CODED);
    msg_format("You drop %s.", name);
    home_carry(obj);
    inv_sort(_inv);
}

static void _drop(void)
{
    obj_prompt_t prompt = {0};
    int          amt = 1;

    prompt.prompt = "Drop which item?";
    prompt.error = "You have nothing to drop.";
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    if (prompt.obj->number > 1)
    {
        amt = get_quantity(NULL, prompt.obj->number);
        if (amt <= 0)
            return;
    }

    if (amt < prompt.obj->number)
    {
        obj_t copy = *prompt.obj;
        copy.number = amt;
        prompt.obj->number -= amt;
        _drop_aux(&copy);
    }
    else
        _drop_aux(prompt.obj);

    obj_release(prompt.obj, OBJ_RELEASE_QUIET);
}

static void _examine(int top)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;

        if (!msg_command("<color:y>Examine which item <color:w>(<color:keypress>Esc</color> when done)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot - top + 1;
        obj = inv_obj(_inv, slot);
        if (!obj) continue;

        obj_display(obj);
    }
}

void home_ui(void)
{
    doc_ptr doc;
    slot_t  top = 1;

    forget_lite(); /* resizing the term would redraw the map ... sigh */
    forget_view();
    character_icky = TRUE;

    msg_line_clear();
    msg_line_init(ui_shop_msg_rect());

    Term_clear();
    doc = doc_alloc(MIN(80, ui_shop_rect().cx));
    for (;;)
    {
        int    max = inv_last(_inv, obj_exists);
        rect_t r = ui_shop_rect(); /* recalculate in case resize */
        int    page_size = MIN(26, r.cy - 3 - 2);
        int    cmd;

        _display(doc, top, page_size);
        cmd = inkey_special(TRUE);
        msg_line_clear();
        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q') break;
        pack_lock();
        if (!shop_common_cmd_handler(cmd))
        {
            switch (cmd)
            {
            case 'g': _get(top); break;
            case 'd': _drop(); break;
            case 'x': _examine(top); break;
            case '?':
                doc_display_help("context_home.txt", NULL);
                Term_clear_rect(ui_shop_msg_rect());
                break;
            case SKEY_PGDOWN: case '3':
                if (top + page_size < max)
                    top += page_size;
                break;
            case SKEY_PGUP: case '9':
                if (top > page_size)
                    top -= page_size;
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

    doc_free(doc);
}

void home_display(doc_ptr doc, obj_p p, int flags)
{
    slot_t slot;
    slot_t max = inv_last(_inv, obj_exists);
    char   name[MAX_NLEN];

    if (max > 100)
    {
        doc_printf(doc, "You have %d items in your home. Here are the top 100:\n", max);
        max = 100;
    }

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(_inv, slot);
        if (!obj) continue; /* bug */
        object_desc(name, obj, OD_COLOR_CODED);
        doc_printf(doc, "%3d) <indent><style:indent>%s</style></indent>\n", slot, name);
    }
}

void home_load(savefile_ptr file)
{
    inv_load(_inv, file);
}

void home_save(savefile_ptr file)
{
    inv_save(_inv, file);
}

