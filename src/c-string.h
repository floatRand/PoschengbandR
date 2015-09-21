#ifndef INCLUDED_C_STRING_H
#define INCLUDED_C_STRING_H

typedef struct string_s string_t;
typedef string_t *string_ptr;

extern string_ptr string_alloc(void);
extern string_ptr string_copy(string_ptr str);
extern void string_free(string_ptr str);

extern void string_set(string_ptr str, const char *val);
extern void string_append(string_ptr str, const char *val);
extern void string_fgets(string_ptr str, FILE* fp);
extern void string_append_char(string_ptr str, char ch);
extern void string_nappend(string_ptr str, const char *val, int cb);
extern void string_format(string_ptr str, const char *fmt, ...);
extern void string_format_append(string_ptr str, const char *fmt, ...);

extern int string_compare(const string_ptr left, const string_ptr right);
extern int string_hash(string_ptr str);
extern int string_hash_imp(const char *str);

extern void string_grow(string_ptr str, int size);
extern void string_shrink(string_ptr str, int size);
extern void string_trim(string_ptr str);

extern int string_length(string_ptr str);
extern const char * string_buffer(string_ptr str);

#endif
