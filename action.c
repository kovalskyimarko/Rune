#include "rune.h"
#define MAX_CAPACITY 100

/*
 * Coordinate convention used by every action:
 *   (cxstart, cystart) - position of the first affected character
 *   (cxend,   cyend)   - position right AFTER the last affected character (exclusive) 
 *   text               - exact text that was inserted / deleted ('\n' = line break)
 */
typedef struct {
  Action_Type type;
  int cxstart;
  int cystart;
  int cxend;
  int cyend;
  char *text;
} Action;

typedef struct {
  Action *actions;
  int currentIndex;
  size_t len;
  size_t capacity;
} ActionsList;

ActionsList actionList;

void initActions(void) {
  actionList.currentIndex = 0;
  actionList.len = 0;
  actionList.capacity = 4;
  actionList.actions = xmalloc(actionList.capacity * sizeof(Action));
}

void createAction(Action_Type type, int cxstart, int cystart, int cxend,
                  int cyend, char *text) {
  if (E.isUndoing)
    return;

  if ((size_t)actionList.currentIndex < actionList.len) {
    for (size_t i = actionList.currentIndex; i < actionList.len; i++) {
      free(actionList.actions[i].text);
    }
    actionList.len = actionList.currentIndex;
  }

  char *textCopy = (text != NULL) ? xstrdup(text) : NULL;

  Action ac = {type, cxstart, cystart, cxend, cyend, textCopy};
  if (actionList.len == actionList.capacity) {
    if (actionList.capacity == MAX_CAPACITY) {
      free(actionList.actions[0].text);
      memmove(&actionList.actions[0], actionList.actions + 1,
              (MAX_CAPACITY - 1) * sizeof(Action));

      actionList.len--;

      if (actionList.currentIndex > 0) {
        actionList.currentIndex--;
      }
    }

    else if (actionList.capacity * 2 > MAX_CAPACITY) {
      actionList.capacity = MAX_CAPACITY;
      actionList.actions =
          xrealloc(actionList.actions, actionList.capacity * sizeof(Action));
    }

    else {
      actionList.capacity *= 2;
      actionList.actions =
          xrealloc(actionList.actions, actionList.capacity * sizeof(Action));
    }
  }

  actionList.actions[actionList.currentIndex] = ac;
  actionList.currentIndex++;
  actionList.len++;
}

void insertTextAt(int x, int y, const char *text) {
  if (text == NULL)
    return;
  if (y < 0 || y > E.numrows)
    return;
  if (y == E.numrows)
    insertRow(E.numrows);

  erow *row = &E.row[y];
  if (x < 0)
    x = 0;
  if (x > row->len)
    x = row->len;

  int tailLen = row->len - x;
  char *tail = xstrdup(row->chars + x);
  row->len = x;
  row->chars[x] = '\0';

  const char *p = text;
  while (1) {
    const char *nl = strchr(p, '\n');
    int chunkLen = nl ? (int)(nl - p) : (int)strlen(p);

    baseInsertString(p, chunkLen, &E.row[y], E.row[y].len);

    if (nl == NULL)
      break;

    parse(&E.row[y]);
    insertRow(y + 1);
    y++;
    p = nl + 1;
  }

  E.cx = E.row[y].len;
  E.cy = y;

  baseInsertString(tail, tailLen, &E.row[y], E.row[y].len);
  parse(&E.row[y]);
  free(tail);
}

void deleteRange(int sx, int sy, int ex, int ey) {
  if (E.numrows == 0)
    return;

  if (sy < 0)
    sy = 0;
  if (ey >= E.numrows) {
    ey = E.numrows - 1;
    ex = E.row[ey].len;
  }
  if (sy > ey)
    return;

  if (sx < 0)
    sx = 0;
  if (sx > E.row[sy].len)
    sx = E.row[sy].len;
  if (ex < 0)
    ex = 0;
  if (ex > E.row[ey].len)
    ex = E.row[ey].len;

  if (sy == ey) {
    baseDeleteString(sx, ex, &E.row[sy]);
  }

  else {
    erow *last = &E.row[ey];
    int tailLen = last->len - ex;
    char *tail = xstrdup(last->chars + ex);

    erow *first = &E.row[sy];
    first->len = sx;
    first->chars[sx] = '\0';
    baseInsertString(tail, tailLen, first, sx);
    free(tail);

    for (int i = sy + 1; i <= ey; i++) {
      deleteRow(sy + 1);
    }

    parse(&E.row[sy]);
  }

  E.cx = sx;
  E.cy = sy;
}

void undo(void) {
  if (actionList.currentIndex <= 0)
    return;

  E.isUndoing = true;

  Action *ac = &actionList.actions[actionList.currentIndex - 1];

  if (ac->type == AC_INSERT) {
    deleteRange(ac->cxstart, ac->cystart, ac->cxend, ac->cyend);
  }

  else if (ac->type == AC_DELETE) {
    insertTextAt(ac->cxstart, ac->cystart, ac->text);
  }

  E.cx = ac->cxstart;
  E.cy = ac->cystart;
  E.dirty++;

  actionList.currentIndex--;
  E.isUndoing = false;
}

void redo(void) {
  if ((size_t)actionList.currentIndex >= actionList.len)
    return;

  E.isUndoing = true;

  Action *ac = &actionList.actions[actionList.currentIndex];

  if (ac->type == AC_INSERT) {
    insertTextAt(ac->cxstart, ac->cystart,
                 ac->text); // cursor ends after the text
  }

  else if (ac->type == AC_DELETE) {
    deleteRange(ac->cxstart, ac->cystart, ac->cxend,
                ac->cyend); // cursor ends at start
  }

  E.dirty++;

  actionList.currentIndex++;
  E.isUndoing = false;
}
