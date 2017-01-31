#include "angband.h"

#include <assert.h>

#define _FAKE_SLOT_ALL   1000000
#define _FAKE_SLOT_FORCE 1000001

static vec_ptr _get_tabs(obj_prompt_ptr prompt)
{
    vec_ptr vec = vec_alloc((vec_free_f)inv_free);
    int     i;

    for (i = 0; i < MAX_LOC; i++)
    {
        inv_ptr inv = NULL;
        switch (prompt->where[i])
        {
        case INV_FLOOR:
            inv = inv_filter_floor(prompt->filter);
            break;
        case INV_EQUIP:
            inv = equip_filter(prompt->filter);
            break;
        case INV_PACK:
            inv = pack_filter(prompt->filter);
            break;
        case INV_QUIVER:
            inv = quiver_filter(prompt->filter);
            break;
        }
        if (inv)
        {
            int ct = inv_count(inv, obj_exists);
            if (!ct)
                inv_free(inv);
            else
                vec_add(vec, inv);
        }
    }
    return vec;
}

static void _sync_doc(doc_ptr doc)
{
    Term_load();
    doc_sync_menu(doc);
}

static void _display(obj_prompt_context_ptr context)
{
    inv_ptr inv = vec_get(context->tabs, context->tab);
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
    inv_display(inv, 1, inv_max(inv), obj_exists, context->doc, NULL, context->prompt->flags);
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

int obj_prompt(obj_prompt_ptr prompt)
{
    obj_prompt_context_t context = {0};
    int                  tmp;
    int                  result = 0;

    context.tabs = _get_tabs(prompt);
    context.prompt = prompt;

    if (!vec_length(context.tabs))
    {
        if (prompt->error)
            msg_format("<color:r>%s</color>", prompt->error);
        vec_free(context.tabs);
        return OP_NO_OBJECTS;
    }

    if (REPEAT_PULL(&tmp))
    {
        int repeat_tab = _find_tab(context.tabs, tmp);
        if (repeat_tab >= 0 && REPEAT_PULL(&tmp))
        {
            inv_ptr inv = vec_get(context.tabs, repeat_tab);
            slot_t  slot;

            inv_calculate_labels(inv, 1, 0);
            slot = inv_label_slot(inv, tmp);
            if (slot)
            {
                prompt->obj = inv_obj(inv, slot);
                vec_free(context.tabs);
                return OP_SUCCESS;
            }
        }
    }

    context.doc = doc_alloc(MIN(80, ui_map_rect().cx));
    Term_save();
    for (;;)
    {
        inv_ptr inv = vec_get(context.tabs, context.tab);
        char    cmd;
        slot_t  slot;

        _display(&context);

        cmd = inkey();
        if (cmd == ' ') continue;
        if (cmd == '-')
        {
            /* Legacy: In the olden days, - was used to autopick
             * the floor item. Of course, you had to do it blind
             * since it was never displayed. */
            int floor_tab = _find_tab(context.tabs, INV_FLOOR);
            if (floor_tab >= 0)
            {
                context.tab = floor_tab;
                inv = vec_get(context.tabs, context.tab);
                if (inv_count_slots(inv, obj_exists) == 1)
                {
                    slot = inv_first(inv, obj_exists);
                    assert(slot);
                    prompt->obj = inv_obj(inv, slot);
                    result = OP_SUCCESS;
                    REPEAT_PUSH(cmd);
                    break;
                }
            }
            continue;
        }
        if (prompt->cmd_handler)
        {
            tmp = prompt->cmd_handler(&context, cmd);
            if (tmp == OP_CMD_HANDLED)
                continue;
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
            REPEAT_PUSH(cmd);
            break;
        }
        if (cmd == '/')
        {
            context.tab++;
            if (context.tab == vec_length(context.tabs))
                context.tab = 0;
        }
        else if (cmd == '\\')
        {
            context.tab--;
            if (context.tab < 0)
                context.tab = vec_length(context.tabs) - 1;
        }
        else if (cmd == '?')
        {
            if (prompt->help)
                doc_display_help(prompt->help, NULL);
            else
                doc_display_help("command.txt", "SelectingObjects");
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

    doc_free(context.doc);
    vec_free(context.tabs);

    return result;
}







