#include "angband.h"

#include <assert.h>

static void _context_make(obj_prompt_context_ptr context);
static void _context_unmake(obj_prompt_context_ptr context);
static int  _find_tab(vec_ptr tabs, int loc);

static void _display(obj_prompt_context_ptr context);
static void _sync_doc(doc_ptr doc);

static int  _basic_cmd(obj_prompt_context_ptr context, int cmd);

int obj_prompt(obj_prompt_ptr prompt)
{
    obj_prompt_context_t context = {0};
    int                  tmp;
    int                  result = 0;

    context.prompt = prompt;
    context.page_size = MIN(26, ui_menu_rect().cy - 4);
    _context_make(&context);

    if (!vec_length(context.tabs))
    {
        if (prompt->error)
            msg_format("<color:r>%s</color>", prompt->error);
        _context_unmake(&context);
        return OP_NO_OBJECTS;
    }

    if (REPEAT_PULL(&tmp))
    {
        int repeat_tab = _find_tab(context.tabs, tmp);
        if (repeat_tab >= 0 && REPEAT_PULL(&tmp))
        {
            obj_prompt_tab_ptr tab = vec_get(context.tabs, repeat_tab);
            slot_t             slot;

            inv_calculate_labels(tab->inv, 1, context.page_size, prompt->flags);
            slot = inv_label_slot(tab->inv, tmp);
            if (slot)
            {
                prompt->obj = inv_obj(tab->inv, slot);
                _context_unmake(&context);
                return OP_SUCCESS;
            }
        }
    }

    context.doc = doc_alloc(MIN(80, ui_map_rect().cx));
    Term_save();
    for (;;)
    {
        obj_prompt_tab_ptr tab = vec_get(context.tabs, context.tab);
        int                cmd;
        slot_t             slot;

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
        slot = inv_label_slot(tab->inv, cmd);
        if (slot)
        {
            prompt->obj = inv_obj(tab->inv, slot);
            result = OP_SUCCESS;
            REPEAT_PUSH(inv_loc(tab->inv));
            /* repeat can be dangerous if the pack shuffles */
            if (tab->ct <= context.page_size && object_is_aware(prompt->obj))
                REPEAT_PUSH(cmd);
            break;
        }
        else if (isupper(cmd))
        {
            slot = inv_label_slot(tab->inv, tolower(cmd));
            if (slot)
            {
                doc_clear(context.doc);
                obj_display_doc(inv_obj(tab->inv, slot), context.doc);
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
    _context_unmake(&context);
    return result;
}

static obj_prompt_tab_ptr _tab_alloc(inv_ptr inv, int page_size)
{
    obj_prompt_tab_ptr tab = malloc(sizeof(obj_prompt_tab_t));
    tab->inv = inv;
    tab->ct = inv_count_slots(inv, obj_exists);
    if (tab->ct)
        tab->page_ct = (tab->ct - 1) / page_size + 1;
    else
        tab->page_ct = 0;
    tab->page = 0;
    return tab;
}

static void _tab_free(obj_prompt_tab_ptr tab)
{
    if (tab)
    {
        inv_free(tab->inv);
        tab->inv = NULL;
        free(tab);
    }
}

static void _context_make(obj_prompt_context_ptr context)
{
    int i;

    context->tabs = vec_alloc((vec_free_f)_tab_free);

    for (i = 0; i < MAX_LOC; i++)
    {
        inv_ptr inv = NULL;
        switch (context->prompt->where[i])
        {
        case INV_FLOOR:
            inv = inv_filter_floor(context->prompt->filter);
            inv_sort(inv);
            break;
        case INV_EQUIP:
            inv = equip_filter(context->prompt->filter);
            /* don't sort: always keep same slot labels! */
            break;
        case INV_PACK:
            inv = pack_filter(context->prompt->filter);
            inv_sort(inv);
            break;
        case INV_QUIVER:
            inv = quiver_filter(context->prompt->filter);
            inv_sort(inv);
            break;
        }
        if (inv)
        {
            int ct = inv_count(inv, obj_exists);
            if (!ct)
                inv_free(inv);
            else
                vec_add(context->tabs, _tab_alloc(inv, context->page_size));
        }
    }
}

static void _context_unmake(obj_prompt_context_ptr context)
{
    assert(context);
    vec_free(context->tabs);
    doc_free(context->doc);
}

static void _sync_doc(doc_ptr doc)
{
    Term_load();
    doc_sync_menu(doc);
}

static void _display(obj_prompt_context_ptr context)
{
    obj_prompt_tab_ptr tab;
    int                i;

    doc_clear(context->doc);
    doc_insert(context->doc, "<style:table>");
    /* Tab Headers */
    for (i = 0; i < vec_length(context->tabs); i++)
    {
        tab = vec_get(context->tabs, i);
        if (i)
            doc_insert(context->doc, " <color:b>|</color> ");
        doc_printf(context->doc, "<color:%c>%s</color>",
            i == context->tab ? 'G' : 'D',
            inv_name(tab->inv));
    }
    if (context->prompt->flags & INV_SHOW_FAIL_RATES)
        doc_printf(context->doc, "<tab:%d><color:r>Fail</color>", doc_width(context->doc) - 5);
    doc_newline(context->doc);

    /* Active Tab */
    tab = vec_get(context->tabs, context->tab);
    inv_display(
        tab->inv,
        tab->page * context->page_size + 1,
        (tab->page + 1) * context->page_size,
        obj_exists,
        context->doc, context->prompt->flags
    );

    if (tab->page_ct > 1)
    {
        doc_printf(context->doc,
            "<color:B>-%s-</color> <color:G>Page %d of %d</color>\n",
            tab->page == tab->page_ct - 1 ? "less" : "more",
            tab->page + 1, tab->page_ct);
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
        obj_prompt_tab_ptr tab = vec_get(tabs, i);
        if (inv_loc(tab->inv) == loc) return i;
    }
    return -1;
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
            obj_prompt_tab_ptr tab = vec_get(context->tabs, floor_tab);
            context->tab = floor_tab;
            if (inv_count_slots(tab->inv, obj_exists) == 1)
            {
                slot_t slot = inv_first(tab->inv, obj_exists);
                assert(slot);
                context->prompt->obj = inv_obj(tab->inv, slot);
                REPEAT_PUSH(inv_loc(tab->inv));
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
        obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
        if (tab->page < tab->page_ct - 1)
            tab->page++;
        return OP_CMD_HANDLED; }
    case SKEY_PGUP: case '9': {
        obj_prompt_tab_ptr tab = vec_get(context->tabs, context->tab);
        if (tab->page > 0)
            tab->page--;
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



