#ifndef RUNE_H
#define RUNE_H

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define EDITOR_VERSION "0.0.1"

#define CLEAR_SCREEN "\x1b[2J"
#define CLEAR_SCREEN_B sizeof(CLEAR_SCREEN) - 1
#define CLEAR_LINE "\x1b[K"
#define CLEAR_LINE_B sizeof(CLEAR_LINE) - 1
#define MOVE_CURSOR_HOME "\x1b[H"
#define MOVE_CURSOR_HOME_B sizeof(MOVE_CURSOR_HOME) - 1
#define HIDE_CURSOR "\033[?25l"
#define HIDE_CURSOR_B sizeof(HIDE_CURSOR) - 1
#define SHOW_CURSOR "\033[?25h"
#define SHOW_CURSOR_B sizeof(SHOW_CURSOR) - 1
#define ENTER_ALT_BUFF "\x1b[?1049h"
#define ENTER_ALT_BUFF_B sizeof(ENTER_ALT_BUFF) - 1
#define LEAVE_ALT_BUFF "\x1b[?1049l"
#define LEAVE_ALT_BUFF_B sizeof(LEAVE_ALT_BUFF) - 1
#define ENABLE_MOUSE "\x1b[?1000h"
#define ENABLE_MOUSE_B sizeof(ENABLE_MOUSE) - 1
#define DISABLE_MOUSE "\x1b[?1000l"
#define DISABLE_MOUSE_B sizeof(DISABLE_MOUSE) - 1

enum SPECIAL_KEYS {
    ARROW_UP = 1000,
    ARROW_DOWN = 1001,
    ARROW_LEFT = 1002,
    ARROW_RIGHT = 1003,
    HOME_KEY = 1004,
    END_KEY = 1005,
    DEL_KEY = 1006,
    PAGE_UP = 1007,
    PAGE_DOWN = 1008
};

typedef struct erow {
    char* chars;
    int len;
} erow;

struct editorConfig {
    struct termios originalTermSettings;
    int cx; /* Cursor x position*/
    int cy; /* Cursor y position (0 to Screen Height -1)*/
    int screenWidth;
    int screenHeight;
    int numrows;
    int coloff; /* From what position start rendering the columns*/
    int rowoff; /* From what position start rendering the rows*/
    char* filename;
    erow* row;  /* Pointer to current row*/
};

extern struct editorConfig E;

void refreshScreen(void); 
// terminal.c
void disableRawMode(void);
void enableAltBuff(void);
void enableMouseTracking(void);
void disableAltBuff(void);
void disableMouseTracking(void);
void enableRawMode(void);
void getWindowSize(int *rows, int *cols);

// input.c
int readKey(void);
void processKey(int c);

// main.c
void init(void);
void error(const char* eMessage);

#endif
