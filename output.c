#include "rune.h"

typedef struct buffer {
    char* chars;
    int len;
} buffer;

#define BUFFER_INIT {NULL, 0}

void bufferAppend(buffer *b, const char *s, int slen) {
    char *newbuf = realloc(b->chars, b->len + slen);
    if (newbuf == NULL) return;
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
    if (E.cx < E.coloff) {
        E.coloff = E.cx;
    }

    if (E.cx >= E.coloff + E.screenWidth) {
       E.coloff = E.cx - E.screenWidth + 1;
    }

    char buf[64];
    for (int i = 0; i < E.screenHeight; i++) {
        if (E.numrows > i) {
            erow *row = &E.row[i];
            int len = row->len;

            if (E.coloff < len) {
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
        if (i != E.screenHeight -1) {
            bufferAppend(b, "\r\n", 2);
        }
    }
}

void refreshScreen() {
    buffer b = BUFFER_INIT;

    bufferAppend(&b, HIDE_CURSOR, HIDE_CURSOR_B);
    bufferAppend(&b, CLEAR_SCREEN, CLEAR_SCREEN_B);
    bufferAppend(&b, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
    bufferAppendRows(&b);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx - E.coloff + 1);
    bufferAppend(&b, buf, strlen(buf));
    bufferAppend(&b, SHOW_CURSOR, SHOW_CURSOR_B);
    //char bufT[32];
    //snprintf(bufT, sizeof(bufT), "%d", E.cx);
    //bufferAppend(&b, bufT, 1);
    write(STDOUT_FILENO, b.chars, b.len);

    free(b.chars);
}