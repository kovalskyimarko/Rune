#include "rune.h"

int readKey() {
    char c;
    int n;

    /* Keep looping until we read exactly 1 byte */
    while ((n =read(STDIN_FILENO, &c, 1)) != 1) {
        if (n == -1 && errno != EAGAIN) {
            error("read");
        }
    }

    return c;
}

void processKey(int c) {
    if (c == CTRL_KEY('q')) {
        write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
        write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
        exit(0);
    }
}
