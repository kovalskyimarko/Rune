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
#include <signal.h>


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

#define u_i8 u_int8_t


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

typedef enum MODES {
    INSERT_MODE,
    NORMAL_MODE,
    COMMANDLINE_MODE,
    VISUAL_MODE
} MODES;

enum HL_TYPE {

    HL_DEFAULT = 0,

    HL_NUMBER,
    HL_STRING,

    HL_BRACKETS,

    HL_TYPE,
    HL_CONTROL,
    HL_STORAGE,
    HL_QUALIFIER,
    HL_STRUCT,
    HL_SPECIAL,
};

typedef struct erow {
    char* chars;
    u_i8* hl;
    int len;
} erow;

struct editorConfig {
    struct termios originalTermSettings;
    int dirty; // Flag for how many changes in a file
    int cx; /* Cursor x position*/
    int cy; /* Cursor y position*/
    int lastcx;
    int vStartcx;
    int vStartcy;
    int screenWidth;
    int screenHeight;
    int numrows;
    int coloff; /* From what position start rendering the columns*/
    int rowoff; /* From what position start rendering the rows*/
    int commandlineColloff;
    int normalModeMult;
    char pendingAction;
    char *yankbuff;
    char *filepath;
    char *filename;
    char *lastSearch;
    MODES mode;
    erow* row;  /* Pointer to current row*/
    erow* lastrow;
    char statusmsg[80];
    bool showLineNumbers;
    bool showRLineNumbers;
};

extern struct editorConfig E;

void refreshScreen(void);

// terminal.c
void disableRawMode(void);
void enableAltBuff(void);
void disableAltBuff(void);
void enableRawMode(void);
void getWindowSize(int *rows, int *cols);
void handleSigwinch(int sig);

// input_processkeys.c
void processNormalModeKey(int c);
void processVisualModeKey(int c);
void processLastRowKeys(int c);
void processBufferKey(int c) ;

// input.c
void splitRow(void);
void showMessageAtCommandLine(const char *s, int len);
void sendCommand(void);
void deleteCharBeforeCursorAtCommandLine(void);
void insertCharAtCommandLine(int c);
void deleteCharAtCursorAtCommandLine(void);
void deleteCharBeforeCursor(void);
void deleteCharAtCursor(void);
void insertChar(int c);
void insertString(const char *s, int len);
int readKey(void);
void find(char* needle, bool reverse);
void processKey(int c);
void insertRowWithText(int at, const char *s, size_t len);
void deleteRow(int at);
void mergeLines(int lineToDeleteY, int lineToMergeWithY);

// file.c
void editorSetFilename(const char *path);
char* expandPath(const char *input);
bool savefile(void);
void emergencySave(void);
void openfile(const char *fullpath);

//syntax_parser.c
void parse(erow* row);

// xmemory.c
void* xmalloc(size_t size);
void* xrealloc(void* ptr, size_t size);
char* xstrdup(const char* str);

// main.c
void init(void);
void error(const char* eMessage);
void cleanup(void);

#endif
