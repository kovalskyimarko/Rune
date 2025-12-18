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

void bufferAppendRows(buffer *b) {
    if (E.cy < E.rowoff && E.insertMode) {
        E.rowoff = E.cy;
    }

    if (E.cy >= E.rowoff + E.screenHeight && E.insertMode) {
        E.rowoff = E.cy - E.screenHeight + 1;
    }

    if (E.cx < E.coloff) {
        E.coloff = E.cx;
    }

    if (E.cx >= E.coloff + E.screenWidth) {
       E.coloff = E.cx - E.screenWidth + 1;
    }

    char buf[128];
    for (int i = E.rowoff; i < E.screenHeight+E.rowoff; i++) {
        if (E.numrows > i) {
            erow *row = &E.row[i];
            int len = row->len;

            if (E.coloff < len && E.rowoff < E.numrows) {
                int visible = len - E.coloff;
                if (visible > E.screenWidth) visible = E.screenWidth;

                bufferAppend(b, &row->chars[E.coloff], visible);
            }
        }
        else {
            if (E.numrows == 0 && i == E.screenHeight / 2) {
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

    
    if (E.insertMode) {
        bufferAppend(b, "\x1b[7m", 4);    
        char status[80];
        int len = snprintf(status, sizeof(status), " %.20s - %d lines | Ln %d, Col %d",
            E.filename ? E.filename : "[No Name]", 
            E.numrows, 
            E.cy + 1, 
            E.cx + 1);

        if (len > E.screenWidth) len = E.screenWidth;
        bufferAppend(b, status, len);

        while (len < E.screenWidth) {
            bufferAppend(b, " ", 1);
            len++;
        }

        bufferAppend(b, "\x1b[m", 3);
    }

    else {
        bufferAppend(b, CLEAR_LINE, CLEAR_LINE_B);
        bufferAppend(b, E.lastrow->chars, E.lastrow->len);
    }
}

void refreshScreen() {
    buffer b = BUFFER_INIT;

    bufferAppend(&b, HIDE_CURSOR, HIDE_CURSOR_B);
    bufferAppend(&b, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
    bufferAppendRows(&b);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, E.cx - E.coloff + 1);
    bufferAppend(&b, buf, strlen(buf));
    bufferAppend(&b, SHOW_CURSOR, SHOW_CURSOR_B);
    write(STDOUT_FILENO, b.chars, b.len);

    free(b.chars);
}