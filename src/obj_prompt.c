#include "angband.h"

#include <assert.h>

static void _get_tabs(obj_prompt_context_ptr context)
{
    int i;

    context->tabs = vec_alloc((vec_free_f)inv_free);
    context->pages = vec_alloc(NULL);

    for (i = 0; i < MAX_LOC; i++)
    {
        inv_ptr inv = NULL;
        switch (context->prompt->where[i])
        {
        case INV_FLOOR:
            inv = inv_filter_floor(context->prompt->filter);
            break;
        case INV_EQUIP:
            inv = equip_filter(context->prompt->filter);
            break;
        case INV_PACK:
            inv = pack_filter(context->prompt->filter);
            break;
        case INV_QUIVER:
            inv = quiver_filter(context->prompt->filter);
            break;
        }
        if (inv)
        {
            int ct = inv_count(inv, obj_exists);
            if (!ct)
                inv_free(inv);
            else
            {
                inv_paginate(inv, context->prompt->filter, context->page_size);
                vec_add(context->tabs, inv);
                vec_add_int(context->pages, 0);
            }
        }
    }
}

static void _sync_doc(doc_ptr doc)
{
    Term_load();
    doc_sync_menu(doc);
}

static void _display(obj_prompt_context_ptr context)
{
    inv_ptr inv = vec_get(context->tabs, context->tab);
    int     page = vec_get_int(context->pages, context->tab);
    int     i;

    doc_clear(context->doc);
    doc_insert(context->doc, "<style:table>");
    /* Tab Headers */
    for (i = 0; i < vec_length(context->tabs); i++)
    {
        inv_ptr inv = vec_get(context->tabs, i);
        if (i)
            doc_insert(context->doc, " <color:b>|</color> ");
        doc_printf(context->doc, "<color:%c>%s</color>",
            i == context->tab ? 'G' : 'D',
            inv_name(inv));
    }
    if (context->prompt->flags & INV_SHOW_FAIL_RATES)
        doc_printf(context->doc, "<tab:%d><color:r>Fail</color>", doc_width(context->doc) - 5);
    doc_newline(context->doc);

    /* Active Tab */
    inv_display_page(inv, page, context->doc, context->prompt->flags);
    if (inv_page_count(inv) > 1)
    {
        int page = vec_get_int(context->pages, context->tab);
        int ct = inv_page_count(inv);

        doc_printf(context->doc,
            "<color:B>-%s-</color> <color:G>Page %d of %d</color>\n",
            page == ct - 1 ? "less" : "more",
            page + 1, ct);
    }
    else
        doc_newline(context->doc);
    if (context->prompt->prompt)
        doc_printf(context->doc, "<color:y>%s</color> ", context->prompt->prompt);
    else
        doc_insert(context->doc, "<color:y>Choice</color>: ");

    doc_insert(context->doc, "</style>");
    _sync_doc(context->doc);
}

static int _find_tab(vec_ptr tabs, int loc)
{
    int i;
    for (i = 0; i < vec_length(tabs); i++)
    {
        inv_ptr inv = vec_get(tabs, i);
        if (inv_loc(inv) == loc) return i;
    }
    return -1;
}

static void _context_free(obj_prompt_context_ptr context)
{
    assert(context);
    vec_free(context->tabs);
    vec_free(context->pages);
    doc_free(context->doc);
}

static int _basic_cmd(obj_prompt_context_ptr context, int cmd)
{
    switch (cmd)
    {
    case '-': {
        /* Legacy: In the olden days, - was used to autopick
         * the floor item. Of course, you had to do it blind
         * since it was never displayed. */
        int floor_tab = _find_tab(context->tabs, INV_FLOOR);
        if (floor_tab >= 0)
        {
            inv_ptr inv = vec_get(context->tabs, floor_tab);
            context->tab = floor_tab;
            if (inv_count_slots(inv, obj_exists) == 1)
            {
                slot_t slot = inv_first(inv, obj_exists);
                assert(slot);
                context->prompt->obj = inv_obj(inv, slot);
                REPEAT_PUSH(inv_loc(inv));
                REPEAT_PUSH('a');
                return OP_CMD_DISMISS;
            }
        }
        return OP_CMD_HANDLED; }
    case '/':
        context->tab++;
        if (context->tab == vec_length(context->tabs))
            context->tab = 0;
        return OP_CMD_HANDLED;
    case '\\':
        context->tab--;
        if (context->tab < 0)
            context->tab = vec_length(context->tabs) - 1;
        return OP_CMD_HANDLED;
    case SKEY_PGDOWN: case '3': {
        int     page = vec_get_int(context->pages, context->tab);
        inv_ptr inv = vec_get(context->tabs, context->tab);
        if (page < inv_page_count(inv) - 1)
            vec_set_int(context->pages, context->tab, page + 1);
        return OP_CMD_HANDLED; }
    case SKEY_PGUP: case '9': {
        int     page = vec_get_int(context->pages, context->tab);
        if (page > 0)
            vec_set_int(context->pages, context->tab, page - 1);
        return OP_CMD_HANDLED; }
    case '?':
        if (context->prompt->help)
            doc_display_help(context->prompt->help, NULL);
        else
            doc_display_help("command.txt", "SelectingObjects");
        return OP_CMD_HANDLED;
    }
    return OP_CMD_SKIPPED;
}

int obj_prompt(obj_prompt_ptr prompt)
{
    obj_prompt_context_t context = {0};
    int                  tmp;
    int                  result = 0;

    context.prompt = prompt;
    context.page_size = MIN(26, ui_menu_rect().cy - 4);
    _get_tabs(&context);

    if (!vec_length(context.tabs))
    {
        if (prompt->error)
            msg_format("<color:r>%s</color>", prompt->error);
        _context_free(&context);
        return OP_NO_OBJECTS;
    }

    if (REPEAT_PULL(&tmp))
    {
        int repeat_tab = _find_tab(context.tabs, tmp);
        if (repeat_tab >= 0 && REPEAT_PULL(&tmp))
        {
            inv_ptr inv = vec_get(context.tabs, repeat_tab);
            slot_t  slot;

            inv_calculate_page_labels(inv, 0);
            slot = inv_label_slot(inv, tmp);
            if (slot)
            {
                prompt->obj = inv_obj(inv, slot);
                _context_free(&context);
                return OP_SUCCESS;
            }
        }
    }

    context.doc = doc_alloc(MIN(80, ui_map_rect().cx));
    Term_save();
    for (;;)
    {
        inv_ptr inv = vec_get(context.tabs, context.tab);
        int     cmd;
        slot_t  slot;

        _display(&context);

        cmd = inkey_special(TRUE);
        if (cmd == ' ') continue;
        tmp = _basic_cmd(&context, cmd);
        if (tmp == OP_CMD_HANDLED) continue;
        if (tmp == OP_CMD_DISMISS)
        {
            result = OP_SUCCESS;
            break;
        }
        if (prompt->cmd_handler)
        {
            tmp = prompt->cmd_handler(&context, cmd);
            if (tmp == OP_CMD_HANDLED) continue;
            if (tmp == OP_CMD_DISMISS)
            {
                result = OP_CUSTOM;
                break;
            }
        }
        slot = inv_label_slot(inv, cmd);
        if (slot)
        {
            prompt->obj = inv_obj(inv, slot);
            result = OP_SUCCESS;
            REPEAT_PUSH(inv_loc(inv));
            /* repeat can be dangerous if the pack shuffles */
            if (vec_get_int(context.pages, context.tab) == 0 && object_is_aware(prompt->obj))
                REPEAT_PUSH(cmd);
            break;
        }
        else if (isupper(cmd))
        {
            slot = inv_label_slot(inv, tolower(cmd));
            if (slot)
            {
                doc_clear(context.doc);
                obj_display_doc(inv_obj(inv, slot), context.doc);
                _sync_doc(context.doc);
                cmd = inkey();
                continue;
            }
        }
        else if (cmd == ESCAPE || cmd == '\r')
        {
            result = OP_CANCELED;
            break;
        }
    }
    Term_load();
    _context_free(&context);
    return result;
}







