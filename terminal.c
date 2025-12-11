#include "rune.h"

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.originalTermSettings);
}

void enableAltBuff() {
    write(STDIN_FILENO, ENTER_ALT_BUFF, ENTER_ALT_BUFF_B);
}

void disableAltBuff() {
    write(STDIN_FILENO, LEAVE_ALT_BUFF, LEAVE_ALT_BUFF_B);
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.originalTermSettings) == -1) {
        error("tcgetattr");
    }

    atexit(disableRawMode);

    struct termios raw = E.originalTermSettings;

    /*
      Local flags - Turn off echoing, canonical mode, signals, extended functions
      Echo - terminal will not print input characters
      ICANON - disable canonical mode (read byte-by-byte)
      ISIG - disable signal chars (Ctrl-C, Ctrl-Z)
      IEXTEN - disable extended functions
    */
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    /*
      Input flags - Turn off software flow control, CR to NL mapping, parity checking, stripping, and more
      IXON - disable Ctrl-S and Ctrl-Q
      ICRNL - disable CR to NL mapping
      BRKINT, INPCK, ISTRIP - disable various input processing
    */
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);

    /*
      Output flags - Turn off post-processing of output
      OPOST - disable all output processing
    */
    raw.c_oflag &= ~(OPOST);

    /* Control flags - Set character size to 8 bits per byte */
    raw.c_cflag &= ~CSIZE; 
    raw.c_cflag |= (CS8);

    /*
      Control characters - Set read timeout
      VMIN = 0: return as soon as there is any data
      VTIME = 1: wait for 100 ms before returning 
    */
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
