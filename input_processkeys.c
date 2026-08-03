    #include "rune.h"

    void processCommandKey(int c) 
    {
        switch (c) {
            case '\n':
            case '\r':
                sendCommand();
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
                free(E.lastrow->chars);
                E.lastrow->chars = strdup("");
                E.lastrow->len = 0;
                E.cx = E.lastcx;
                E.mode = NORMAL_MODE;
                break;
            
            default:
                insertCharAtCommandLine(c);
                break;
        }
    }

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

    void processNormalModeKey(int c)
    {
        switch (c)
        {
            case 'i':
                E.mode = INSERT_MODE;
                break;
            case 'v':
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

            case ':':
                E.mode = COMMANDLINE_MODE;
                E.lastcx = E.cx;
                E.cx = 0;
                insertCharAtCommandLine(':');
                break;
        }
    }

    void processBufferKey(int c) 
    {
        switch (c) {
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
            
            default:
                insertChar(c);
                break;
        }
    }
