#ifndef INCLUDED_OBJ_H
#define INCLUDED_OBJ_H

struct object_type; /* s/b obj_t */
typedef struct object_type obj_t, *obj_ptr;
typedef void (*obj_f)(obj_ptr obj);
typedef bool (*obj_p)(obj_ptr obj);

extern obj_ptr obj_alloc(void);
extern obj_ptr obj_copy(obj_ptr obj);
extern void    obj_clear_dun_info(obj_ptr obj);

/* Predicates */
extern bool obj_is_art(obj_ptr obj);
extern bool obj_is_ego(obj_ptr obj);
extern bool obj_exists(obj_ptr obj); /* Useful for counting/iterating occupied slots */

/* Sorting */
extern void obj_clear_scratch(obj_ptr obj); /* Call before sorting ... scratch is used to memoize obj_value */
extern int  obj_cmp(obj_ptr left, obj_ptr right);

/* Menus */
extern char obj_label(obj_ptr obj);

/* Stacking */
#define OBJ_STACK_MAX     99
extern bool obj_can_combine(obj_ptr dest, obj_ptr obj, int options);
extern int  obj_combine(obj_ptr dest, obj_ptr obj, int options);

/* Savefiles */
extern void obj_load(obj_ptr obj, savefile_ptr file);
extern void obj_save(obj_ptr obj, savefile_ptr file);
#endif
