#include "angband.h"

#include "z-doc.h"

#include <assert.h>

msg_ptr _msg_alloc(cptr s)
{
    msg_ptr m = malloc(sizeof(msg_t));
    m->msg = string_alloc(s);
    m->turn = 0;
    m->count = 1;
    return m;
}

void _msg_free(msg_ptr m)
{
    if (m)
    {
        string_free(m->msg);
        free(m);
    }
}

static int _msg_max = 2048;
static int _msg_count = 0;
static msg_ptr *_msgs = NULL;
static int _msg_head = 0;
static bool _msg_append = FALSE;

void msg_on_startup(void)
{
    int cb = _msg_max * sizeof(msg_ptr);
    _msgs = malloc(cb);
    memset(_msgs, 0, cb);
    _msg_count = 0;
    _msg_head = 0;
    _msg_append = FALSE;
}

void msg_on_shutdown(void)
{
    int i;
    for (i = 0; i < _msg_max; i++)
        _msg_free(_msgs[i]);
    free(_msgs);
}

static int _msg_index(int age)
{
    assert(0 <= age && age < _msg_max);
    return (_msg_head + _msg_max - (age + 1)) % _msg_max;
}

int msg_count(void)
{
    return _msg_count;
}

msg_ptr msg_get(int age)
{
    int i = _msg_index(age);
    return _msgs[i];
}

int msg_get_plain_text(int age, char *buffer, int max)
{
    int         ct = 0, i;
    doc_token_t token;
    msg_ptr     msg = msg_get(age);
    cptr        pos = string_buffer(msg->msg);
    bool        done = FALSE;

    while (!done)
    {
        pos = doc_lex(pos, &token);
        if (token.type == DOC_TOKEN_EOF) break;
        if (token.type == DOC_TOKEN_TAG) continue; /* assume only color tags */
        for (i = 0; i < token.size; i++)
        {
            if (ct >= max - 4)
            {
                buffer[ct++] = '.';
                buffer[ct++] = '.';
                buffer[ct++] = '.';
                done = TRUE;
                break;
            }
            buffer[ct++] = token.pos[i];
        }
    }
    buffer[ct] = '\0';
    return ct;
}

void msg_add(cptr str)
{
    cmsg_add(TERM_WHITE, str);
}

void cmsg_append(byte color, cptr str)
{
    if (!_msg_count)
        cmsg_add(color, str);
    else
    {
        msg_ptr m = msg_get(0);

        assert(m);
        if (string_length(m->msg))
            string_append_char(m->msg, ' ');
        if (color != TERM_WHITE)
            string_printf(m->msg, "<color:%c>%s<color:*>", attr_to_attr_char(color), str);
        else
            string_append(m->msg, str);
    }
}

void _cmsg_add_aux(byte color, cptr str, int turn)
{
    msg_ptr m;

    /* Repeat last message? */
    if (_msg_count)
    {
        m = msg_get(0);
        if (strcmp(string_buffer(m->msg), str) == 0)
        {
            m->count++;
            m->turn = turn;
            return;
        }
    }

    m = _msgs[_msg_head];
    if (!m)
    {
        m = _msg_alloc(NULL);
        _msgs[_msg_head] = m;
        _msg_count++;
    }
    else
    {
        string_clear(m->msg);
        string_shrink(m->msg, 128);
    }
    if (color != TERM_WHITE)
        string_printf(m->msg, "<color:%c>%s<color:*>", attr_to_attr_char(color), str);
    else
        string_append(m->msg, str);

    m->turn = turn;
    m->count = 1;

    _msg_head = (_msg_head + 1) % _msg_max;
}

void cmsg_add(byte color, cptr str)
{
    _cmsg_add_aux(color, str, player_turn);
}

int msg_display_len(cptr msg)
{
    int         ct = 0;
    doc_token_t token;
    cptr        pos = msg;

    if (!msg)
        return 0;

    for (;;)
    {
        pos = doc_lex(pos, &token);
        if (token.type == DOC_TOKEN_EOF) break;
        if (token.type == DOC_TOKEN_TAG) continue; /* assume only color tags */
        if (token.type == DOC_TOKEN_NEWLINE) continue; /* unexpected */
        ct += token.size;
    }
    return ct;
}

/* State of the "Message Line":
   We have currently reserved (min_row, min_col, max_row, max_col)
   We can reserve up to (min_row, min_col, max_max_row, max_col)
   In other words, the message region expands dynamically, as needed,
   until cleared with the option to close the "drop down".

   msg_max_col gets reset in cmsg_print to respect the current terminal width.
*/
static int msg_min_row = 0;
static int msg_min_col = 13;

static int msg_max_row = 0;
static int msg_max_col = 100;

static int msg_max_max_row = 10;

static int msg_current_row = 0;  /* = msg_min_row; */
static int msg_current_col = 13; /* = msg_min_col; */

static const char * msg_more_prompt = "-more-";

rect_t msg_line_rect(void)
{
    return rect_create(
        msg_min_col,
        msg_min_row,
        msg_max_col - msg_min_col + 1,
        msg_max_row - msg_min_row + 1
    );
}

void msg_line_init(const rect_t *display_rect)
{
    if (display_rect && rect_is_valid(display_rect))
    {
        msg_line_clear(TRUE);
        msg_min_row = display_rect->y;
        msg_min_col = display_rect->x;
        msg_max_row = msg_min_row;
        msg_max_max_row = msg_min_row + display_rect->cy - 1;
        msg_max_col = msg_min_col + display_rect->cx - 1;

        msg_current_row = msg_min_row;
        msg_current_col = msg_min_col;
    }
    else
    {
        rect_t r = rect_create(13, 0, 80, 10);
        msg_line_init(&r);
    }
}

void msg_boundary(void)
{
    _msg_append = FALSE;
    /* Force a line break (display only) */
    if (msg_current_col > msg_min_col)
    {
        if (msg_current_row == msg_max_max_row)
            msg_current_col = msg_max_col - strlen(msg_more_prompt);
        else
            msg_current_col = msg_max_col + 1;
    }

    if (auto_more_state == AUTO_MORE_SKIP_BLOCK)
    {
        /* Flush before updating the state to skip
           the remnants of the last message from the
           previous message block */
        msg_print(NULL);
        auto_more_state = AUTO_MORE_PROMPT;
    }
}

bool msg_line_contains(int row, int col)
{
    rect_t r = msg_line_rect();
    if (col < 0)
        col = r.x;
    if (row < 0)
        row = r.y;
    return rect_contains_pt(&r, col, row);
}

bool msg_line_is_empty(void)
{
    if (msg_current_row == msg_min_row && msg_current_col == msg_min_col)
        return TRUE;
    return FALSE;
}

void msg_line_clear(bool close_drop_down)
{
    int i;
    for (i = msg_min_row; i <= msg_max_row; i++)
        Term_erase(msg_min_col, i, msg_max_col - msg_min_col + 1);

    if (close_drop_down)
    {
        if (msg_max_row)
        {
            /* Note: We need not redraw the entire map if this proves too slow */
            p_ptr->redraw |= PR_MAP;
            if (msg_min_col <= 12)
                p_ptr->redraw |= PR_BASIC | PR_EQUIPPY;
        }
        msg_max_row = msg_min_row;
    }
    msg_current_row = msg_min_row;
    msg_current_col = msg_min_col;
}

static void msg_flush(void)
{
    if (auto_more_state == AUTO_MORE_PROMPT)
    {
        /* Pause for response */
        Term_putstr(msg_current_col, msg_current_row, -1, TERM_L_BLUE, msg_more_prompt);

        /* Get an acceptable keypress */
        while (1)
        {
            int cmd = inkey();
            if (cmd == ESCAPE)
            {
                auto_more_state = AUTO_MORE_SKIP_ALL;
                break;
            }
            else if (cmd == '\n' || cmd == '\r' || cmd == 'n')
            {
                auto_more_state = AUTO_MORE_SKIP_BLOCK;
                break;
            }
            else if (cmd == '?')
            {
                screen_save_aux();
                show_file(TRUE, "context_more_prompt.txt", NULL, 0, 0);
                screen_load_aux();
                continue;
            }
            else if (cmd == ' ' || cmd == 'm' || quick_messages)
            {
                break;
            }
            bell();
        }
    }

    msg_line_clear(TRUE);
}


/* Display another message on the "Message Line" */
static void msg_line_display(byte color, cptr msg)
{
    cptr pos = msg, seek;
    byte base_color = color;
    int len, xtra;

    while (*pos)
    {
        if (*pos == ' ')
        {
            while (*pos && *pos == ' ')
            {
                Term_putch(msg_current_col++, msg_current_row, color, ' ');
                pos++;
            }
            continue;
        }
        else if (*pos == '<') /* Perhaps a tag? */
        {
            doc_tag_t tag = {0};

            pos = doc_parse_tag(pos, &tag);
            if (tag.type != DOC_TAG_NONE)
            {
                if (tag.type == DOC_TAG_COLOR)
                {
                    assert(tag.arg);
                    if (tag.arg[0] == '*')
                        color = base_color;
                    else
                        color = color_char_to_attr(tag.arg[0]);
                }
                continue;
            }
            /* Perhaps not! Draw it normally! */
        }

        /* Get current word: [pos, seek) */
        seek = pos;
        switch (*seek)
        {
        case '<':
        case '\n':
            seek++;
            break;
        default:
            while (*seek && !strchr(" <\n", *seek))
                seek++;
        }

        /* Measure the current word and check for a line break */
        len = seek - pos;
        if (!len)
            break; /* oops */

        if (0 /*_punctuation_hack(seek)*/)
            xtra = 1;
        else if (msg_current_row == msg_max_max_row && !*seek)
            xtra += strlen(msg_more_prompt);
        else
            xtra = 0;

        if (*pos == '\n' || msg_current_col + len + xtra > msg_max_col)
        {
            if (msg_current_row >= msg_max_max_row)
                msg_flush();
            else
            {
                Term_erase(msg_current_col, msg_current_row, msg_max_col - msg_current_col + 1);
                msg_current_row++;
                if (msg_current_row > msg_max_row)
                    msg_max_row = msg_current_row;
                msg_current_col = msg_min_col;
                Term_erase(msg_current_col, msg_current_row, msg_max_col - msg_current_col + 1);
            }
            if (*pos == '\n')
            {
                pos++;
                continue;
            }
        }

        /* Draw the current word */
        Term_putstr(msg_current_col, msg_current_row, len, color, pos);
        msg_current_col += len;

        pos = seek;
    }

    /* Add a space to separate consecutive messages */
    if (msg_current_col > msg_min_col)
        Term_putch(msg_current_col++, msg_current_row, color, ' ');
}

/* Prompt the user for a choice:
   [1] keys[0] is the default choice if quick_messages and PROMPT_FORCE_CHOICE is not set.
   [2] keys[0] is returned on ESC if PROMPT_ESCAPE_DEFAULT is set.
   [3] You get back the char in the keys prompt, not the actual character pressed.
       This makes a difference if PROMPT_CASE_SENSITIVE is not set (and simplifies
       your coding).

   Sample Usage:
   char ch = cmsg_prompt(TERM_VIOLET, "Really commit suicide? [Y,n]", "nY", PROMPT_NEW_LINE | PROMPT_CASE_SENSITIVE);
   if (ch == 'Y') {...}
*/
static char cmsg_prompt_imp(byte color, cptr prompt, char keys[], int options)
{
    if (options & PROMPT_NEW_LINE)
        msg_boundary();

    /* TODO: Make sure the entire prompt will fit. -more- prompts are very
       rare these days, so I'm lazily omitting this check */
    auto_more_state = AUTO_MORE_PROMPT;
    cmsg_print(color, prompt);

    for (;;)
    {
        char ch = inkey();
        int  i;

        if (ch == ESCAPE && (options & PROMPT_ESCAPE_DEFAULT))
            return keys[0];

        for (i = 0; ; i++)
        {
            char choice = keys[i];
            if (!choice) break;
            if (ch == choice) return choice;
            if (!(options & PROMPT_CASE_SENSITIVE))
            {
                if (tolower(ch) == tolower(choice)) return choice;
            }
        }

        if (!(options & PROMPT_FORCE_CHOICE) && quick_messages)
            return keys[0];
    }
}

char cmsg_prompt(byte color, cptr prompt, char keys[], int options)
{
    char ch = cmsg_prompt_imp(color, prompt, keys, options);
    msg_line_clear(TRUE);
    return ch;
}

char msg_prompt(cptr prompt, char keys[], int options)
{
    return cmsg_prompt(TERM_WHITE, prompt, keys, options);
}

/* Add a new message to the message 'line', and memorize this
   message in the message history. */
void cmsg_print(byte color, cptr msg)
{
    int n;

    msg_max_col = MIN(msg_min_col + 80, Term->wid -1);

    if (world_monster) return;
    if (statistics_hack) return;

    /* Flush when requested or needed */
    if (!msg)
    {
        if (!msg_line_is_empty())
            msg_flush();
        return;
    }

    n = msg_display_len(msg);
    if (msg_current_row == msg_max_max_row && msg_current_col + n + (int)strlen(" -more-") > msg_max_col)
    {
        msg_flush();
    }

    /* Paranoia */
    if (n > 1000) return;

    /* Memorize the message */
    if (character_generated)
    {
        if (_msg_append && _msg_count)
            cmsg_append(color, msg);
        else
            cmsg_add(color, msg);
    }

    msg_line_display(color, msg);

    if (auto_more_state == AUTO_MORE_SKIP_ONE)
        auto_more_state = AUTO_MORE_PROMPT;

    /* Window stuff */
    p_ptr->window |= (PW_MESSAGE);
    window_stuff();

    /* Optional refresh */
    if (fresh_message) Term_fresh();

    _msg_append = TRUE;
}

void msg_print(cptr msg)
{
    cmsg_print(TERM_WHITE, msg);
}

/*
 * Display a formatted message, using "vstrnfmt()" and "msg_print()".
 */
void msg_format(cptr fmt, ...)
{
    va_list vp;

    char buf[1024];

    /* Begin the Varargs Stuff */
    va_start(vp, fmt);

    /* Format the args, save the length */
    (void)vstrnfmt(buf, 1024, fmt, vp);

    /* End the Varargs Stuff */
    va_end(vp);

    /* Display */
    cmsg_print(TERM_WHITE, buf);
}

void cmsg_format(byte color, cptr fmt, ...)
{
    va_list vp;

    char buf[1024];

    /* Begin the Varargs Stuff */
    va_start(vp, fmt);

    /* Format the args, save the length */
    (void)vstrnfmt(buf, 1024, fmt, vp);

    /* End the Varargs Stuff */
    va_end(vp);

    /* Display */
    cmsg_print(color, buf);
}

void msg_on_load(savefile_ptr file)
{
    int i;
    char buf[1024];
    int count = savefile_read_s16b(file);

    for (i = 0; i < count; i++)
    {
        s32b turn = 0;
        if (!savefile_is_older_than(file, 4, 0, 0, 1) && savefile_is_older_than(file, 4, 0, 0, 3))
            savefile_read_byte(file);
        savefile_read_string(file, buf, sizeof(buf));
        if (!savefile_is_older_than(file, 4, 0, 0, 2))
        {
            turn = savefile_read_s32b(file);
            if (savefile_is_older_than(file, 4, 0, 0, 3))
                turn = 0; /* used to be game_turn, not player_turn! */
        }
        _cmsg_add_aux(TERM_WHITE, buf, turn);
    }
}

void msg_on_save(savefile_ptr file)
{
    int i;
    int count = msg_count();
    if (compress_savefile && count > 40) count = 40;

    savefile_write_u16b(file, count);
    for (i = count - 1; i >= 0; i--)
    {
        msg_ptr m = msg_get(i);
        savefile_write_string(file, string_buffer(m->msg));
        savefile_write_s32b(file, m->turn);
    }
}


