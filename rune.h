#ifndef RUNE_H
#define RUNE_H

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

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
    int cy; /* Cursor y position*/
    int lastcy;
    int lastcx;
    int screenWidth;
    int screenHeight;
    int numrows;
    int coloff; /* From what position start rendering the columns*/
    int rowoff; /* From what position start rendering the rows*/
    char *filepath;
    char *filename;
    bool insertMode;
    erow* row;  /* Pointer to current row*/
    erow* lastrow;
};

extern struct editorConfig E;

void refreshScreen(void); 
// terminal.c
void disableRawMode(void);
void enableAltBuff(void);
void disableAltBuff(void);
void enableRawMode(void);
void getWindowSize(int *rows, int *cols);

// input.c
int readKey(void);
void processKey(int c);
void insertRowWithText(int at, const char *s, size_t len);

// file.c
void savefile(void);
void openfile(void);

// main.c
void init(void);
void error(const char* eMessage);

#endif
