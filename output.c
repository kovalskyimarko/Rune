#include "rune.h"

typedef struct buffer {
    char* chars;
    int len;
} buffer;

#define BUFFER_INIT {NULL, 0}

void bufferAppend(buffer *b, const char *s, int slen) {
    char *newbuf = realloc(b->chars, b->len + slen);
    if (!newbuf) return;
    memcpy(&(newbuf[b->len]), s, slen);
    b->chars = newbuf;
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

    if (E.cx < E.coloff) {
        E.coloff = E.cx;
    }

    if (E.cx >= E.coloff + E.screenWidth) {
       E.coloff = E.cx - E.screenWidth + 1;
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

void bufferAppendRows(buffer *b) {
    if (E.mode != COMMANDLINE_MODE)
    {
        scroll();
    }

    else
    {
        scrollAtCommandLine();
    }

    char buf[128];
    for (int i = E.rowoff; i < E.screenHeight+E.rowoff; i++) {
        int numWidth = 0;

        if ((E.showLineNumbers || E.showRLineNumbers) && (E.numrows > i || (E.numrows == 0 && i == 0)))
        {
            numWidth = 5;
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

            int nlen = snprintf(num_buf, sizeof(num_buf), "\x1b[90m%4d \x1b[m", num);
            bufferAppend(b, num_buf, nlen);
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

                    if (in_hl && !hl_active) {
                        bufferAppend(b, "\x1b[7m", 4);
                        hl_active = true;
                    }

                    else if (!in_hl && hl_active) {
                        bufferAppend(b, "\x1b[m", 3);
                        hl_active = false;
                    }

                    bufferAppend(b, &row->chars[cx], 1);
                }

                if (hl_active) {
                    bufferAppend(b, "\x1b[m", 3);
                }

            }
        }
        else {
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
                bufferAppend(b, "~", 1);
            }
        }

        bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);
        bufferAppend(b, "\r\n", 2);
    }

    if (E.mode == COMMANDLINE_MODE)
    {
        bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);

        if (E.lastrow && E.lastrow->chars) {
            int cmd_len = E.lastrow->len - E.commandlineColloff;

            if (cmd_len > E.screenWidth)
                cmd_len = E.screenWidth;

            if (cmd_len < 0) cmd_len = 0;

            bufferAppend(b, &E.lastrow->chars[E.commandlineColloff], cmd_len);
        }
    }

    else if (strcmp(E.statusmsg, "") != 0)
    {
        bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);
        bufferAppend(b, "\x1b[92m", 5);

        int msglen = strlen(E.statusmsg);
        if (msglen > E.screenWidth) msglen = E.screenWidth;
        bufferAppend(b, E.statusmsg, msglen);
        bufferAppend(b, "\x1b[m", 3); 
    }
    
    else {
        bufferAppend(b, "\x1b[7m", 4);    
        char status[80];
        int len = snprintf(status, sizeof(status), " %.20s - %d lines | Ln %d, Col %d",
            E.filename ? E.filename : "[No Name]", 
            E.numrows, 
            E.cy + 1, 
            E.cx + 1
        );

        if (len > E.screenWidth) len = E.screenWidth;
        bufferAppend(b, status, len);

        char *mode_str;

        if (E.mode == NORMAL_MODE)
        {
            mode_str = "--NORMAL--";
        }

        else if (E.mode == INSERT_MODE)
        {
            mode_str = "--INSERT--";
        }

        else
        {
            mode_str = "--VISUAL--";
        }

        int rlen = strlen(mode_str);
        while (len < E.screenWidth) {
            if (E.screenWidth - len == rlen)
            {
                bufferAppend(b, mode_str, rlen);
                break;
            }

            else 
            {
                bufferAppend(b, " ", 1);
                len++;
            }
        }

        bufferAppend(b, "\x1b[m", 3);
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
