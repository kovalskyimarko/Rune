#include "rune.h"

void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.originalTermSettings);
}

void enableAltBuff(void) {
    write(STDOUT_FILENO, ENTER_ALT_BUFF, ENTER_ALT_BUFF_B);
}

void disableAltBuff(void) {
    write(STDOUT_FILENO, LEAVE_ALT_BUFF, LEAVE_ALT_BUFF_B);
}

void enableRawMode(void) {
    if (tcgetattr(STDIN_FILENO, &E.originalTermSettings) == -1) {
        error("tcgetattr");
    }

    atexit(disableRawMode);

    struct termios raw = E.originalTermSettings;

    /* Local modes: echoing off, canonical off, no extended functions, no signal chars (^Z, ^C) */
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    /* Input modes: no break, no CR to NL, no parity check, no strip char, no flow control */
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);

    /* Output modes: disable post processing */
    raw.c_oflag &= ~(OPOST);

    /* Control modes: set 8 bit chars */
    raw.c_cflag &= ~CSIZE; 
    raw.c_cflag |= (CS8);

    /* Control chars: set return condition min bytes and timeout (100 ms) */
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;


    /* Apply the new attributes to STDIN_FILENO */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
        error("tcsetattr");
}

void getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        error("ioctl");
    }

    *rows = ws.ws_row; // height
    *cols = ws.ws_col; // width
}
