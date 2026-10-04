#include "rune.h"

int readKey(void) {
    char c;
    int n;

    /* Keep looping until we read exactly 1 byte */
    while ((n = read(STDIN_FILENO, &c, 1)) != 1) {
        if (n == -1 && errno == EINTR) {
            return -1;
        }
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

void showMessageAtCommandLine(const char *s, int len) {
    memset(E.statusmsg, '\0', sizeof(E.statusmsg));
    int bytesCopy = (len > 79) ? 79 : len;

    memcpy(E.statusmsg, s, bytesCopy);
    E.statusmsg[bytesCopy] = '\0';
}

ExCommand commands[] = {
    {":w", ex_w},
    {":wq", ex_qsave},
    {":x", ex_qsave},
    {":e", ex_e},
    {":q", ex_q},
    {":q!", ex_qforce},
    {":info", ex_info},
    {":set", ex_set},
    {":pwd", ex_pwd},
    {":!", ex_bang}
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

void parseCommand(const char *cmd) {
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;

    char cmd_name[16] = {0};
    const char *args = NULL;

    if (cmd[0] == ':' && cmd[1] == '!') {
        cmd_name[0] = ':';
        cmd_name[1] = '!';
        cmd_name[2] = '\0';
        args = cmd + 2;
    } else {
        int i = 0;
        while (cmd[i] && cmd[i] != ' ' && i < 15) {
            cmd_name[i] = cmd[i];
            i++;
        }
        args = cmd + i;
    }

    while (args && *args == ' ') args++;

    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(cmd_name, commands[i].name) == 0) {
            commands[i].execute(args);
            return;
        }
    }

    const char* errormessage = "Unknown command";
    showMessageAtCommandLine(errormessage, strlen(errormessage));
}

void baseDeleteChar(int x, erow* row)
{
    if (x < 0 || x >= row->len) return;

    memmove(&row->chars[x], &row->chars[x+1], (row->len - x));
    row->len--;
    parse(row);
}

void baseDeleteString(int startCx, int endCx, erow* row)
{
    if (startCx < 0 || startCx >= row->len || endCx <= startCx) return;
    if (endCx > row->len) endCx = row->len;

    memmove(
        &row->chars[startCx],
        &row->chars[endCx],
        row->len - endCx + 1
    );

    row->len -= endCx - startCx;
    parse(row);
}

void mergeLines(int lineToDeleteY, int lineToMergeWithY)
{
    if (lineToDeleteY != lineToMergeWithY + 1) return;
    if (lineToDeleteY < 0 || lineToDeleteY >= E.numrows) return;
    if (lineToMergeWithY < 0 || lineToMergeWithY >= E.numrows) return;

    erow* lineToDel = &E.row[lineToDeleteY];
    erow* lineToMerge = &E.row[lineToMergeWithY];

    lineToMerge->chars = xrealloc(lineToMerge->chars, lineToMerge->len + lineToDel->len + 1);
    memcpy(lineToMerge->chars + lineToMerge->len, lineToDel->chars, lineToDel->len + 1);
    lineToMerge->len += lineToDel->len;

    free(lineToDel->chars);
    if (lineToDel->hl) free(lineToDel->hl);
    memmove(&E.row[lineToDeleteY], &E.row[lineToDeleteY + 1], sizeof(erow) * (E.numrows - lineToDeleteY - 1));

    E.numrows--;
    parse(lineToMerge);
}

void deleteCharBeforeCursor(void) {
    if (E.cx == 0) {
        if (E.cy == 0) return;

        createAction(AC_DELETE, E.row[E.cy - 1].len, E.cy - 1, 0, E.cy, "\n");

        int newCursorX = E.row[E.cy - 1].len;
        mergeLines(E.cy, E.cy - 1);
        E.cy--;
        E.cx = newCursorX;

        E.dirty++;
        return;
    }

    char buf[2] = { E.row[E.cy].chars[E.cx - 1], '\0' };
    createAction(AC_DELETE, E.cx - 1, E.cy, E.cx, E.cy, buf);

    baseDeleteChar(E.cx-1, &E.row[E.cy]);
    E.dirty++;
    E.cx--;
}

void deleteCharAtCursor(void) {
    if (E.numrows == 0) return;
    erow *row = &E.row[E.cy];

    if (E.cx < 0 || E.cx > row->len) return;
    if (E.cx == row->len) {
        if (E.cy + 1 == E.numrows) return;
        
        createAction(AC_DELETE, E.cx, E.cy, 0, E.cy + 1, "\n");
        mergeLines(E.cy + 1, E.cy);

        E.dirty++;
        return;
    }
    
    char buf[2] = { row->chars[E.cx], '\0' };
    createAction(AC_DELETE, E.cx, E.cy, E.cx + 1, E.cy, buf);
    baseDeleteChar(E.cx, &E.row[E.cy]);
    E.dirty++;
}

void deleteCharBeforeCursorAtCommandLine(void) 
{
    if (E.cx <= 0) return;
    baseDeleteChar(E.cx - 1, E.lastrow);
    E.cx--;
}

void deleteCharAtCursorAtCommandLine(void) {
    baseDeleteChar(E.cx, E.lastrow);
    return;
}

void insertRow(int at) {
    if (at < 0 || at > E.numrows) return;

    E.row = xrealloc(E.row, sizeof(erow) * (E.numrows + 1));

    if (at < E.numrows) {
        memmove(&E.row[at + 1], &E.row[at],
                sizeof(erow) * (E.numrows - at));
    }

    E.row[at].len = 0;
    E.row[at].chars = xstrdup("");
    E.row[at].hl = NULL;

    E.numrows++;
    E.dirty++;
}

void deleteRow(int at) {
    if (at < 0 || at >= E.numrows) return;
    
    free(E.row[at].chars);
    if (E.row[at].hl) free(E.row[at].hl);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}

void splitRow(void) {
    if (E.numrows == 0)
    {
        insertRow(E.cy);
        return;
    }

    erow *row = &E.row[E.cy];

    if (E.cx > row->len) E.cx = row->len;

    int spaces = 0;

    if (E.autoindent)
    {
        for (int i = 0; i < row->len; i++)
        {
            if (row->chars[i] != ' ')
            {
                break;
            }

            spaces++;
        }

        bool hasTextBefore = false;

        for (int i = E.cx - 1; i >= 0; i--)
        {
            if (row->chars[i] == ' ') continue;

            hasTextBefore = true;

            if (row->chars[i] == '{') spaces+=E.tabSize;

            break;
        }

        for (int i = E.cx; i < row->len; i++)
        {
            if (row->chars[i] == ' ') continue;

            else if (row->chars[i] == '}' && hasTextBefore) spaces-=E.tabSize;

            break;
        }
    }

    bool burger = E.cx > 0 && E.cx < row->len &&
              row->chars[E.cx - 1] == '{' &&
              row->chars[E.cx] == '}';

    if (spaces < E.tabSize) spaces = 0;
    if (spaces % E.tabSize != 0) spaces = spaces - spaces % E.tabSize;

    if (burger) {
        char *buf = xmalloc(spaces + E.tabSize + spaces + 3);
        buf[0] = '\n';
        memset(buf + 1, ' ', spaces + E.tabSize);
        buf[spaces + E.tabSize + 1] = '\n';
        memset(buf + spaces + E.tabSize + 2, ' ', spaces);
        buf[spaces + E.tabSize + spaces + 2] = '\0';
        createAction(AC_INSERT, E.cx, E.cy, spaces, E.cy + 2, buf);
        free(buf);
    } else {
        char *buf = xmalloc(spaces + 2);
        buf[0] = '\n';
        memset(buf + 1, ' ', spaces);
        buf[spaces + 1] = '\0';
        createAction(AC_INSERT, E.cx, E.cy, spaces, E.cy + 1, buf);
        free(buf);
    }

    char *right = xmalloc(spaces + (row->len - E.cx) + 1);

    memset(right, ' ', spaces);
    strcpy(right + spaces, row->chars + E.cx);

    row->chars[E.cx] = '\0';
    row->len = E.cx;
    row->chars = xrealloc(row->chars, row->len + 1);

    insertRow(E.cy + 1);

    free(E.row[E.cy + 1].chars);
    E.row[E.cy + 1].chars = right;
    E.row[E.cy + 1].len = strlen(right);

    if (burger)
    {
        insertRow(E.cy + 1);

        free(E.row[E.cy + 1].chars);
        E.row[E.cy + 1].chars = xmalloc(spaces + E.tabSize + 1);

        // I am adding here tabSize because earlier it was deleted if it detected }
        memset(E.row[E.cy + 1].chars, ' ', spaces + E.tabSize); 
        E.row[E.cy + 1].chars[spaces + E.tabSize] = '\0';
        E.row[E.cy + 1].len = spaces + E.tabSize;

        E.cy++;
        E.cx = spaces + E.tabSize;

        parse(&E.row[E.cy - 1]);
        parse(&E.row[E.cy]);
        parse(&E.row[E.cy+1]);
        return;
    }

    E.cy++;
    E.cx = spaces;

    parse(&E.row[E.cy - 1]);
    parse(&E.row[E.cy]);
}

void insertRowWithText(int at, const char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;

    insertRow(at);

    erow *row = &E.row[at];
    free(row->chars);

    row->chars = xmalloc(len + 1);
    if (!row->chars) return;

    memcpy(row->chars, s, len);
    row->chars[len] = '\0';
    row->len = len;
    parse(row);
}

void baseInsertString(const char* s, int len, erow* row, int x)
{
    if (len <= 0) return;
    if (x < 0) x = 0;
    if (x > row->len) x = row->len;

    row->chars = xrealloc(row->chars, row->len + len + 1);

    memmove(&row->chars[x + len], &row->chars[x], row->len - x + 1);

    memcpy(&row->chars[x], s, len);

    row->len += len;
    parse(row);
}

void insertString(const char *s, int len) {
    if (E.numrows == 0) {
        insertRow(0);
    }

    char *temp = xmalloc(len + 1);
    memcpy(temp, s, len);
    temp[len] = '\0';
    createAction(AC_INSERT, E.cx, E.cy, E.cx + len, E.cy, temp);
    free(temp);

    erow* row = &E.row[E.cy];

    baseInsertString(s, len, row, E.cx);

    E.cx += len;
    E.dirty+=len;
}

void insertChar(int c) {
    char ch = (char) c;

    insertString(&ch, 1);
}

void insertCharAtCommandLine(int c) {
    char ch = (char) c;
    
    baseInsertString(&ch, 1, E.lastrow, E.cx);

    E.cx++;
}

void sendCommand(void) {
    parseCommand(E.lastrow->chars);

    free(E.lastrow->chars);
    if (E.lastrow->hl) { free(E.lastrow->hl); E.lastrow->hl = NULL; }
    E.lastrow->chars = xstrdup("");
    E.lastrow->len = 0;

    E.mode = NORMAL_MODE;
    E.cx = E.lastcx;
}

char* baseNextFind(char* needle, int x, int y, int offset, int *res_idx)
{
    for (int i = 0; i < E.numrows; i++)
    {
        int idx = (y + i) % E.numrows;
        char *startSearch = E.row[idx].chars;

        if (offset == 1)
        {
            if (i == 0 && x + 1 < E.row[idx].len) {
                startSearch+=x + offset;
            } else if (i == 0 && x + 1 >= E.row[idx].len) {
                continue;
            }
        }

        char* result = strstr(startSearch, needle);

        if (result != NULL) 
        {
            *res_idx = idx;
            return result;
        }
    }

    return NULL;
}

char* baseReverseFind(char* needle, int x, int y, int *res_idx)
{
    for (int i = E.numrows; i > 0; i--)
    {
        int idx = (y + i) % E.numrows;
        char *startSearch = E.row[idx].chars;

        if (i == E.numrows && x - 1 < 0)
        {
            continue;
        }

        char *last = NULL;
        char *curr = NULL;

        if (i == E.numrows)
        {
            char tempChar = startSearch[x];
            startSearch[x] = '\0';

            curr = strstr(startSearch, needle);

            while (curr != NULL)
            {
                last = curr;
                curr = strstr(curr + 1, needle);
            }

            startSearch[x] = tempChar;
        }

        else
        {
            curr = strstr(startSearch, needle);

            while (curr != NULL)
            {
                last = curr;
                curr = strstr(curr + 1, needle);
            }
        }

        if (last != NULL) 
        {
            *res_idx = idx;
            return last;
        }
    }

    return NULL;
}

void find(char* needle, bool reverse)
{
    if (E.numrows == 0) return;

    int realcx = (E.mode == COMMANDLINE_MODE) ? E.lastcx : E.cx;
    int foundY = -1;

    char* result = NULL;
    if (reverse)
    {
        result = baseReverseFind(needle, realcx, E.cy, &foundY);
    }

    else
    {
        if (E.mode == NORMAL_MODE)
        {
            result = baseNextFind(needle, realcx, E.cy, 1, &foundY);
        } else {
            result = baseNextFind(needle, realcx, E.cy, 0, &foundY);
        }
    }

    if (result != NULL)
    {
        E.cx = result - E.row[foundY].chars;
        E.cy = foundY;

        if (E.mode == COMMANDLINE_MODE) 
        {
            if (E.lastSearch) free(E.lastSearch);
            E.lastSearch = xstrdup(E.lastrow->chars + 1);

            free(E.lastrow->chars);
            if (E.lastrow->hl) { free(E.lastrow->hl); E.lastrow->hl = NULL; }
            E.lastrow->chars = xstrdup("");
            E.lastrow->len = 0;
        }

        E.mode = NORMAL_MODE;
        return;
    }

    else
    {
        if (E.mode == COMMANDLINE_MODE)
        {
            free(E.lastrow->chars);
            if (E.lastrow->hl) { free(E.lastrow->hl); E.lastrow->hl = NULL; }
            E.lastrow->chars = xstrdup("");
            E.lastrow->len = 0;
            E.cx = E.lastcx;
            E.mode = NORMAL_MODE;
        }
        const char* msg = "Pattern not found";
        showMessageAtCommandLine(msg, strlen(msg));
    }
}

void processKey(int c) {
    if (c == -1) return;

    if (c == CTRL_KEY('q'))
    {
        write(STDOUT_FILENO, CLEAR_ALL_COLLORS, CLEAR_ALL_COLLORS_B);
        write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
        write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
        write(STDOUT_FILENO, SHOW_CURSOR, SHOW_CURSOR_B);
        exit(0);
    }

    switch (E.mode)
    {
        case INSERT_MODE:
            processBufferKey(c);
            break;
        case NORMAL_MODE:
            processNormalModeKey(c);
            break;
        case COMMANDLINE_MODE:
            processLastRowKeys(c);
            break;
        case VISUAL_MODE:
            processVisualModeKey(c);
            break;
    }
}
