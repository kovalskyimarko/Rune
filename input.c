#include "rune.h"

int readKey(void) {
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

void showMessageAtCommandLine(const char *s, int len) {
    memset(E.statusmsg, '\0', sizeof(E.statusmsg));
    int bytesCopy = (len > 79) ? 79 : len;

    memcpy(E.statusmsg, s, bytesCopy);
    E.statusmsg[bytesCopy] = '\0';
}

char* getCommandArg(const char *cmd)
{
    char *space_ptr = strchr(cmd, ' ');
    if (space_ptr != NULL) {
        while (*space_ptr == ' ') space_ptr++;

        if (*space_ptr != '\0') return space_ptr;
    }
    
    return NULL;
}

void parseCommand(const char *cmd) {
    while (*cmd == ' ') cmd++;

    if (strncmp(cmd, ":q", 2) == 0 &&
        cmd[2] == '\0') {
        
        if (E.dirty > 0)
        {
            const char* message = "Unsaved changes. To quit without saving write :q!";
            showMessageAtCommandLine(message, strlen(message));
        }

        else
        {
            processKey(CTRL_KEY('q'));
        }

    }

    else if (strncmp(cmd, ":q!", 3) == 0 &&
        cmd[3] == '\0') {
        processKey(CTRL_KEY('q'));
    }

    else if((strncmp(cmd, ":wq", 3) == 0 && (cmd[3] == '\0' || cmd[3] == ' ')) || 
        (strncmp(cmd, ":x", 2) == 0&& (cmd[2] == '\0' || cmd[2] == ' '))) {

        char* arg = getCommandArg(cmd);

        if (arg != NULL) {
            char *target_path = expandPath(arg);
            if (target_path)
            {
                editorSetFilename(target_path);
                free(target_path);
            }
        }

        savefile();
        processKey(CTRL_KEY('q'));
    }

    else if (strncmp(cmd, ":pwd", 4) == 0 &&
        cmd[4] == '\0') {
        
        char buf[1024];
        if (getcwd(buf, sizeof(buf)) != NULL) {
            showMessageAtCommandLine(buf, strlen(buf));
        } else {
            const char* errormessage = "Unknown error";
            showMessageAtCommandLine(errormessage, strlen(errormessage));
        }

    }

    else if (strncmp(cmd, ":info", 5) == 0 &&
        cmd[5] == '\0') {

        long size = -1;
        
        if (E.filename != NULL)
        {
            FILE *fp = fopen(E.filename, "rb");
            if (fp != NULL)
            {
                
                if (fseek(fp, 0, SEEK_END) == 0) 
                {
                    size = ftell(fp);
                    fclose(fp);
                }
            }
        }

        char buf[128];
        snprintf(buf, sizeof(buf),
            "File: %s, Number of rows: %d, Cursor pos: x: %d, y: %d, size: %ld %s%s",
            E.filename ? E.filename : "[No Name]",
            E.numrows,
            E.cx + 1,
            E.cy + 1,
            size == -1 ? 0 : size,
            size == -1 ? "" : "bytes",
            E.dirty > 0 ? " [MODIFIED]" : ""
        );

        showMessageAtCommandLine(buf, strlen(buf));
    }

    else if (strncmp(cmd, ":w", 2) == 0 &&
        (cmd[2] == '\0' || cmd[2] == ' ')) {

        char* arg = getCommandArg(cmd);

        if (arg != NULL)
        {
            char *target_path = expandPath(arg);
            if (target_path)
            {
                editorSetFilename(target_path);
                free(target_path);
            }
        }

        savefile();
    }

    else if (strncmp(cmd, ":e", 2) == 0 &&
        (cmd[2] == '\0' || cmd[2] == ' ')) {
        if (cmd[2] == '\0')
        {
            if (E.filename == NULL)
            {
                const char* errormessage = "No file name: Usage :e file_name";
                showMessageAtCommandLine(errormessage, strlen(errormessage));
            }

            else
            {
                openfile(E.filepath);
            }
        }

        else
        {
            char* arg = getCommandArg(cmd);

            if (arg != NULL)
            {
                char *target_path = expandPath(arg);
                if (target_path)
                {
                    openfile(target_path);
                    free(target_path);
                }
            }
        }
    }

    else if (strncmp(cmd, ":set", 4) == 0 && 
        (cmd[4] == '\0' || cmd[4] == ' '))
    {
        char* arg = getCommandArg(cmd);

        if (arg != NULL)
        {
            if ((strncmp(arg, "nu", 2) == 0  && (arg[2] == ' ' || arg[2] == '\0')) || 
                ((strncmp(arg, "number", 6) == 0) && (arg[6] == ' ' || arg[6] == '\0')))
            {
                E.showLineNumbers = true;
            }

            else if ((strncmp(arg, "nonu", 4) == 0  && (arg[4] == ' ' || arg[4] == '\0')) || 
                ((strncmp(arg, "nonumber", 8) == 0) && (arg[8] == ' ' || arg[8] == '\0')))
            {
                E.showLineNumbers = false;
            }

            else if ((strncmp(arg, "number!", 7)) == 0  && (arg[7] == ' ' || arg[7] == '\0'))
            {
                E.showLineNumbers = !E.showLineNumbers;
            }

            else if ((strncmp(arg, "rnu", 3) == 0  && (arg[3] == ' ' || arg[3] == '\0')) || 
                ((strncmp(arg, "relativenumber", 14) == 0) && (arg[14] == ' ' || arg[14] == '\0')))
            {
                E.showRLineNumbers = true;
            }

            else if ((strncmp(arg, "nornu", 5) == 0  && (arg[5] == ' ' || arg[5] == '\0')) || 
                ((strncmp(arg, "norelativenumber", 16) == 0) && (arg[16] == ' ' || arg[16] == '\0')))
            {
                E.showRLineNumbers = false;
            }

            else if ((strncmp(arg, "relativenumber!", 15)) == 0  && (arg[15] == ' ' || arg[15] == '\0'))
            {
                E.showRLineNumbers = !E.showRLineNumbers;
            }
        }
    }

    else 
    {
        const char* errormessage = "Unknown command";
        showMessageAtCommandLine(errormessage, strlen(errormessage));
    }
}

void baseDeleteChar(int x, erow* row)
{
    if (x < 0 || x >= row->len) return;

    memmove(&row->chars[x], &row->chars[x+1], (row->len - x));
    row->len--;
}

void mergeLines(int lineToDeleteY, int lineToMergeWithY)
{
    if (lineToDeleteY < 0 || lineToDeleteY >= E.numrows) return;
    if (lineToMergeWithY < 0 || lineToMergeWithY >= E.numrows) return;

    erow* lineToDel = &E.row[lineToDeleteY];
    erow* lineToMerge = &E.row[lineToMergeWithY];

    char *tmp = realloc(lineToMerge->chars, lineToMerge->len + lineToDel->len + 1);
    if (!tmp) return;

    lineToMerge->chars = tmp;
    memcpy(lineToMerge->chars + lineToMerge->len, lineToDel->chars, lineToDel->len + 1);
    lineToMerge->len += lineToDel->len;

    free(lineToDel->chars);

    memmove(&E.row[lineToDeleteY], &E.row[lineToDeleteY + 1], sizeof(erow) * (E.numrows - lineToDeleteY - 1));

    E.numrows--;
}

void deleteCharBeforeCursor(void) {
    if (E.cx == 0) {
        if (E.cy == 0) return;

        int newCursorX = E.row[E.cy - 1].len;
        mergeLines(E.cy, E.cy - 1);
        E.cy--;
        E.cx = newCursorX;

        E.dirty++;
        return;
    }

    baseDeleteChar(E.cx-1, &E.row[E.cy]);
    E.dirty++;
    E.cx--;
}

void deleteCharAtCursor(void) {
    erow *row = &E.row[E.cy];

    if (E.cx < 0 || E.cx > row->len) return;
    if (E.cx == row->len) {
        if (E.cy + 1 == E.numrows) return;
        
        mergeLines(E.cy + 1, E.cy);

        E.dirty++;
        return;
    }
    
    baseDeleteChar(E.cx, &E.row[E.cy]);
    E.dirty++;
}

void deleteCharBeforeCursorAtCommandLine(void) 
{
    baseDeleteChar(E.cx - 1, E.lastrow);
    E.cx--;
}

void deleteCharAtCursorAtCommandLine(void) {
    baseDeleteChar(E.cx, E.lastrow);
    return;
}

void insertRow(int at) {
    E.dirty++;
    if (at < 0 || at > E.numrows) return;

    erow *tmp = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    if (!tmp) return;
    E.row = tmp;

    if (at < E.numrows) {
        memmove(&E.row[at + 1], &E.row[at],
                sizeof(erow) * (E.numrows - at));
    }

    E.row[at].len = 0;
    E.row[at].chars = strdup("");

    E.numrows++;
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

        if (row->chars[i] == '{') spaces+=4;

        break;
    }

    for (int i = E.cx; i < row->len; i++)
    {
        if (row->chars[i] == ' ') continue;

        else if (row->chars[i] == '}' && hasTextBefore) spaces-=4;

        break;
    }

    if (spaces < 4) spaces = 0;
    if (spaces % 4 != 0) spaces = spaces - spaces % 4;

    char *right = malloc(spaces + (row->len - E.cx) + 1);

    if (!right) return;

    memset(right, ' ', spaces);
    strcpy(right + spaces, row->chars + E.cx);

    row->chars[E.cx] = '\0';
    row->len = E.cx;

    insertRow(E.cy + 1);

    free(E.row[E.cy + 1].chars);
    E.row[E.cy + 1].chars = right;
    E.row[E.cy + 1].len = strlen(right);
    E.cy++;
    E.cx = spaces;
}

void insertRowWithText(int at, const char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;

    insertRow(at);

    erow *row = &E.row[at];
    free(row->chars);

    row->chars = malloc(len + 1);
    if (!row->chars) return;

    memcpy(row->chars, s, len);
    row->chars[len] = '\0';
    row->len = len;
}

void baseInsertString(const char* s, int len, erow* row, int x)
{
    if (x < 0) x = 0;
    if (x > row->len) x = row->len;

    char* newstr = realloc(row->chars, row->len + len + 1);
    if (!newstr) return;

    memmove(&newstr[x + len], &newstr[x], row->len - x + 1);

    memcpy(&newstr[x], s, len);

    row->chars = newstr;
    row->len += len;
}

void insertString(const char *s, int len) {
    if (E.numrows == 0) {
        insertRow(0);
    }

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
    E.lastrow->chars = strdup("");
    E.lastrow->len = 0;

    E.mode = NORMAL_MODE;
    E.cx = E.lastcx;
}

void find(char* needle, bool reverse)
{
    if (E.numrows == 0) return;

    int realcx = (E.mode == COMMANDLINE_MODE) ? E.lastcx : E.cx;
    int offset = 0;

    if (E.mode == NORMAL_MODE)
    {
        offset = reverse == false ? 1 : -1;
    }

    int i = reverse == false ? 0 : E.numrows;
    int limit = reverse == false ? E.numrows : 0;

    while (true)
    {
        int idx = (E.cy + i) % E.numrows;
        char *startSearch = E.row[idx].chars;

        if (offset == 1)
        {
            if (i == 0 && realcx + 1 < E.row[idx].len) {
                startSearch+=realcx + offset;
            } else if (i == 0 && realcx + 1 >= E.row[idx].len) {
                i++;
                continue;
            }
        }
        else if (offset == -1 && i == E.numrows && realcx - 1 < 0) 
        {
            i--;
            continue;
        }

        if (!reverse)
        {
            char* result = strstr(startSearch, needle);

            if (result != NULL)
            {
                E.cx = result - E.row[idx].chars;
                E.cy = idx;

                if (E.mode == COMMANDLINE_MODE) 
                {
                    if (E.lastSearch) free(E.lastSearch);
                    E.lastSearch = strdup(E.lastrow->chars + 1);
                }

                E.mode = NORMAL_MODE;

                free(E.lastrow->chars);
                E.lastrow->chars = strdup("");
                E.lastrow->len = 0;
                return;
            }
        }

        if (reverse)
        {
            char *last = NULL;
            char *curr = NULL;

            if (i == E.numrows)
            {
                char tempChar = startSearch[E.cx];
                startSearch[E.cx] = '\0';

                curr = strstr(startSearch, needle);

                while (curr != NULL)
                {
                    last = curr;
                    curr = strstr(curr + 1, needle);
                }

                startSearch[E.cx] = tempChar;
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
                E.cx = last - E.row[idx].chars;
                E.cy = idx;

                if (E.mode == COMMANDLINE_MODE) 
                {
                    if (E.lastSearch) free(E.lastSearch);
                    E.lastSearch = strdup(E.lastrow->chars + 1);
                }

                E.mode = NORMAL_MODE;

                free(E.lastrow->chars);
                E.lastrow->chars = strdup("");
                E.lastrow->len = 0;
                return;
            }
        }

        if (reverse)
        {
            i--;
        } else {
            i++;
        }

        if (i == limit)
        {
            break;
        }        
    }

    free(E.lastrow->chars);
    E.lastrow->chars = strdup("");
    E.lastrow->len = 0;

    if (E.mode == COMMANDLINE_MODE)
    {
        E.cx = E.lastcx;
        E.mode = NORMAL_MODE;
    }
    const char* msg = "Pattern not found";
    showMessageAtCommandLine(msg, strlen(msg));
}

void processKey(int c) {
    if (c == CTRL_KEY('q'))
    {
        write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
        write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
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
