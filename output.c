#include "rune.h"

typedef struct buffer {
    char* chars;
    int len;
} buffer;

#define BUFFER_INIT {NULL, 0}

void bufferAppend(buffer *b, const char *s, int slen) {
    char *newbuf = realloc(b->chars, b->len + slen);
    if (newbuf == NULL) return;
    memcpy(newbuf + b->len, s, slen);
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
    char buf[64];
    for (int i = 0; i < E.screenHeight; i++) {
        if (E.numrows > i) {
            // Here we will render existing rows later
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
    bufferAppendRows(&b);
    bufferAppend(&b, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
    bufferAppend(&b, SHOW_CURSOR, SHOW_CURSOR_B);
    write(STDOUT_FILENO, b.chars, b.len);

    free(b.chars);
}