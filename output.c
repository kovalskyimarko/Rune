#include "rune.h"

typedef struct buffer {
    char* chars;
    int len;
    int capacity;
} buffer;

#define BUFFER_INIT {NULL, 0, 0}

void bufferAppend(buffer *b, const char *s, int slen) {
    if (b->len + slen > b->capacity) {
        int newCap = (b->capacity == 0) ? 4096 : (b->capacity*2) + slen;

        if (b->len + slen > newCap) {
           newCap = b->len + slen;
        }

        b->chars = xrealloc(b->chars, newCap);

        b->capacity = newCap;
    }

    memcpy(&(b->chars[b->len]), s, slen);
    b->len += slen;
}

void appendCentered(buffer *b, const char *s) {
    int slen = strlen(s);
    int space = (E.screenWidth - slen) / 2;
    if (space < 0) space = 0;
    for (int i = 0; i < space; i++) bufferAppend(b, " ", 1);
    bufferAppend(b, s, slen);
}

void scroll(void)
{
    if (E.mode == COMMANDLINE_MODE)
    {
        return;
    }

    int numWidth = (E.showLineNumbers || E.showRLineNumbers) ? 5 : 0;

    if (E.cx < E.coloff) {
        E.coloff = E.cx;
    }

    if (E.cx >= E.coloff + E.screenWidth) {
       E.coloff = E.cx - E.screenWidth + 1 + numWidth;
    }

    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }

    if (E.cy >= E.rowoff + E.screenHeight) {
        E.rowoff = E.cy - E.screenHeight + 1;
    }
}

void scrollAtCommandLine(void)
{
    if (E.cx < E.commandlineColloff) {
        E.commandlineColloff = E.cx;
    }

    if (E.cx >= E.commandlineColloff + E.screenWidth) {
       E.commandlineColloff = E.cx - E.screenWidth + 1;
    }
}

void drawLineNumber(buffer *b, ThemePalette *currentTheme, int i)
{
    bufferAppend(b, currentTheme->hl_default, strlen(currentTheme->hl_default));
    bufferAppend(b, currentTheme->bg_color, strlen(currentTheme->bg_color));

    char num_buf[32];
    int num = 0;

    if (E.showLineNumbers && E.showRLineNumbers) {
        if (i == E.cy) {
            num = i + 1;
        } else
        {
            num = i - E.cy;
            if (num < 0) num = -num;
        }
    }

    else if (E.showRLineNumbers) {
        num = i - E.cy;
        if (num < 0) num = -num;
    
    } 

    else {
        num = i + 1;
    }

    int nlen = snprintf(num_buf, sizeof(num_buf), "%4d ", num);
    bufferAppend(b, num_buf, nlen);
}

void drawTildasAndGreeting(buffer *b, ThemePalette *currentTheme, int i)
{
    char buf[128];

    bufferAppend(b, currentTheme->hl_default, strlen(currentTheme->hl_default));
    bufferAppend(b, currentTheme->bg_color, strlen(currentTheme->bg_color));

    // The first line should never have ~
    if (E.numrows == 0 && i == 0) {
    }

    else if (E.numrows == 0 && i == E.screenHeight / 2) {
        appendCentered(b, "Rune - terminal based editor");
    }

    else if(E.numrows == 0 && i == E.screenHeight /2 + 1) {
        snprintf(buf, sizeof(buf), "Version %s", EDITOR_VERSION);
        appendCentered(b,buf);
    }

    else {
        bufferAppend(b, currentTheme->hl_default, strlen(currentTheme->hl_default));
        bufferAppend(b, "~", 1);
    }
}

void drawCommandLine(buffer *b, ThemePalette *currentTheme)
{
    bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);
    bufferAppend(b, currentTheme->bg_color, strlen(currentTheme->bg_color));
    bufferAppend(b, currentTheme->hl_default, strlen(currentTheme->hl_default));

    if (E.lastrow && E.lastrow->chars) {
        int cmd_len = E.lastrow->len - E.commandlineColloff;

        if (cmd_len > E.screenWidth)
            cmd_len = E.screenWidth;

        if (cmd_len < 0) cmd_len = 0;

        bufferAppend(b, &E.lastrow->chars[E.commandlineColloff], cmd_len);
    }
}

void drawCommandLineMsg(buffer *b, ThemePalette *currentTheme)
{
    bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);
    bufferAppend(b, currentTheme->message_fg, strlen(currentTheme->message_fg));
    bufferAppend(b, currentTheme->bg_color, strlen(currentTheme->bg_color));


    int msglen = strlen(E.statusmsg);
    if (msglen > E.screenWidth) msglen = E.screenWidth;
    bufferAppend(b, E.statusmsg, msglen);
}

void drawStatusMsg(buffer *b, ThemePalette *currentTheme)
{
    bufferAppend(b, currentTheme->status_bg, strlen(currentTheme->status_bg));
    bufferAppend(b, currentTheme->status_fg, strlen(currentTheme->status_fg));
    
    bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);

    char left_str[128];
    char right_str[128];
    int left_len, right_len;

    char *mode_str;
    if (E.mode == NORMAL_MODE)
        mode_str = "--NORMAL--";
    else if (E.mode == INSERT_MODE)
        mode_str = "--INSERT--";
    else
        mode_str = "--VISUAL--";

    left_len = snprintf(left_str, sizeof(left_str), " %.20s%s",
        E.filename ? E.filename : "[No Name]",
        E.dirty > 0 ? " [+]" : "    "
    );

    if (E.statusSize == STATUS_MINIMAL)
    {
        right_len = 0;
        right_str[0] = '\0';
    }
    else
    {
        right_len = snprintf(right_str, sizeof(right_str), "%d lines | Ln %d, Col %d   %s ",
            E.numrows,
            E.cy + 1,
            E.cx + 1,
            mode_str
        );
    }

    if (left_len > E.screenWidth) left_len = E.screenWidth;
    bufferAppend(b, left_str, left_len);

    if (E.statusSize != STATUS_MINIMAL)
    {
        int spaces = E.screenWidth - left_len - right_len;

        while (spaces > 0)
        {
            bufferAppend(b, " ", 1);
            spaces--;
        }

        if (left_len + right_len > E.screenWidth) {
            right_len = E.screenWidth - left_len;
            if (right_len < 0) right_len = 0;
        }

        if (right_len > 0) {
            bufferAppend(b, right_str, right_len);
        }
    }
}

void bufferAppendRows(buffer *b) {
    if (E.mode != COMMANDLINE_MODE)
    {
        scroll();
    }

    else
    {
        scrollAtCommandLine();
    }

    ThemePalette *currentTheme = &themes[E.theme];
    int lastHl = -1;
    
    bufferAppend(b, currentTheme->hl_default, strlen(currentTheme->hl_default));
    bufferAppend(b, currentTheme->bg_color, strlen(currentTheme->bg_color));

    bufferAppend(b, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);

    for (int i = E.rowoff; i < E.screenHeight+E.rowoff; i++) {
        int numWidth = 0;

        if ((E.showLineNumbers || E.showRLineNumbers) && (E.numrows > i || (E.numrows == 0 && i == 0)))
        {
            drawLineNumber(b, currentTheme, i);
            numWidth = 5;
        }

        if (E.numrows > i) {

            erow *row = &E.row[i];
            int len = row->len;

            if (E.coloff < len && E.rowoff < E.numrows) {
                int visible = len - E.coloff;
                if (visible > ( E.screenWidth - numWidth )) visible = E.screenWidth - numWidth;

                int sy = E.vStartcy, sx = E.vStartcx;
                int ey = E.cy, ex = E.cx;

                if (sy > ey) {
                    sy = E.cy; ey = E.vStartcy;
                    sx = E.cx; ex = E.vStartcx;
                } else if (sy == ey && sx > ex) {
                    sx = E.cx; ex = E.vStartcx;
                }

                bool hl_active = false;

                for (int j = 0; j < visible; j++)
                {
                    int cx = E.coloff + j;
                    int cy = i;

                    bool in_hl = false;

                    if (E.mode == VISUAL_MODE) {
                        if (cy > sy && cy < ey) in_hl = true;                                 // Between rows
                        else if (sy == ey && cy == sy && cx >= sx && cx <= ex) in_hl = true;  // On one row
                        else if (cy == sy && cy < ey && cx >= sx) in_hl = true;               // First row
                        else if (cy == ey && cy > sy && cx <= ex) in_hl = true;               // Last row
                    }

                    if (in_hl) {
                        if (!hl_active) {
                            bufferAppend(b, REVERSE_COLORS, REVERSE_COLORS_B);
                            hl_active = true;
                        }
                        bufferAppend(b, &row->chars[cx], 1);
                    }

                    else {
                        if (hl_active) 
                        {
                            bufferAppend(b, OFF_REVERSE_COLOR, OFF_REVERSE_COLOR_B);
                            hl_active = false;
                        }

                        int current_hl = (row->hl) ? row->hl[cx] : HL_DEFAULT;

                        if (current_hl != lastHl)
                        {
                            const char* color_code = NULL;

                            switch (current_hl) {
                                case HL_DEFAULT:   color_code = currentTheme->hl_default; break;
                                case HL_STRING:    color_code = currentTheme->hl_string; break;
                                case HL_NUMBER:    color_code = currentTheme->hl_number; break;
                                case HL_TYPE:      color_code = currentTheme->hl_type; break;
                                case HL_STRUCT:    color_code = currentTheme->hl_struct; break;
                                case HL_CONTROL:   color_code = currentTheme->hl_control; break;
                                case HL_STORAGE:   color_code = currentTheme->hl_storage; break;
                                case HL_QUALIFIER: color_code = currentTheme->hl_qualifier; break;
                                case HL_SPECIAL:   color_code = currentTheme->hl_special; break;
                                case HL_BRACKETS:  color_code = currentTheme->hl_brackets; break;
                                case HL_COMMENT:   color_code = currentTheme->hl_comment; break;
                            }

                            if (color_code) {
                                bufferAppend(b, color_code, strlen(color_code));
                            }
                        }

                        lastHl = current_hl;

                        bufferAppend(b, &row->chars[cx], 1);
                    }
                }

                if (hl_active) {
                    bufferAppend(b, OFF_REVERSE_COLOR, OFF_REVERSE_COLOR_B);
                }

            }
        }
        else {
            drawTildasAndGreeting(b, currentTheme, i);
        }

        bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);
        bufferAppend(b, "\r\n", 2);
    }

    if (E.mode == COMMANDLINE_MODE)
    {
        drawCommandLine(b, currentTheme);
    }

    else if (strcmp(E.statusmsg, "") != 0)
    {
        drawCommandLineMsg(b, currentTheme);
    }
    
    else if (E.statusSize != STATUS_OFF) {
        drawStatusMsg(b, currentTheme);
    }
}

void refreshScreen(void) {
    buffer b = BUFFER_INIT;

    bufferAppend(&b, HIDE_CURSOR, HIDE_CURSOR_B);
    bufferAppend(&b, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);

    bufferAppendRows(&b);

    char buf[32];

    int posY;
    int posX;

    if (E.mode != COMMANDLINE_MODE)
    {
        posY = E.cy - E.rowoff + 1;
        int numwidth = (E.showLineNumbers || E.showRLineNumbers) ? 5 : 0;
        posX = E.cx - E.coloff + numwidth + 1;
    }

    else
    {
        posY = E.screenHeight + 1;
        posX = E.cx - E.commandlineColloff + 1;
    }

    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", posY, posX);
    bufferAppend(&b, buf, strlen(buf));
    bufferAppend(&b, SHOW_CURSOR, SHOW_CURSOR_B);
    write(STDOUT_FILENO, b.chars, b.len);

    free(b.chars);
}
