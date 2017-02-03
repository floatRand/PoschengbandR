#ifndef INCLUDED_HOME_H
#define INCLUDED_HOME_H

#include "inv.h"

extern void    home_init(void);
extern inv_ptr home_filter(obj_p p);

extern void    home_ui(void);
extern void    home_display(doc_ptr doc, obj_p p, int flags);

extern void    home_load(savefile_ptr file);
extern void    home_save(savefile_ptr file);

#endif
