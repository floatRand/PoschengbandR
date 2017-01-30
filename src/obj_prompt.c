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

static void _display(doc_ptr doc, vec_ptr tabs, int tab, obj_prompt_ptr prompt)
{
    inv_ptr inv = vec_get(tabs, tab);
    int     i;

    doc_clear(doc);

    /* Tab Headers */
    for (i = 0; i < vec_length(tabs); i++)
    {
        inv_ptr inv = vec_get(tabs, i);
        if (i)
            doc_insert(doc, " <color:b>|</color> ");
        doc_printf(doc, "<color:%c>%s</color>",
            i == tab ? 'G' : 'D',
            inv_name(inv));
    }
    doc_newline(doc);

    /* Active Tab */
    inv_display(inv, 1, inv_max(inv), obj_exists, doc, NULL, 0);
    doc_newline(doc);
    #if 0
    doc_insert(doc, "[");
    if (prompt->flags & OBJ_PROMPT_ALL)
        doc_insert(doc, "<color:keypress>*</color> for all, ");
    if (prompt->flags & OBJ_PROMPT_FORCE)
        doc_insert(doc, "<color:keypress>F</color> for the Force, ");
    if (vec_length(tabs) > 1)
        doc_insert(doc, "<color:keypress>/</color> for next tab, ");
    doc_insert(doc, "<color:keypress>?</color> for help]\n");
    #endif 
    if (prompt->prompt)
        doc_insert(doc, prompt->prompt);
    else
        doc_insert(doc, "<color:y>Choice</color>: ");

    _sync_doc(doc);
}

obj_ptr obj_prompt(obj_prompt_ptr prompt)
{
    obj_ptr  result = NULL;
    vec_ptr  tabs = _get_tabs(prompt);
    doc_ptr  doc;
    int      tab = 0;

    if (!vec_length(tabs))
    {
        if (prompt->error)
            msg_print(prompt->error);
        vec_free(tabs);
        return NULL;
    }

    doc = doc_alloc(MIN(80, ui_map_rect().cx));
    Term_save();
    for (;;)
    {
        inv_ptr inv = vec_get(tabs, tab);
        char    cmd;
        slot_t  slot;

        _display(doc, tabs, tab, prompt);

        cmd = inkey();
        if (cmd == ' ') continue;
        if (prompt->cmd_handler)
        {
            if (prompt->cmd_handler(doc, inv, cmd))
                continue;
        }
        slot = inv_label_slot(inv, cmd);
        if (slot)
        {
            result = inv_obj(inv, slot);
            break;
        }
        if (isupper(cmd))
        {
            slot = inv_label_slot(inv, tolower(cmd));
            if (slot)
            {
                doc_clear(doc);
                obj_display_doc(inv_obj(inv, slot), doc);
                _sync_doc(doc);
                cmd = inkey();
                continue;
            }
        }
        if (cmd == '/')
        {
            tab++;
            if (tab == vec_length(tabs))
                tab = 0;
        }
        else if (cmd == '\\')
        {
            tab--;
            if (tab < 0)
                tab = vec_length(tabs) - 1;
        }
        else if ((prompt->flags & OBJ_PROMPT_ALL) && cmd == '*')
        {
            result = obj_alloc();
            result->loc.where = inv_loc(inv);
            result->loc.slot = _FAKE_SLOT_ALL;
            break;
        }
        else if ((prompt->flags & OBJ_PROMPT_FORCE) && cmd == 'F')
        {
            result = obj_alloc();
            result->loc.where = inv_loc(inv);
            result->loc.slot = _FAKE_SLOT_FORCE;
            break;
        }
        else if (cmd == '?')
        {
            doc_display_help("command.txt", "SelectingObjects");
        }
        else if (cmd == ESCAPE || cmd == '\r')
        {
            break;
        }
    }
    Term_load();

    doc_free(doc);
    vec_free(tabs);

    return result;
}

bool obj_prompt_all(obj_ptr obj)
{
    assert(obj);
    if (obj->loc.slot == _FAKE_SLOT_ALL) return TRUE;
    return FALSE;
}

bool obj_prompt_force(obj_ptr obj)
{
    assert(obj);
    if (obj->loc.slot == _FAKE_SLOT_FORCE) return TRUE;
    return FALSE;
}

static bool obj_prompt_special(obj_ptr obj)
{
    assert(obj);
    if (obj->loc.slot >= _FAKE_SLOT_ALL) return TRUE;
    return FALSE;
}

void obj_release(obj_ptr obj, int options)
{
    char name[MAX_NLEN];
    bool quiet = (options & OBJ_RELEASE_QUIET) ? TRUE : FALSE;

    assert(obj);
    if (obj_prompt_special(obj))
    {
        obj_free(obj);
        return;
    }
    if (!quiet)
        object_desc(name, obj, OD_COLOR_CODED);

    switch (obj->loc.where)
    {
    case INV_FLOOR:
        if (!quiet)
            msg_format("You see %s.", name);
        if (obj->number <= 0)
            delete_object_idx(obj->loc.slot);
        break;
    case INV_EQUIP:
        if (!quiet)
            msg_format("You are no longer wearing %s.", name);
        if (obj->number <= 0)
            equip_remove(obj->loc.slot);
        break;
    case INV_PACK:
        if (!quiet)
            msg_format("You have %s in your pack.", name);
        if (obj->number <= 0)
            pack_remove(obj->loc.slot);
        break;
    case INV_QUIVER:
        if (!quiet)
            msg_format("You have %s in your quiver.", name);
        if (obj->number <= 0)
            quiver_remove(obj->loc.slot);
        break;
    }
}







