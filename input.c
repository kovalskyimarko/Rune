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
    if (!E.lastrow) {
        E.lastrow = malloc(sizeof(erow));
        E.lastrow->chars = NULL;
    }

    if (E.lastrow->chars) {
        free(E.lastrow->chars);
    }

    E.lastrow->chars = malloc(len + 1);
    if (!E.lastrow->chars) return;

    strcpy(E.lastrow->chars, s);
    E.lastrow->len = len;
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

        char *space_ptr = strchr(cmd, ' ');

        if (space_ptr != NULL) {
            while (*space_ptr == ' ') space_ptr++;

            if (*space_ptr != '\0') {
                char *target_path = expandPath(space_ptr);
                if (target_path)
                {
                    editorSetFilename(target_path);
                    free(target_path);
                }
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
            "File: %s, Number of rows: %d, Cursor pos: x: %d, y: %d, size: %s%ld %s%s",
            E.filename ? E.filename : "[No Name]",
            E.numrows,
            E.cx + 1,
            E.cy + 1,
            size == -1 ? 0 : size,
            size == -1 ? "" : "bytes",
            E.dirty > 0 ? " [ MODIFIED ]" : "");

        showMessageAtCommandLine(buf, strlen(buf));
    }

    else if (strncmp(cmd, ":w", 2) == 0 &&
        (cmd[2] == '\0' || cmd[2] == ' ')) {

        char *space_ptr = strchr(cmd, ' ');

        if (space_ptr != NULL) {
            while (*space_ptr == ' ') space_ptr++;

            if (*space_ptr != '\0') {
                char *target_path = expandPath(space_ptr);
                if (target_path)
                {
                    editorSetFilename(target_path);
                    free(target_path);
                }
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
            char *space_ptr = strchr(cmd, ' ');
            if (*space_ptr != '\0')
            {
                while (*space_ptr == ' ') space_ptr++;

                if (*space_ptr != NULL)
                {
                    char *target_path = expandPath(space_ptr);
                    if (target_path)
                    {
                        openfile(target_path);
                        free(target_path);
                    }
                }
            }
        }
    }

    else 
    {
        const char* errormessage = "Unknown command";
        showMessageAtCommandLine(errormessage, strlen(errormessage));
    }
}

void deleteCharBeforeCursor(void) {
    E.dirty++;
    if (E.cx == 0) {
        if (E.cy == 0) return;

        erow *row = &E.row[E.cy];
        erow *prev = &E.row[E.cy - 1];

        char *tmp = realloc(prev->chars, prev->len + row->len + 1);
        if (!tmp) return;
        prev->chars = tmp;
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

void deleteCharBeforeCursorAtCommandLine(void) 
{
    if (E.cx == 0) return;
    memmove(&E.lastrow->chars[E.cx-1], &E.lastrow->chars[E.cx], (E.lastrow->len - E.cx+1));
    E.cx--;
    E.lastrow->len--;
    return;
}


void deleteCharAtCursor(void) {
    E.dirty++;
    erow *row = &E.row[E.cy];

    if (E.cx < 0 || E.cx > row->len) return;
    if (E.cx == row->len) {
        if (E.cy + 1 == E.numrows) return;    
        
        erow *nextRow = &E.row[E.cy+1];
        char *tmp = realloc(row->chars, row->len + nextRow->len + 1);
        if (!tmp) return;
        row->chars = tmp;
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

void deleteCharAtCursorAtCommandLine(void) {
        if (E.cx == E.lastrow->len) return;
        memmove(&E.lastrow->chars[E.cx], &E.lastrow->chars[E.cx + 1], E.lastrow->len - E.cx+1);
        E.lastrow->len--;
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
    erow *row = &E.row[E.cy];

    if (E.cx > row->len) E.cx = row->len;

    char *right = strdup(row->chars + E.cx);
    if (!right) return;

    row->chars[E.cx] = '\0';
    row->len = E.cx;

    insertRow(E.cy + 1);

    free(E.row[E.cy + 1].chars);
    E.row[E.cy + 1].chars = right;
    E.row[E.cy + 1].len = strlen(right);
    E.cy++;
    E.cx = 0;
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


void insertChar(int c) {
    E.dirty++;
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
    
    newstr[E.cx] = ch;
    
    row->chars = newstr;
    row->len++;
    E.cx++;
}

void insertString(const char *s, int len) {
    E.dirty+=len;
    if (E.numrows == 0) {
        insertRow(0);
    }

    erow* row = &E.row[E.cy];

    if (E.cx < 0) E.cx = 0;
    if (E.cx > row->len) E.cx = row->len;

    char* newstr = realloc(row->chars, row->len + len + 1);
    if (!newstr) return;

    memmove(&newstr[E.cx + len], &newstr[E.cx], row->len - E.cx + 1);

    memcpy(&newstr[E.cx], s, len);

    row->chars = newstr;
    row->len += len;
    E.cx += len;
}

void insertCharAtCommandLine(int c) {
    char ch = (char) c;
    char* newstr = realloc(E.lastrow->chars, E.lastrow->len + 2);
    if (!newstr) return;
    memmove(&newstr[E.cx+1], &newstr[E.cx], (E.lastrow->len - E.cx + 1));
    
    newstr[E.cx] = ch;
    
    E.lastrow->chars = newstr;
    E.lastrow->len++;
    E.cx++;
    return;
}


void sendCommand(void) {
    char *cmd_copy = strdup(E.lastrow->chars);

    free(E.lastrow->chars);
    E.lastrow->chars = strdup("");
    E.lastrow->len = 0;

    E.mode = NORMAL_MODE;
    E.cx = E.lastcx;

    parseCommand(cmd_copy);
    free(cmd_copy);
}

void processKey(int c) {
    if (c == CTRL_KEY('q'))
    {
        write(STDOUT_FILENO, CLEAR_SCREEN, CLEAR_SCREEN_B);
        write(STDOUT_FILENO, MOVE_CURSOR_HOME, MOVE_CURSOR_HOME_B);
        cleanup();
        disableAltBuff();
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
            processCommandKey(c);
            break;
        case VISUAL_MODE:
            porcessVisualModeKey(c);
            break;
    }
}
