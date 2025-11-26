#ifndef RUNE_H
#define RUNE_H

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define CLEAR_SCREEN "\x1b[2J"
#define CLEAR_SCREEN_B sizeof(CLEAR_SCREEN) - 1
#define MOVE_CURSOR_HOME "\x1b[H"
#define MOVE_CURSOR_HOME_B sizeof(MOVE_CURSOR_HOME) - 1


struct editorConfig {
    struct termios originalTermSettings;
    int cx; /* Cursor x position (0 to Screen Width - 1) */
    int cy; /* Cursor y position (0 to Screen Height -1) */
    int screenWidth;
    int screenHeight;
};

extern struct editorConfig E;

//void refreshScreen(void); 
// terminal.c
void disableRawMode(void);
void enableRawMode(void);
void getWindowSize(int *rows, int *cols);

// input.c
int readKey(void);
void processKey(int c);

// main.c
void init(void);
void error(const char* eMessage);

#endif
