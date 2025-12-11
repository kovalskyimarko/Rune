#include "rune.h"

struct editorConfig E;

void error(const char* eMessage) {
    disableRawMode();
    write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
    write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
    perror(eMessage);
    exit(1);
}

void init() {
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.row = NULL;
    E.coloff = 0;
    getWindowSize(&E.screenHeight, &E.screenWidth);
}

int main() {
    init();
    enableAltBuff();
    enableRawMode();

    while (1) {
        refreshScreen();
        int c = readKey();
        processKey(c);
    }

    return 0;
}
