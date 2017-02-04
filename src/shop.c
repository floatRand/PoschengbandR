#include "angband.h"

#include <assert.h>

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
            doc_style_t style = *doc_current_style(doc);

            object_desc(name, obj, OD_COLOR_CODED);

            doc_printf(doc, " %c) ", slot_label(slot - top + 1));
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

