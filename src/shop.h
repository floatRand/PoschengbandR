#ifndef INCLUDED_SHOP_H
#define INCLUDED_SHOP_H

extern bool shop_common_cmd_handler(int cmd);
extern void shop_display_inv(doc_ptr doc, inv_ptr inv, slot_t top, int page_size);

#endif
