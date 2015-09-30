#include "c-string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#define vsnprintf _vsnprintf
#endif

#ifdef _MSC_VER
/* Release code is 4x slower than debug code without the following! */
#pragma function(memset, strlen)
#endif

struct string_s
{
    int   size;
    int   len;
    char *buf;
};

string_ptr string_alloc(const char *val)
{
    if (!val)
        val = "";
    return string_nalloc(val, strlen(val));
}

string_ptr string_nalloc(const char *val, int cb)
{
    string_ptr res = malloc(sizeof(string_t));

    assert(val);
    res->buf = malloc(cb + 1);
    memcpy(res->buf, val, cb);
    res->len = cb;
    res->buf[res->len] = '\0';
    res->size = cb + 1;
    return res;
}

string_ptr string_salloc(string_ptr str)
{
    return string_nalloc(str->buf, str->len);
}

string_ptr string_falloc(FILE *fp)
{
    string_ptr res = string_alloc(NULL);
    string_read_file(res, fp);
    return res;
}

void string_free(string_ptr str)
{
    if (str)
    {
        free(str->buf);
        free(str);
    }
}

void string_append(string_ptr str, const char *val)
{
    if (!val)
        return;

    string_nappend(str, val, strlen(val));
}

void string_nappend(string_ptr str, const char *val, int cb)
{
    int cbl;  /* left += right */

    if (!cb)
        return;

    cbl = str->len;

    string_grow(str, cbl + cb + 1);
    memcpy(str->buf + cbl, val, cb);
    str->len += cb;
    str->buf[str->len] = '\0';
}

void string_append_char(string_ptr str, char ch)
{
    string_grow(str, str->len + 2);
    str->buf[str->len++] = ch;
    str->buf[str->len] = '\0';
}

void string_read_line(string_ptr str, FILE *fp)
{
    string_clear(str);
    for (;;)
    {
        int c = fgetc(fp);
        if (c == EOF) break;
        if (c == '\n') break;
        if (str->len >= str->size - 1)
            string_grow(str, str->size * 2);
        str->buf[str->len++] = c;
    }
    str->buf[str->len] = '\0';
}

void string_read_file(string_ptr str, FILE *fp)
{
    string_clear(str);
    for (;;)
    {
        int c = fgetc(fp);
        if (c == EOF) break;
        if (c == '\r') continue; /* \r\n -> \n */
        if (str->len >= str->size - 1)
            string_grow(str, str->size * 2);
        str->buf[str->len++] = c;
    }
    str->buf[str->len] = '\0';
}

void string_write_file(string_ptr str, FILE *fp)
{
    int i;
    for (i = 0; i < str->len; i++)
        fputc(str->buf[i], fp);
}

int string_compare(const string_ptr left, const string_ptr right)
{
    return strcmp(left->buf, right->buf);
}

void string_printf(string_ptr str, const char *fmt, ...)
{
    va_list vp;

    for (;;)
    {
        int cb = str->len;
        int res;

        va_start(vp, fmt);
        res = vsnprintf(str->buf + cb, str->size - cb, fmt, vp);
        va_end(vp);

        if (res >= str->size - cb)
        {
            str->buf[cb] = '\0';
            string_grow(str, cb + res + 1);
        }
        else if (res < 0)
        {
            str->buf[cb] = '\0';
            string_grow(str, str->size * 2);
        }
        else
        {
            str->len += res;
            break;
        }
    }
}

void string_grow(string_ptr str, int size)
{
    if (size > str->size)
    {
        int   new_size;
        char *buf;

        new_size = str->size * 2;
        if (new_size < size)
            new_size = size;

        buf = malloc(new_size);
        memcpy(buf, str->buf, str->size);
        free(str->buf);

        str->size = new_size;
        str->buf = buf;
    }
}

int string_hash_imp(const char *str) /* djb2 hash algorithm */
{
    int hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int string_hash(string_ptr str)
{
    return string_hash_imp(str->buf);
}

void string_clear(string_ptr str)
{
    str->len = 0;
    str->buf[str->len] = '\0';
}

void string_shrink(string_ptr str, int size)
{
    if (size < str->size)
    {
        int   cb = str->len + 1;
        char *buf;

        /* Never shrink below the current strlen() */
        if (size < cb)
            size = cb;

        buf = malloc(size);
        memcpy(buf, str->buf, cb);
        free(str->buf);

        str->size = size;
        str->buf = buf;
    }
}

void string_strip(string_ptr str)
{
    int i, j, k;
    for (i = 0; i < str->len; i++)
    {
        if (str->buf[i] != ' ') break;
    }
    for (j = str->len - 1; j > i; j--)
    {
        if (str->buf[j] != ' ') break;
    }
    if (0 < i || j < str->len - 1)
    {
        str->len = j - i + 1;
        for (k = 0; k < str->len; k++)
            str->buf[k] = str->buf[i+k];
        str->buf[str->len] = '\0';
    }
}

void string_trim(string_ptr str)
{
    string_shrink(str, 0);
}


int string_length(string_ptr str)
{
    return str->len;
}

const char *string_buffer(string_ptr str)
{
    if (str)
        return str->buf;
    return NULL;
}

int string_chr(string_ptr str, int start, char ch)
{
    if (start < str->len)
    {
        const char *pos = strchr(str->buf + start, ch);
        if (pos)
            return pos - str->buf;
    }
    return -1;
}

int string_last_chr(string_ptr str, char ch)
{
    int pos = 0;
    int result = -1;
    for (;;)
    {
        pos = string_chr(str, pos, ch);
        if (pos >= 0)
        {
            result = pos;
            pos++;
        }
        else
            break;
    }
    return result;
}

substring_t string_left(string_ptr str, int len)
{
    substring_t result;

    result.str = str;
    result.pos = 0;
    if (len <= str->len)
        result.len = len;
    else
        result.len = str->len;

    return result;
}

substring_t string_right(string_ptr str, int len)
{
    substring_t result;

    result.str = str;
    if (len <= str->len)
    {
        result.pos = str->len - len;
        result.len = len;
    }
    else
    {
        result.pos = 0;
        result.len = str->len;
    }

    return result;
}

const char *string_ssbuffer(substring_ptr ss)
{
    assert(ss->pos + ss->len <= ss->str->len);
    return ss->str->buf + ss->pos;
}

string_ptr string_ssalloc(substring_ptr ss)
{
    return string_nalloc(string_ssbuffer(ss), ss->len);
}

vec_ptr string_split(string_ptr str, char sep)
{
    vec_ptr     v = vec_alloc((vec_free_f)string_free);
    const char *pos = str->buf;
    int         done = 0;

    while (!done && *pos)
    {
        const char *next = strchr(pos, sep);
        string_ptr  s;

        if (!next && *pos)
        {
            next = strchr(pos, '\0');
            assert(next);
            done = 1;
        }

        s = string_nalloc(pos, next - pos);
        vec_add(v, s);
        pos = next + 1;
    }
    return v;
}

string_ptr string_join(vec_ptr vec, char sep)
{
    int        i;
    string_ptr result = string_alloc(NULL);

    for (i = 0; i < vec_length(vec); i++)
    {
        string_ptr s = vec_get(vec, i);
        if (i > 0)
            string_append_char(result, sep);
        string_nappend(result, s->buf, s->len);
    }
    return result;
}
