#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define CLEAR_SCREEN "\x1b[2J"
#define CLEAR_SCREEN_B 4
#define MOVE_CURSOR_HOME "\x1b[H"
#define MOVE_CURSOR_HOME_B 3

struct editorConfig {
    struct termios originalTermSettings;
    int cx; /* Cursor x position (0 to Screen Width - 1) */
    int cy; /* Cursor y position (0 to Screen Height -1) */
    int screenHeight;
    int screenWidth;
};

struct editorConfig E;

void error(const char* eMessage) {
    write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
    write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
    perror(eMessage);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.originalTermSettings) == -1) {
        error("tcsetattr");
    }
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
    raw.c_cflag |= (CS8);

    /*
      Control characters - Set read timeout
      VMIN = 0: return as soon as any data is available
      VTIME = 1: wait up up 100 milliseconds
    */
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    /* Apply the new attributes to STDIN_FILENO */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
        error("tcsetattr");
}

int readKey() {
    char c;
    int n;

    while (n == read(STDIN_FILENO, &c, 1) != 1) {
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

void getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        error("ioctl");
    }

    *rows = ws.ws_row; // height
    *cols = ws.ws_col; // width
}

void init() {
    E.cx = 0;
    E.cy = 0;
    getWindowSize(&E.screenHeight, &E.screenWidth);
}

int main() {
    init();
    enableRawMode();

    while (1) {
        int c = readKey();
        processKey(c);
    }

    return 0;
}