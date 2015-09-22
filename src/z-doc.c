#include "z-doc.h"

#include "angband.h"
#include "c-string.h"

#include <assert.h>

doc_pos_t doc_pos_create(int x, int y)
{
    doc_pos_t result;
    result.x = x;
    result.y = y;
    return result;
}

doc_pos_t doc_pos_invalid(void)
{
    return doc_pos_create(-1, -1);
}

bool doc_pos_is_valid(doc_pos_t pos)
{
    if (pos.x >= 0 && pos.y >= 0)
        return TRUE;
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


bool doc_region_is_valid(doc_region_ptr region)
{
    if (region)
    {
        if (doc_pos_compare(region->start, region->stop) <= 0)
            return TRUE;
    }
    return FALSE;
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

static void _doc_bookmark_free(vptr pv)
{
    doc_bookmark_ptr mark = pv;
    string_free(mark->name);
    free(mark);
}

static void _doc_link_free(vptr pv)
{
    doc_link_ptr link = pv;
    string_free(link->file);
    string_free(link->topic);
    free(link);
}

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
    res->bookmarks = vec_alloc(_doc_bookmark_free);
    res->links = int_map_alloc(_doc_link_free);

    /* Default Styles */
    res->current_style = *doc_style(res, "normal");
    res->current_color = res->current_style.a;
    doc_style(res, "title")->a = TERM_L_BLUE;
    doc_style(res, "heading")->a = TERM_RED;
    doc_style(res, "key_press")->a = TERM_YELLOW;
    doc_style(res, "link")->a = TERM_L_GREEN;

    return res;
}

void doc_free(doc_ptr doc)
{
    if (doc)
    {
        vec_free(doc->pages);
        str_map_free(doc->styles);
        vec_free(doc->bookmarks);
        int_map_free(doc->links);

        free(doc);
    }
}

doc_pos_t doc_cursor(doc_ptr doc)
{
    return doc->cursor;
}

doc_pos_t doc_next_bookmark(doc_ptr doc, doc_pos_t pos)
{
    int i;

    for (i = 0; i < vec_length(doc->bookmarks); i++)
    {
        doc_bookmark_ptr mark = vec_get(doc->bookmarks, i);

        if (doc_pos_compare(pos, mark->pos) < 0)
            return mark->pos;
    }
    return doc_pos_invalid();
}

doc_pos_t doc_prev_bookmark(doc_ptr doc, doc_pos_t pos)
{
    int i;
    doc_bookmark_ptr last = NULL;

    for (i = 0; i < vec_length(doc->bookmarks); i++)
    {
        doc_bookmark_ptr mark = vec_get(doc->bookmarks, i);

        if (doc_pos_compare(mark->pos, pos) < 0)
            last = mark;
        else
            break;
    }
    if (last)
        return last->pos;
    return doc_pos_invalid();
}

doc_pos_t doc_find_bookmark(doc_ptr doc, cptr name)
{
    int i;

    for (i = 0; i < vec_length(doc->bookmarks); i++)
    {
        doc_bookmark_ptr mark = vec_get(doc->bookmarks, i);

        if (strcmp(name, string_buffer(mark->name)) == 0)
            return mark->pos;
    }
    return doc_pos_invalid();
}

doc_style_ptr doc_style(doc_ptr doc, cptr name)
{
    doc_style_ptr result = str_map_find(doc->styles, name);
    if (!result)
    {
        result = malloc(sizeof(doc_style_t));
        result->a = TERM_WHITE;
        result->indent = 0;
        str_map_add(doc->styles, name, result);
    }
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
        else if (strcmp(name, "$") == 0 || strcmp(name, "var") == 0)
            result.type = DOC_TAG_VAR;
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
              || !strchr("dwsorgbuDWvyRGBU*", result.arg[0]) )
            {
                return pos;
            }
        }
        else if (!result.arg_size)
            return pos;

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
            char buf[80];
            switch (token.tag.type)
            {
            case DOC_TAG_STYLE:
                assert(token.tag.arg);
                assert(token.tag.arg_size < 80);
                memcpy(buf, token.tag.arg, token.tag.arg_size);
                buf[token.tag.arg_size] = '\0';
                style = *doc_style(doc, buf);
                break;
            }
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

static void _doc_process_var(doc_ptr doc, cptr name)
{
    if (strcmp(name, "version") == 0)
    {
        string_ptr s = string_alloc();
        string_format(s, "%d.%d.%d.", VER_MAJOR, VER_MINOR, VER_PATCH);
        doc_insert(doc, string_buffer(s));
        string_free(s);
    }
}

static void _doc_process_tag(doc_ptr doc, doc_tag_ptr tag)
{
    if (tag->type == DOC_TAG_COLOR)
    {
        assert(tag->arg);
        assert(tag->arg_size == 1);
        switch (tag->arg[0])
        {
        case '*': doc->current_color = doc->current_style.a; break;
        case 'd': doc->current_color = TERM_DARK; break;
        case 'w': doc->current_color = TERM_WHITE; break;
        case 's': doc->current_color = TERM_SLATE; break;
        case 'o': doc->current_color = TERM_ORANGE; break;
        case 'r': doc->current_color = TERM_RED; break;
        case 'g': doc->current_color = TERM_GREEN; break;
        case 'b': doc->current_color = TERM_BLUE; break;
        case 'u': doc->current_color = TERM_UMBER; break;
        case 'D': doc->current_color = TERM_L_DARK; break;
        case 'W': doc->current_color = TERM_L_WHITE; break;
        case 'v': doc->current_color = TERM_VIOLET; break;
        case 'y': doc->current_color = TERM_YELLOW; break;
        case 'R': doc->current_color = TERM_L_RED; break;
        case 'G': doc->current_color = TERM_L_GREEN; break;
        case 'B': doc->current_color = TERM_L_BLUE; break;
        case 'U': doc->current_color = TERM_L_UMBER; break;
        }
    }
    else
    {
        string_ptr arg = string_alloc();

        string_nappend(arg, tag->arg, tag->arg_size);

        switch (tag->type)
        {
        case DOC_TAG_STYLE:
            doc->current_style = *doc_style(doc, string_buffer(arg));
            doc->current_color = doc->current_style.a;
            break;
        case DOC_TAG_VAR:
            _doc_process_var(doc, string_buffer(arg));
            break;
        case DOC_TAG_TOPIC:
        {
            doc_bookmark_ptr mark = malloc(sizeof(doc_bookmark_t));
            mark->name = arg; /* steal ownership */
            arg = NULL;
            mark->pos = doc->cursor;
            vec_add(doc->bookmarks, mark);
            break;
        }
        case DOC_TAG_LINK:
        {
            doc_link_ptr link = malloc(sizeof(doc_link_t));
            cptr         pos = strchr(string_buffer(arg), '#');
            int          ch = 'a' + int_map_count(doc->links);

            if (pos)
            {
                link->file = string_alloc();
                string_nappend(link->file, tag->arg, pos - tag->arg);
                link->topic = string_alloc();
                string_nappend(link->topic, pos + 1, tag->arg_size - 1 - (pos - tag->arg));
            }
            else
            {
                link->file = arg; /* steal ownership */
                arg = NULL;
                link->topic = NULL;
            }
            int_map_add(doc->links, ch, link);
            {
                string_ptr s = string_alloc();
                string_format(s, "<style:link>[%c]<style:normal>", ch);
                doc_insert(doc, string_buffer(s));
                string_free(s);
            }
            break;
        }
        }

        string_free(arg);
    }
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
            _doc_process_tag(doc, &token.tag);
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
                cell->a = doc->current_color;
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
                       It might be best to just forget the rest of this word.
                       TODO: Fix doc_measure() for consistency.*/
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
    int            cb = doc->width * PAGE_HEIGHT * sizeof(doc_char_t);
    int            page_num = PAGE_NUM(pos.y);
    int            offset = PAGE_OFFSET(pos.y);
    doc_char_ptr   page = NULL;

    while (page_num >= vec_length(doc->pages))
    {
        page = malloc(cb);
        memset(page, 0, cb);
        vec_add(doc->pages, page);
    }

    page = vec_get(doc->pages, page_num);
    assert(offset * doc->width + pos.x < cb);
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

void doc_copy_to_term(doc_ptr doc, doc_pos_t term_pos, int row, int ct)
{
    doc_pos_t pos;
    int       i, j;

    for (i = 0; i < ct; i++)
    {
        pos.x = 0;
        pos.y = row + i;
        Term_erase(term_pos.x, term_pos.y + i, doc->width);
        if (doc_pos_compare(pos, doc->cursor) < 0)
        {
            doc_char_ptr cell = doc_char(doc, pos);
            int          cx = doc->width;

            if (pos.y == doc->cursor.y)
                cx = doc->cursor.x;

            for (j = 0; j < cx; j++)
            {
                if (cell->c)
                    Term_putch(term_pos.x + j, term_pos.y + i, cell->a, cell->c);

                cell++;
                pos.x++;
            }
        }
    }
}

#define _UNWIND 1
#define _OK 0
int doc_display_help(cptr file_name, cptr topic)
{
    int     rc = _OK;
    FILE   *fp = NULL;
    char    path[1024];
    char    caption[1024];
    doc_ptr doc = NULL;
    int     cx, cy;
    int     top = 0, page_size;
    bool    done = FALSE;

    sprintf(caption, "Help file '%s'", file_name);
    path_build(path, sizeof(path), ANGBAND_DIR_HELP, file_name);
    fp = my_fopen(path, "r");
    if (!fp)
    {
        cmsg_format(TERM_VIOLET, "Cannot open '%s'.", file_name);
        msg_print(NULL);
        return _OK;
    }

    Term_get_size(&cx, &cy);
    page_size = cy - 4;
    doc = doc_alloc(80); /* Note: We include screen dumps of 80x27, so don't go below 80 */
    doc_read_file(doc, fp);

    if (topic)
    {
        doc_pos_t pos = doc_find_bookmark(doc, topic);
        if (doc_pos_is_valid(pos))
            top = pos.y;
    }

    Term_clear();
    while (!done)
    {
        int cmd;

        prt(format("[%s, Line %d/%d]", caption, top, doc->cursor.y), 0, 0);
        if (doc->cursor.y < cy)
            prt("[Press ESC to exit.]", cy - 1, 0);
        else
            prt("[Press Return, Space, -, =, /, |, or ESC to exit.]", cy - 1, 0);
        doc_copy_to_term(doc, doc_pos_create(0, 2), top, cy - 4);

        cmd = inkey_special(TRUE);

        if ('a' <= cmd && cmd <= 'z')
        {
            doc_link_ptr link = int_map_find(doc->links, cmd);
            if (link)
            {
                rc = doc_display_help(string_buffer(link->file), string_buffer(link->topic));
                if (rc == _UNWIND)
                    done = TRUE;
                continue;
            }
        }

        switch (cmd)
        {
        case '?':
            if (strcmp(file_name, "helpinfo.txt") != 0)
            {
                rc = doc_display_help("helpinfo.txt", NULL);
                if (rc == _UNWIND)
                    done = TRUE;
            }
            break;
        case ESCAPE:
            done = TRUE;
            break;
        case 'q':
            done = TRUE;
            rc = _UNWIND;
            break;
        case SKEY_TOP:
        case '7':
            top = 0;
            break;
        case SKEY_BOTTOM:
        case '1':
            top = doc->cursor.y - page_size;
            break;
        case SKEY_PGUP:
        case '9':
            top -= page_size;
            if (top < 0) top = 0;
            break;
        case SKEY_PGDOWN:
        case '3':
            top += page_size;
            if (top > doc->cursor.y - page_size)
                top = doc->cursor.y - page_size;
            break;
        case SKEY_UP:
        case '8':
            top--;
            if (top < 0) top = 0;
            break;
        case SKEY_DOWN:
        case '2':
            top++;
            if (top > doc->cursor.y - page_size)
                top = doc->cursor.y - page_size;
            break;
        }
    }

    doc_free(doc);
    return rc;
}
