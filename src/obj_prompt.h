#ifndef INCLUDED_OBJ_PROMPT_H
#define INCLUDED_OBJ_PROMPT_H

#include "obj.h"

typedef bool (*obj_prompt_cmd_f)(doc_ptr doc, inv_ptr current, char cmd);

#define MAX_LOC           5
#define OBJ_PROMPT_ALL    0x0001
#define OBJ_PROMPT_FORCE  0x0002
struct obj_prompt_s
{
    cptr  prompt;         /* "Wear/Wield which item?" */
    cptr  error;          /* "You have nothing you can wear or wield." */
    obj_p filter; 
    int   where[MAX_LOC]; /* INV_EQUIP, INV_FLOOR, etc. order matters */
    int   flags;

    obj_prompt_cmd_f
          cmd_handler;    /* return TRUE to eat the cmd */
};

typedef struct obj_prompt_s obj_prompt_t, *obj_prompt_ptr;

/* Prompt for an object. For simplicity, we simply return the
 * selected object or NULL if the user cancels (or there are
 * no valid objects meeting prompt.filter). Call obj_prompt_release()
 * if obj->number changes or if obj is a special return code. */
extern obj_ptr obj_prompt(obj_prompt_ptr prompt);

/* Now, we must pay for the simplicity. Sometimes you want to
 * be able to prompt with special options, such as selecting all
 * objects (or a Force-trainer prompts for a spellbook, but wants
 * to be able to choose their mental Force realm). In these cases
 * the returned object is a fake one, so you need to call one
 * of the following to check for it. You must always obj_prompt_release()
 * a fake object return code! */
extern bool    obj_prompt_all(obj_ptr obj);
extern bool    obj_prompt_force(obj_ptr obj);

/* You usually release the prompted object when you are done with
 * it. This will delete the object if obj->number was reduced to 
 * 0 and will also inform the user of the change (msg). If obj
 * is a special return code (i.e. fake object), then memory is freed.
 * If you prompt for an object and then do nothing with it, then you
 * should not release() it (e.g. trying to takeoff a cursed item). */
#define OBJ_RELEASE_QUIET 0x0001
extern void    obj_release(obj_ptr obj, int options);
#endif
