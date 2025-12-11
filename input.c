#include "rune.h"

int readKey() {
    char c;
    int n;

    /* Keep looping until we read exactly 1 byte */
    while ((n = read(STDIN_FILENO, &c, 1)) != 1) {
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
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '4': return END_KEY;
                        case '3': return DEL_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }

            } 
            else {
                switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        }
        /* Usually for MacOS*/
        else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
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
        erow *prev = &E.row[E.cy - 1];

        prev->chars = realloc(prev->chars, prev->len + row->len + 1);
        if (!prev->chars) return;
        memcpy(prev->chars + prev->len, row->chars, row->len + 1);
        E.cx = prev->len;
        prev->len += row->len;

        free(row->chars);

        memmove(&E.row[E.cy], &E.row[E.cy + 1],
            sizeof(erow) * (E.numrows - E.cy - 1));

        E.numrows--;
        E.cy--;

        return;
    }

    erow *row = &E.row[E.cy];
    memmove(&row->chars[E.cx-1], &row->chars[E.cx], (row->len - E.cx+1));
    E.cx--;
    row->len--;
}

void deleteCharAtCursor() {
    erow *row = &E.row[E.cy];

    if (E.cx < 0 || E.cx > row->len) return;
    if (E.cx == row->len) {
        if (E.cy + 1 == E.numrows) return;    
        
        erow *nextRow = &E.row[E.cy+1];
        row->chars = realloc(row->chars, row->len + nextRow->len + 1);
        memcpy(row->chars + row-> len, nextRow -> chars, nextRow -> len + 1);
        row->len += nextRow->len;
        free(nextRow->chars); 
        memmove(&E.row[E.cy+1], &E.row[E.cy+2], sizeof(erow) * (E.numrows - E.cy - 1));

        E.numrows--;
        
        return;
    }
    memmove(&row->chars[E.cx], &row->chars[E.cx + 1], row->len - E.cx+1);
    row->len--;
}

// what index to be inserted in
void insertRow(int at) {
    if (at < 0 || at > E.numrows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    if (!E.row) return;

    if (at < E.numrows) {
        memmove(&E.row[at + 1], &E.row[at],
                sizeof(erow) * (E.numrows - at));
    }

    E.row[at].len = 0;
    E.row[at].chars = strdup("");

    E.numrows++;
}

void splitRow(int y, int x) {
    if (y < 0 || y >= E.numrows) return;
    if (x < 0) x = 0;
    erow *row = &E.row[y];

    if (x > row->len) x = row->len;

    char *right = strdup(row->chars + x);
    if (!right) return;

    row->chars[x] = '\0';
    row->len = x;

    insertRow(y + 1);

    free(E.row[y + 1].chars);
    E.row[y + 1].chars = right;
    E.row[y + 1].len = strlen(right);
    E.cy++;
    E.cx = 0;
}

void insertChar(int c) {
    if (E.numrows == 0) {
        insertRow(0);
    }

    erow* row = &E.row[E.cy];
    if (E.cx < 0) E.cx = 0;
    if (E.cx > row->len) E.cx = row->len;
    char ch = (char)c;

    char* newstr = realloc(row->chars, row->len + 2);
    if (!newstr) return;
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
            free(E.row);
            disableAltBuff();
            exit(0);
            break;

        case '\n':
        case '\r':
            splitRow(E.cy, E.cx);
            break;

        case 127:
        case '\b':
            if (E.numrows == 0) return;
            deleteCharBeforeCursor();
            break;
        
        case '\t':
            for (int i = 0; i < 4; i++) {
                insertChar(32);
            }
            break;
            
        case DEL_KEY:
            if (E.numrows == 0) return;
            deleteCharAtCursor();
            break;
        
        case ARROW_LEFT:
            if (E.numrows == 0) return;
            if (E.cx > 0) E.cx--;
            else if (E.cy > 0) { 
                E.cy--;
                E.cx = E.row[E.cy].len;
            }
            break;

        case ARROW_RIGHT:
            if (E.numrows == 0) return;
            if (E.cx < E.row[E.cy].len) E.cx++;
            else if (E.cy + 1 < E.numrows) {
                E.cy++;
                E.cx = 0;
            }
            break;

        case ARROW_UP:
            if (E.numrows == 0) return;
            if (E.cy > 0) {
                E.cy--;
                if (E.cx > E.row[E.cy].len) {
                    E.cx = E.row[E.cy].len;
                }            
            }
            break;

        case ARROW_DOWN:
            if (E.numrows == 0) return;
            if (E.cy + 1 < E.numrows) {
                E.cy++;
                if (E.cx > E.row[E.cy].len) {
                    E.cx = E.row[E.cy].len;
                }
            }
            break;

        case HOME_KEY:
            if (E.numrows == 0) return;
            E.cx = 0;
            break;

        case END_KEY:
            if (E.numrows == 0) return;
            E.cx = E.row[E.cy].len;
            break;

        case PAGE_UP:
            if (E.numrows == 0) return;
            E.cy = 0;
            if (E.cx > E.row[E.cy].len) {
                E.cx = E.row[E.cy].len;
            }
            break;

        case PAGE_DOWN:
            if (E.numrows == 0) return;
            E.cy = E.numrows-1;
            if (E.cx > E.row[E.cy].len) {
                E.cx = E.row[E.cy].len;
            }
            break;

        case '\x1b':
            break;

        default:
            insertChar(c);
            break;
    }
}
