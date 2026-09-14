#include "rune.h"

struct editorConfig E;

void error(const char* eMessage) {
    disableAltBuff();
    disableRawMode();
    write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
    write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
    perror(eMessage);
    exit(1);
}

void init(void) {
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.row = NULL;
    E.coloff = 0;
    E.rowoff = 0;
    E.filename = NULL;
    E.filepath = NULL;
    E.mode = INSERT_MODE;
    getWindowSize(&E.screenHeight, &E.screenWidth);
    E.screenHeight-=1; // For the status bar
    E.lastrow = malloc(sizeof(erow));
    E.lastrow->chars = malloc(2048);
    E.lastrow->chars[0] = '\0';
    E.lastrow->len = 0;
    E.statusmsg[0] = '\0';
    E.yankbuff = NULL;
    E.showLineNumbers = false;
    E.showRLineNumbers = false;
}

void cleanup(void) {
    if (E.row) {
        for (int i = 0; i < E.numrows; i++) {
            if (E.row[i].chars) {
                free(E.row[i].chars);
                E.row[i].chars = NULL;
            }
        }
        free(E.row);
        E.row = NULL;
    }

    if (E.lastrow) {
        if (E.lastrow->chars) {
            free(E.lastrow->chars);
            E.lastrow->chars = NULL;
        }
        free(E.lastrow);
        E.lastrow = NULL;
    }

    if (E.filepath) { free(E.filepath); E.filepath = NULL;}
    if (E.filename) { free(E.filename); E.filename = NULL;}
    if (E.yankbuff) { free(E.yankbuff); E.yankbuff = NULL;}
}

int main(void) {
    init();
    enableAltBuff();
    enableRawMode();

    atexit(disableAltBuff);
    atexit(cleanup);

    while (1) {
        refreshScreen();
        int c = readKey();
        processKey(c);
    }

    return 0;
}
