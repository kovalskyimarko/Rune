#include "rune.h"

void moveCursor(int c)
{
    switch (c)
    {
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

        case PAGE_UP:
            if (E.numrows == 0) return;
            E.cy = E.cy - E.screenHeight + 1;

            if (E.cy < 0) {
                E.cy = 0;
            }

            if (E.cx > E.row[E.cy].len) {
                E.cx = E.row[E.cy].len;
            }
            break;

        case PAGE_DOWN:
            if (E.numrows == 0) return;
            E.cy = E.cy + E.screenHeight - 1;
            
            if (E.cy > E.numrows-1) {
                E.cy = E.numrows-1;
            }

            if (E.cx > E.row[E.cy].len) {
                E.cx = E.row[E.cy].len;
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
    }
}

void moveCursorKeyBinds(int c)
{
    switch (c)
    {
        case '0':
            if (E.normalModeMult == 0) {
                E.cx = 0;
            } else {
                E.normalModeMult = (E.normalModeMult * 10);
            }
            break;
        case '$':
            if (E.numrows > 0)
                E.cx = E.row[E.cy].len;
            E.normalModeMult = 0;
            break;
            
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            E.normalModeMult = (E.normalModeMult * 10) + (c - '0');
            break;
        
        case 'G':
            if (E.numrows == 0) break;
            if (E.normalModeMult == 0)
            {
                E.cy = E.numrows - 1;
            }
            else 
            {
                if (E.normalModeMult > E.numrows)
                {
                    E.cy = E.numrows - 1;
                }

                else
                {
                    E.cy = E.normalModeMult - 1;
                }
            }
            E.cx = 0;

            E.normalModeMult = 0;
            break;
        case 'w': {
            if (E.numrows == 0) break;
            
            int times = (E.normalModeMult == 0) ? 1 : E.normalModeMult;
            for (int i = 0; i < times; i++) {
                bool seenSpace = false;
                while (E.cx < E.row[E.cy].len)
                {
                    if (E.row[E.cy].chars[E.cx] == ' ')
                    {
                        seenSpace = true;
                    } else if (seenSpace == true)
                    {
                        break;
                    }

                    E.cx++;
                }

                if (E.cx >= E.row[E.cy].len)
                {
                    if (E.cy < E.numrows - 1)
                    {
                        E.cy++;
                        E.cx = 0;
                    }
                }
            }

            E.normalModeMult = 0;
            break;
        }

        case 'b': {
            if (E.numrows == 0) break;
            
            int times = (E.normalModeMult == 0) ? 1 : E.normalModeMult;
            for (int i = 0; i < times; i++) {
                if (E.cx == 0)
                {
                    if (E.cy == 0) break;
                    E.cy--;
                    E.cx = E.row[E.cy].len;
                }

                if (E.cx > 0)
                {
                    E.cx--;
                }

                while (E.cx > 0 && E.row[E.cy].chars[E.cx] == ' ')
                {
                    E.cx--;
                }

                while (E.cx > 0 && E.row[E.cy].chars[E.cx] != ' ')
                {
                    E.cx--;
                }

                if (E.cx != 0)
                {
                    E.cx++;
                }
            }

            E.normalModeMult = 0;
            break;
        }

        case 'h': {
            int times = (E.normalModeMult == 0) ? 1 : E.normalModeMult;
            for (int i = 0; i < times; i++) {
                moveCursor(ARROW_LEFT);
            }
            E.normalModeMult = 0;
            break;
        }
        case 'j': {
            int times = (E.normalModeMult == 0) ? 1 : E.normalModeMult;
            for (int i = 0; i < times; i++) {
                moveCursor(ARROW_DOWN);
            }
            E.normalModeMult = 0;
            break;
        }
        case 'k': {
            int times = (E.normalModeMult == 0) ? 1 : E.normalModeMult;
            for (int i = 0; i < times; i++) {
                moveCursor(ARROW_UP);
            }
            E.normalModeMult = 0;
            break;
        }
        case 'l': {
            int times = (E.normalModeMult == 0) ? 1 : E.normalModeMult;
            for (int i = 0; i < times; i++) {
                moveCursor(ARROW_RIGHT);
            }
            E.normalModeMult = 0;
            break;
        }
    }

}

void normalize(int* startX, int* startY, int* endX, int* endY)
{
    if (*endY < *startY)
    {
        int t = *endY;
        *endY = *startY;
        *startY = t;

        t = *endX;
        *endX = *startX;
        *startX = t;
    }

    if ((*endX < *startX) && (*startY == *endY))
    {
        int t = *endX;
        *endX = *startX;
        *startX = t;
    }

    if (*endX >= E.row[*endY].len) {
        *endX = E.row[*endY].len - 1;
    }

    if (*endX < 0) {
        *endX = 0; 
    }
}

void copy(void)
{
    if (E.yankbuff) free(E.yankbuff);
    E.yankbuff = NULL;
    int len = 0;

    int startY = E.vStartcy;
    int endY =   E.cy;

    int startX = E.vStartcx;
    int endX =   E.cx;

    normalize(&startX, &startY, &endX, &endY);

    if (startY < endY) 
    {
        for (int currY = startY; currY <= endY; currY++)
        {
            if (currY == startY)
            {
                len += E.row[startY].len - startX;
            }

            else if (currY < endY)
            {
                len += E.row[currY].len;
            }
            
            else if (currY == endY)
            {
                len += endX + 1;
            }

            if (currY != endY)
            {
                len+=1;
            }
        }
    }
    else if (startY == endY) 
    {
        len = endX - startX + 1;
    }

    len+=1;

    E.yankbuff = malloc(len);
    if (!E.yankbuff) return;
    int currIndex = 0;

    if (startY < endY) 
    {
        for (int currRow = startY; currRow <= endY; currRow++)
        {
            if (currRow == startY)
            {
                for (int currRowIndex = startX; currRowIndex < E.row[currRow].len; currRowIndex++)
                {
                    E.yankbuff[currIndex] = E.row[currRow].chars[currRowIndex];
                    currIndex++;
                }
            }

            else if (currRow < endY)
            {
                for (int currRowIndex = 0; currRowIndex < E.row[currRow].len; currRowIndex++)
                {
                    E.yankbuff[currIndex] = E.row[currRow].chars[currRowIndex];
                    currIndex++;
                }
            }
            
            else if (currRow == endY)
            {
                for (int currRowIndex = 0; currRowIndex <= endX; currRowIndex++)
                {
                    E.yankbuff[currIndex] = E.row[currRow].chars[currRowIndex];
                    currIndex++;
                }
            }

            if (currRow != endY)
            {
                E.yankbuff[currIndex] = '\n';
                currIndex++;
            }
        }
    }

    else if (startY == endY) 
    {
        for (int i = startX; i <= endX; i++)
        {
            E.yankbuff[currIndex] = E.row[startY].chars[i];
            currIndex++;
        }
    }

    E.yankbuff[currIndex] = '\0';
    E.mode = NORMAL_MODE;

}

void processLastRowKeys(int c)
{
    switch (c)
    {
        case '\n':
        case '\r':
            if (E.lastrow->chars[0] == '/' && E.lastrow->chars[1] != '\0')
            {
                find(E.lastrow->chars + 1, false);
            }

            else if (E.lastrow->chars[0] == ':' && E.lastrow->chars[1] != '\0')
            {
                sendCommand();
            }

            break;

        case 127:
        case '\b':
            deleteCharBeforeCursorAtCommandLine();
            break;
        case '\t':
            for (int i = 0; i < 4; i++) {
                insertCharAtCommandLine(32);
            }
            break;
        case DEL_KEY:
            deleteCharAtCursorAtCommandLine();
            break;
        case ARROW_LEFT:
            if (E.cx > 0) E.cx--;
            break;
        case ARROW_RIGHT:
            if (E.cx < E.lastrow->len) E.cx++;
            break;

        case HOME_KEY:
            E.cx=0; 
            break;
        
        case END_KEY:
            E.cx=E.lastrow->len;
            break;

        case '\x1b':
            E.lastrow->chars[0] = '\0';
            E.lastrow->len = 0;
            E.cx = E.lastcx;
            E.mode = NORMAL_MODE;
            break;

        default:
            insertCharAtCommandLine(c);
            break;
        
    }
}

void processNormalModeKey(int c)
{
    // After leaving find or sending a command you will always be back at Normal mode so its logical to clear a buffer here
    E.statusmsg[0] = '\0';
    switch (c)
    {
        case 'i':
            E.mode = INSERT_MODE;
            break;
        case 'v':
            E.vStartcx = E.cx;
            E.vStartcy = E.cy;
            E.mode = VISUAL_MODE;
            break;

        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
        case PAGE_UP:
        case PAGE_DOWN:
        case HOME_KEY:
        case END_KEY:
            moveCursor(c);
            break;

        case '0':case'$':
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
        case 'w': case 'b':
        case 'h': case 'j': case 'k': case 'l':
            moveCursorKeyBinds(c);
            break;
        
        case 'p': {
            if (!E.yankbuff) break;;

            int len = strlen(E.yankbuff);
            int tempLenRow = 0;
            int offset = 0;

            if (E.numrows > 0 && E.row[E.cy].len > 0 && E.cx < E.row[E.cy].len) {
                E.cx++;
            }

            for (int i = 0; i < len; i++)
            {
                if (E.yankbuff[i] != '\n')
                {
                    tempLenRow+=1;
                }

                else
                {
                    insertString(&E.yankbuff[offset], tempLenRow);
                    splitRow();
                    offset+=tempLenRow + 1; // +1 to skip \n
                    tempLenRow = 0;
                }
            }

            insertString(&E.yankbuff[offset], tempLenRow);

            break;
        }

        case 'x': {
            if (E.numrows == 0 || E.cx >= E.row[E.cy].len) break;
            if (E.yankbuff) free(E.yankbuff);

            E.yankbuff = malloc(sizeof(char) * 2);
            if (!E.yankbuff) break;

            E.yankbuff[0] = E.row[E.cy].chars[E.cx];
            E.yankbuff[1] = '\0';

            deleteCharAtCursor();
            break;
        }

        case 'A':
            if (E.numrows > 0)
                E.cx = E.row[E.cy].len;
            E.normalModeMult = 0;
            E.mode = INSERT_MODE;
            break;

        case 'I':
            E.cx = 0;
            E.normalModeMult = 0;
            E.mode = INSERT_MODE;
            break;

        case 'n':
            if (E.lastSearch) {
                find(E.lastSearch, false);
            } else {
                const char* msg = "Search buffer is empty";
                showMessageAtCommandLine(msg, strlen(msg));
            }
            break;
        
        case 'N':
            if (E.lastSearch) {
                find(E.lastSearch, true);
            } else {
                const char* msg = "Search buffer is empty";
                showMessageAtCommandLine(msg, strlen(msg));
            }
            break;

        case ':':
        case '/':
            E.lastrow->chars[0] = '\0';
            E.lastrow->len = 0;
            E.mode = COMMANDLINE_MODE;
            E.lastcx = E.cx;
            E.cx = 0;
            insertCharAtCommandLine(c);
            break;
    }
}

void processVisualModeKey(int c)
{
    switch (c)
    {
        case 'y': {
            copy();
            break;
        }

        case 'd':
        case 'x': {
            copy();

            int startY = E.vStartcy;
            int endY =   E.cy;

            int startX = E.vStartcx;
            int endX =   E.cx;

            int old_cy = E.cy;

            normalize(&startX, &startY, &endX, &endY);

            E.cx = startX;
            E.cy = startY;

            int len = strlen(E.yankbuff);
            
            if (startY < endY)
            {
                for (int i = 0; i < len; i++)
                {
                    deleteCharAtCursor();
                }
            }

            else
            {
                for (int i = 0; i <= (endX - startX); i++)
                {
                    deleteCharAtCursor();
                }

                E.cx = startX;
                E.cy = old_cy;
            }

            break;
        }

        case '0':case'$':
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
        case 'w': case 'b':
        case 'h': case 'j': case 'k': case 'l':
            moveCursorKeyBinds(c);
            break;
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
        case PAGE_UP:
        case PAGE_DOWN:
        case HOME_KEY:
        case END_KEY:
            moveCursor(c);
            break;
        case '\x1b':
            E.mode = NORMAL_MODE;
            break;
    }
}

void processBufferKey(int c) 
{
    switch (c) {
        case '\n':
        case '\r':
            splitRow();
            break;
        case 127:
        case '\b': {
            if (E.numrows == 0) return;

            erow* row = &E.row[E.cy];
            bool onlySpaces = true;

            for (int i = E.cx - 1; i >= 0; i--)
            {

                if (row->chars[i] != ' ') 
                {
                    onlySpaces = false;
                    break;
                }
            }

            if (onlySpaces && E.cx > 0)
            {
                int spacesToRemove = (E.cx % 4 != 0) ? (E.cx % 4) : 4;

                for (int i = 0; i < spacesToRemove; i++)
                {
                    deleteCharBeforeCursor();
                }
            }

            else {
                deleteCharBeforeCursor();
            }
            break;
        }
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
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
        case PAGE_UP:
        case PAGE_DOWN:
        case HOME_KEY:
        case END_KEY:
            moveCursor(c);
            break;

        case '\x1b':
            E.mode = NORMAL_MODE;
            break;

        case '}': {
            if (E.numrows == 0) {
                insertChar(c);
                break;
            }

            erow* row = &E.row[E.cy];
            bool onlySpaces = true;

            // If user closes the bracket and see letters before it I just assume its something like {code}
            for (int i = E.cx - 1; i >= 0; i--)
            {

                if (row->chars[i] != ' ') 
                {
                    onlySpaces = false;
                    break;
                }
            }

            if (onlySpaces && E.cx > 0)
            {
                int spacesToRemove = (E.cx % 4 != 0) ? (E.cx % 4) : 4;

                for (int i = 0; i < spacesToRemove; i++)
                {
                    deleteCharBeforeCursor();
                }
            }

            insertChar(c);
            break;
        }
        
        default:
            insertChar(c);
            break;
    }
}
