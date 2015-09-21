#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "c-string.h"

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

string_ptr string_alloc(void)
{
    string_ptr res = malloc(sizeof(string_t));
    res->buf = 0;
    res->len = 0;
    res->size = 0;
    return res;
}

void string_free(string_ptr str)
{
    if (str->buf)
        free(str->buf);
    free(str);
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

void string_fgets(string_ptr str, FILE* fp)
{
    string_set(str, "");
    for (;;)
    {
        int c = fgetc(fp);
        if (c == EOF) break;
        if (c == '\n') break; /* unlike fgets, omit the line break */
        string_append_char(str, (char)c);
    }
}

int string_compare(const string_ptr left, const string_ptr right)
{
    if (!left->buf && !right->buf)
        return 0;
    else if (!left->buf)
        return -1;
    else if (!right->buf)
        return 1;

    return strcmp(left->buf, right->buf);
}

string_ptr string_copy(string_ptr str)
{
    string_ptr res = string_alloc();
    if (str->buf)
        string_set(res, str->buf);
    return res;
}

void string_format(string_ptr str, const char *fmt, ...)
{
    va_list vp;

    if (!str->buf)
        string_grow(str, 100);
    else
    {
        assert(str->size);
        str->buf[0] = '\0';
        str->len = 0;
    }

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
            /* Note: On Linux, we must va_start(vp, fmt) again before
               trying vsnprintf() on the larger buffer. On Windows, this
               is not necessary. This means code duplication ... see
               string_format_append() below. */
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

void string_format_append(string_ptr str, const char *fmt, ...)
{
    va_list vp;

    if (!str->buf)
        string_grow(str, 100);

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
        if (str->buf)
        {
            memcpy(buf, str->buf, str->size);
            free(str->buf);
        }
        else
            buf[0] = '\0';

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
    if (!str->buf)
        return 0;

    return string_hash_imp(str->buf);
}

void string_set(string_ptr str, const char *val)
{
    if (!val)
        string_set(str, "");
    else if (str->buf == val)
    {
    }
    else
    {
        int size = strlen(val) + 1;

        string_grow(str, size);
        memcpy(str->buf, val, size);
        str->len = size - 1;
    }
}

void string_shrink(string_ptr str, int size)
{
    if (size < str->size)
    {
        int   cb = 1;
        char *buf;

        if (str->buf)
            cb = str->len + 1;

        /* Never shrink below the current strlen() */
        if (size < cb)
            size = cb;

        buf = malloc(size);
        if (str->buf)
        {
            memcpy(buf, str->buf, cb);
            free(str->buf);
        }
        else
            buf[0] = '\0';
        str->size = size;
        str->buf = buf;
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
    return str->buf;
}

