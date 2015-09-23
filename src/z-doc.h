#ifndef INCLUDED_Z_DOC_H
#define INCLUDED_Z_DOC_H

#include "h-basic.h"

#include "str-map.h"
#include "int-map.h"
#include "c-vec.h"
#include "c-string.h"

/* Utilities for Formatted Text Processing
   We support the ability to render rich text to a "virtual terminal",
   and then rapidly transfer a region of rendered text to the actual
   terminal for display. This is useful any time you want to support
   scrolling, wordwrapping, or color coded text (e.g. messages or
   the help system). We also support linked navigation, named styles and
   named bookmarks (called topics).

   The virtual terminal is a fixed width beast, but can grow its height
   dynamically. This is the way a normal "document" behaves: It grows downward
   as you type in new content.

   We support tags to control output and provide help features:
     <color:x> where x is one of the color codes:dwsorgbuDWvyRGBU*
     <style:name>
     <topic:name>
     <link:file#topic>
     <$:var> where var is a valid variable reference (e.g. <$:version> for news.txt)
*/

/* The Datatypes
   Key concepts are a position (for the cursor), a region (for transfers),
   a character (for contents and color), a style (for named formatting) and,
   of course, a document.

   Sample Usage:
    void test(int width)
    {
        doc_ptr doc = doc_alloc(width);

        doc_read_file(doc, stdin);
        doc_write_file(doc, stdout);

        doc_free(doc);
    }
*/

struct doc_pos_s
{
    int x;
    int y;
};
typedef struct doc_pos_s doc_pos_t;

doc_pos_t doc_pos_create(int x, int y);
doc_pos_t doc_pos_invalid(void);
bool      doc_pos_is_valid(doc_pos_t pos);
int       doc_pos_compare(doc_pos_t left, doc_pos_t right);

struct doc_region_s
{/* [start, stop) */
    doc_pos_t start;
    doc_pos_t stop;
};
typedef struct doc_region_s doc_region_t, *doc_region_ptr;

doc_region_t doc_region_invalid(void);
bool doc_region_is_valid(doc_region_ptr region);
bool doc_region_contains(doc_region_ptr region, doc_pos_t pos);

struct doc_char_s
{
    char c;
    byte a; /* attribute */
};
typedef struct doc_char_s doc_char_t, *doc_char_ptr;

struct doc_style_s
{
    byte color;
    int  left;
    int  right;
    int  options;
};
typedef struct doc_style_s doc_style_t, *doc_style_ptr;
enum doc_style_options_e
{
    DOC_STYLE_NO_WORDWRAP = 0x0001,
};

struct doc_bookmark_s
{
    string_ptr name;
    doc_pos_t  pos;
};
typedef struct doc_bookmark_s doc_bookmark_t, *doc_bookmark_ptr;

struct doc_link_s
{
    string_ptr file;
    string_ptr topic;
};
typedef struct doc_link_s doc_link_t, *doc_link_ptr;

struct doc_s
{
    doc_pos_t      cursor;
    doc_region_t   selection;
    int            width;
    doc_style_t    current_style;
    byte           current_color;
    vec_ptr        pages;
    str_map_ptr    styles;
    vec_ptr        bookmarks;
    int_map_ptr    links;
};
typedef struct doc_s doc_t, *doc_ptr;


/* Document API
   All text is added at the current location. The intention is that you build your
   document up front (by reading a help file, for example), and then use the
   copy_to_term() method to render to the display one screen at a time, depending
   on the current scroll position.
*/

doc_ptr       doc_alloc(int width);
void          doc_free(doc_ptr doc);

doc_pos_t     doc_cursor(doc_ptr doc);

doc_pos_t     doc_next_bookmark(doc_ptr doc, doc_pos_t pos);
doc_pos_t     doc_prev_bookmark(doc_ptr doc, doc_pos_t pos);
doc_pos_t     doc_find_bookmark(doc_ptr doc, cptr name);

doc_pos_t     doc_find_string(doc_ptr doc, cptr text, doc_pos_t start);

doc_style_ptr doc_style(doc_ptr doc, cptr name);
void          doc_change_style(doc_ptr doc, cptr name);

doc_pos_t     doc_insert(doc_ptr doc, cptr text);
doc_pos_t     doc_newline(doc_ptr doc);
doc_pos_t     doc_measure(doc_ptr doc, cptr text);

doc_pos_t     doc_read_file(doc_ptr doc, FILE *fp);
void          doc_write_file(doc_ptr doc, FILE *fp);

doc_char_ptr  doc_char(doc_ptr doc, doc_pos_t pos);
void          doc_copy_to_term(doc_ptr doc, doc_pos_t term_pos, int row, int ct);

/* Parsing */
enum doc_tag_e
{
    DOC_TAG_NONE,
    DOC_TAG_COLOR,
    DOC_TAG_STYLE,
    DOC_TAG_TOPIC,
    DOC_TAG_LINK,
    DOC_TAG_VAR,
};
struct doc_tag_s
{
    int  type;
    cptr arg;
    int  arg_size;
};
typedef struct doc_tag_s doc_tag_t, *doc_tag_ptr;

enum doc_token_e
{
    DOC_TOKEN_EOF,
    DOC_TOKEN_TAG,
    DOC_TOKEN_WHITESPACE,
    DOC_TOKEN_NEWLINE,
    DOC_TOKEN_WORD,
};
struct doc_token_s
{
    int       type;
    cptr      pos;
    int       size;
    doc_tag_t tag;
};
typedef struct doc_token_s doc_token_t, *doc_token_ptr;

cptr doc_parse_tag(cptr pos, doc_tag_ptr tag);
cptr doc_lex(cptr pos, doc_token_ptr token);


int doc_display_help(cptr file_name, cptr topic);
int doc_display(doc_ptr doc, cptr caption, int top);

#endif
