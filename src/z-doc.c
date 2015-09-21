#include "z-doc.h"

#include "angband.h"
#include "c-string.h"

#include <assert.h>

#define PAGE_HEIGHT 128
#define PAGE_NUM(a) ((a)>>7)
#define PAGE_OFFSET(a) ((a)%128)

doc_ptr doc_alloc(int width)
{
    doc_ptr res = malloc(sizeof(doc_t));

    res->cursor.x = 0;
    res->cursor.y = 0;
    res->width = width;
    res->pages = vec_alloc(free);
    res->styles = str_map_alloc(free);
    res->topics = str_map_alloc(free);
    res->links = int_map_alloc(free);

    res->current_style.a = TERM_WHITE;
    res->current_style.indent = 0;

    return res;
}

void doc_free(doc_ptr doc)
{
    vec_free(doc->pages);
    str_map_free(doc->styles);
    str_map_free(doc->topics);
    int_map_free(doc->links);

    free(doc);
}

doc_pos_t doc_cursor(doc_ptr doc)
{
    return doc->cursor;
}

bool doc_pos_is_valid(doc_pos_t pos)
{
    if (pos.x >= 0 && pos.y >= 0)
        return TRUE;
    return FALSE;
}

bool doc_region_is_valid(doc_region_ptr region)
{
    if (region)
    {
        if (doc_pos_compare(region->start, region->stop) <= 0)
            return TRUE;
    }
    return FALSE;
}

int doc_pos_compare(doc_pos_t left, doc_pos_t right)
{
    if (left.y < right.y)
        return -1;
    if (left.y > right.y)
        return 1;
    if (left.x < right.x)
        return -1;
    if (left.x > right.x)
        return 1;
    return 0;
}

bool doc_region_contains(doc_region_ptr region, doc_pos_t pos)
{
    if (doc_region_is_valid(region) && doc_pos_is_valid(pos))
    {
        if ( doc_pos_compare(region->start, pos) <= 0
          && doc_pos_compare(region->stop, pos) > 0 )
        {
            return TRUE;
        }
    }
    return FALSE;
}

doc_pos_t doc_current_topic(doc_ptr doc, doc_pos_t pos)
{
    doc_pos_t        result = {-1, -1};
    str_map_iter_ptr iter;

    for (iter = str_map_iter_alloc(doc->topics);
            str_map_iter_is_valid(iter);
            str_map_iter_next(iter))
    {
        doc_region_ptr region = str_map_iter_current(iter);
        if (doc_region_contains(region, pos))
        {
            result = region->start;
            break;
        }
    }
    str_map_iter_free(iter);
    return result;
}

doc_pos_t doc_next_topic(doc_ptr doc, doc_pos_t pos)
{
    doc_pos_t        result = {-1, -1};
    str_map_iter_ptr iter;

    for (iter = str_map_iter_alloc(doc->topics);
            str_map_iter_is_valid(iter);
            str_map_iter_next(iter))
    {
        doc_region_ptr region = str_map_iter_current(iter);
        if (doc_pos_compare(pos, region->start) < 0)
        {
            if ( !doc_pos_is_valid(result)
              || doc_pos_compare(region->start, result) < 0)
            {
                result = region->start;
            }
        }
    }
    str_map_iter_free(iter);
    return result;
}

doc_pos_t doc_prev_topic(doc_ptr doc, doc_pos_t pos)
{
    doc_pos_t        result = {-1, -1};
    str_map_iter_ptr iter;

    for (iter = str_map_iter_alloc(doc->topics);
            str_map_iter_is_valid(iter);
            str_map_iter_next(iter))
    {
        doc_region_ptr region = str_map_iter_current(iter);
        if (doc_pos_compare(pos, region->stop) >= 0)
        {
            if ( !doc_pos_is_valid(result)
              || doc_pos_compare(region->start, result) > 0)
            {
                result = region->start;
            }
        }
    }
    str_map_iter_free(iter);
    return result;
}

doc_pos_t doc_find_topic(doc_ptr doc, cptr name)
{
    doc_pos_t      result = {-1, -1};
    doc_region_ptr region = str_map_find(doc->topics, name);

    if (region)
        result = region->start;

    return result;
}

doc_pos_t doc_newline(doc_ptr doc)
{
    doc->cursor.y++;
    doc->cursor.x = 0 + doc->current_style.indent;
    return doc->cursor;
}

cptr doc_parse_tag(cptr pos, doc_tag_ptr tag)
{
    /* prepare to fail! */
    tag->type = DOC_TAG_NONE;
    tag->arg = NULL;
    tag->arg_size = 0;

    /* <name:arg> where name in {"color", "style", "topic", "link"} */
    if (*pos == '<')
    {
        doc_tag_t result = {0};
        cptr seek = pos + 1;
        char name[MAX_NLEN];
        int  ct = 0;
        for (;;)
        {
            if (!*seek || strchr(" <>\r\n\t", *seek)) return pos;
            if (*seek == ':') break;
            name[ct++] = *seek;
            if (ct >= MAX_NLEN) return pos;
            seek++;
        }
        name[ct] = '\0';

        /* [pos,seek) is the name of the tag */
        if (strcmp(name, "color") == 0)
            result.type = DOC_TAG_COLOR;
        else if (strcmp(name, "style") == 0)
            result.type = DOC_TAG_STYLE;
        else if (strcmp(name, "topic") == 0)
            result.type = DOC_TAG_TOPIC;
        else if (strcmp(name, "link") == 0)
            result.type = DOC_TAG_LINK;
        else
            return pos;

        assert(*seek == ':');
        seek++;
        result.arg = seek;

        ct = 0;
        for (;;)
        {
            if (!*seek || strchr(" <\r\n\t", *seek)) return pos;
            if (*seek == '>') break;
            ct++;
            seek++;
        }
        result.arg_size = ct;

        assert(*seek == '>');
        seek++;

        if (result.type == DOC_TAG_COLOR)
        {
            if ( result.arg_size != 1
              || !strchr("dwsorgbuDWvyRGBU", result.arg[0]) )
            {
                return pos;
            }
        }

        *tag = result;
        return seek;
    }

    return pos;
}

cptr doc_lex(cptr pos, doc_token_ptr token)
{
    if (!*pos)
    {
        token->type = DOC_TOKEN_EOF;
        token->pos = pos;
        token->size = 0;
        return pos;
    }
    if (*pos == '\r')
        pos++;

    if (*pos == '\n')
    {
        token->type = DOC_TOKEN_NEWLINE;
        token->pos = pos++;
        token->size = 1;
        return pos;
    }
    if (*pos == ' ' || *pos == '\t')
    {
        token->type = DOC_TOKEN_WHITESPACE;
        token->pos = pos++;
        token->size = 1;
        while (*pos && (*pos == ' ' || *pos == '\t'))
        {
            token->size++;
            pos++;
        }
        return pos;
    }
    if (*pos == '<')
    {
        token->type = DOC_TOKEN_TAG;
        token->pos = pos;
        token->size = 0;
        pos = doc_parse_tag(pos, &token->tag);
        if (token->tag.type != DOC_TAG_NONE)
        {
            token->size = pos - token->pos;
            return pos;
        }
    }
    token->type = DOC_TOKEN_WORD;
    token->pos = pos++;
    token->size = 1;

    while (*pos && !strchr(" <\n", *pos))
    {
        token->size++;
        pos++;
    }
    return pos;
}

doc_pos_t doc_measure(doc_ptr doc, cptr text)
{
    doc_pos_t   cursor = doc->cursor;
    doc_token_t token;
    doc_style_t style = doc->current_style;
    cptr        pos = text;
    for (;;)
    {
        pos = doc_lex(pos, &token);
        if (token.type == DOC_TOKEN_EOF)
            break;

        assert(pos != token.pos);

        if (token.type == DOC_TOKEN_TAG)
        {
            /* TODO: Update the style */
            continue;
        }

        if (token.type == DOC_TOKEN_NEWLINE)
        {
            cursor.y++;
            cursor.x = 0 + style.indent;
            continue;
        }

        if (token.type == DOC_TOKEN_WHITESPACE)
        {
            cursor.x += token.size;
            continue;
        }

        assert(token.type == DOC_TOKEN_WORD);
        if (cursor.x + token.size >= doc->width)
        {
            cursor.y++;
            cursor.x = 0 + style.indent;
        }
        cursor.x += token.size;
    }
    return cursor;
}

doc_pos_t doc_insert(doc_ptr doc, cptr text)
{
    doc_token_t token;
    cptr        pos = text;
    int         i;

    for (;;)
    {
        pos = doc_lex(pos, &token);
        if (token.type == DOC_TOKEN_EOF)
            break;

        assert(pos != token.pos);

        if (token.type == DOC_TOKEN_TAG)
        {
            /* TODO */
            continue;
        }

        if (token.type == DOC_TOKEN_NEWLINE)
        {
            doc_newline(doc);
            continue;
        }

        if (token.type == DOC_TOKEN_WHITESPACE)
        {
            doc->cursor.x += token.size;
            continue;
        }

        assert(token.type == DOC_TOKEN_WORD);
        assert(token.size > 0);

        if (doc->cursor.x + token.size >= doc->width)
        {
            /* TODO: This should be a style check. Allow the user
               to turn off wordwrapping */
            doc_newline(doc);
        }

        {
            doc_char_ptr cell = doc_char(doc, doc->cursor);
            assert(cell);
            for (i = 0; i < token.size; i++)
            {
                cell->a = TERM_WHITE; /* TODO */
                cell->c = token.pos[i];
                if (cell->c == '\t')
                    cell->c = ' ';
                doc->cursor.x++;
                if (doc->cursor.x >= doc->width)
                {
                    /* Wrap inside a word ... this is unexpected, but
                       we may be drawing a screen dump or something where
                       the words aren't really words after all.
                       TODO: Check the style to see if we should wrap here.
                       It might be best to just forget the rest of this word. */
                    doc_newline(doc);
                    cell = doc_char(doc, doc->cursor);
                }
                else
                    cell++;
            }
        }
    }
    return doc->cursor;
}

doc_char_ptr doc_char(doc_ptr doc, doc_pos_t pos)
{
    int            page_num = PAGE_NUM(pos.y);
    int            offset = PAGE_OFFSET(pos.y);
    doc_char_ptr   page = NULL;

    while (page_num >= vec_length(doc->pages))
    {
        int  cb = doc->width * PAGE_HEIGHT * sizeof(doc_char_t);

        page = malloc(cb);
        memset(page, 0, cb);
        vec_add(doc->pages, page);
    }

    page = vec_get(doc->pages, page_num);
    return page + offset * doc->width + pos.x;
}

doc_pos_t doc_read_file(doc_ptr doc, FILE *fp)
{
    string_ptr s = string_alloc();

    while (!feof(fp))
    {
        string_fgets(s, fp);
        doc_insert(doc, string_buffer(s));
        doc_newline(doc);
    }

    string_free(s);
    return doc->cursor;
}

void doc_write_file(doc_ptr doc, FILE *fp)
{
    doc_pos_t    pos;
    doc_char_ptr cell;

    for (pos.y = 0; pos.y <= doc->cursor.y; pos.y++)
    {
        int cx = doc->width;
        pos.x = 0;
        if (pos.y == doc->cursor.y)
            cx = doc->cursor.x;
        cell = doc_char(doc, pos);
        for (; pos.x < cx; pos.x++)
        {
            if (cell->c)
                fputc(cell->c, fp);
            else
                fputc(' ', fp);
            cell++;
        }
        fputc('\n', fp);
   }
}
