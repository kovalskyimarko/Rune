#include "rune.h"

//TODO: a lot of memory mistakes need to be fixed look into them

int readKey() {
    char c;
    int n;

    /* Keep looping until we read exactly 1 byte */
    while ((n =read(STDIN_FILENO, &c, 1)) != 1) {
        if (n == -1 && errno != EAGAIN) {
            error("read");
        }
    }

    if (c == '\x1b') {          // Escape sequence detected
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                // Handle some other types later;
            } else {
                switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                }
            }
        }

        return '\x1b';
    }

    return c;
}

void deleteCharBeforeCursor() {
    if (E.cx == 0) {
        if (E.cy == 0) return;
        erow *row = &E.row[E.cy];
        erow *prevRow = &E.row[E.cy-1];
        prevRow->chars = realloc(prevRow->chars, prevRow->len + row->len + 1);
        memmove(&prevRow->chars[prevRow->len], row->chars, (row->len+1));
        prevRow->len += row->len;

        E.cx = prevRow->len;
        E.cy--;
        E.numrows--;
        return;
    }

    erow *row = &E.row[E.cy];
    memmove(&row->chars[E.cx-1], &row->chars[E.cx], (row->len - E.cx+1));
    E.cx--;
    row->len--;
}

// at - variable insert to what line (what index to be inserted in)
void insertRow(int at) {
    if (at >= E.screenHeight) return; // Return for now (add later vertical scroll);

    if (E.row == NULL) {
        E.row = malloc(sizeof(erow));
        E.numrows = 1;
        E.row->chars = NULL;
        E.row->len=0;
    }

    else if (at == E.numrows) {
        erow *row = &E.row[E.cy];
        char *newchars = strdup(row->chars + E.cx); // right part
        row->chars[E.cx] = '\0';
        row->len = E.cx;

        E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
        E.row[E.cy + 1].chars = newchars;
        E.row[E.cy + 1].len = strlen(newchars);

        E.cx = 0;
        E.cy++;
        E.numrows++;
    }

    else {
        if (at == 0) {
            return;
        }

        erow *row = &E.row[at-1];
        char *newchars = strdup(row->chars + E.cx); // right part
        row->chars[E.cx] = '\0';
        row->len = E.cx;

        E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
        memmove(&E.row[at + 1], &E.row[at], sizeof(erow)*(E.numrows - at));
        E.row[at].chars = newchars;
        E.row[at].len = strlen(newchars);
        E.cx = 0;
        E.cy++;
        E.numrows++;
    }
}

void insertChar(int c) {
    if (E.numrows == 0) {
        insertRow(0);
    }

    erow* row = &E.row[E.cy];
    char ch = (char)c;

    char* newstr = realloc(row->chars, row->len + 2);
    memmove(&newstr[E.cx+1], &newstr[E.cx], (row->len - E.cx + 1));
    
    newstr[E.cx] = ch;      // store character
    
    row->chars = newstr;
    row->len++;
    E.cx++;
}

void processKey(int c) {
    switch (c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
            write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
            exit(0);
            break;
        case 'n':
        case '\r':
            insertRow(E.cy+1);
            break;
        case 127:
        case '\b':
            deleteCharBeforeCursor();
            break;
        
        case ARROW_LEFT:
            if (E.cx > 0) E.cx--;
            else if (E.cy > 0) { 
                E.cy--;
                E.cx = E.row[E.cy].len;
            }
            break;
        case ARROW_RIGHT:
            if (E.cx < E.row[E.cy].len) E.cx++;
            else if (E.cy + 1 < E.numrows) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy > 0) E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy + 1 < E.numrows) E.cy++;
            break;
        
        default:
            insertChar(c);
            break;
    }
}
