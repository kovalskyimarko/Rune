#include "../rune.h"
#include "test.h"

struct editorConfig E;

void error(const char *s) {
    perror(s);
    exit(1);
}

void initEditorHeadless(void) {
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.mode = NORMAL_MODE;
    E.autoindent = true;
    E.tabSize = 4;
    E.isUndoing = false;
    initActions();
}

void teardown(void) {
    for (int i = 0; i < E.numrows; i++) {
        free(E.row[i].chars);
        if (E.row[i].hl) free(E.row[i].hl);
    }
    free(E.row);
    E.row = NULL;
    E.numrows = 0;
}

void test_basic_undo_redo(void) {
    initEditorHeadless();
    
    insertChar('H');
    insertChar('i');
    
    ASSERT_EQ(E.numrows, 1);
    ASSERT_EQ(E.row[0].len, 2);
    ASSERT_STR_EQ(E.row[0].chars, "Hi");
    
    undo();
    ASSERT_EQ(E.row[0].len, 1);
    ASSERT_STR_EQ(E.row[0].chars, "H");
    
    undo();
    ASSERT_EQ(E.row[0].len, 0);
    ASSERT_STR_EQ(E.row[0].chars, "");
    
    redo();
    ASSERT_EQ(E.row[0].len, 1);
    ASSERT_STR_EQ(E.row[0].chars, "H");
    
    teardown();
}

void test_undo_empty(void) {
    initEditorHeadless();
    undo();
    redo();
    ASSERT_EQ(E.numrows, 0);
    teardown();
}

int main(void) {
    printf("\n--- Running Actions Tests ---\n");
    RUN_TEST(test_basic_undo_redo);
    RUN_TEST(test_undo_empty);
    
    TEST_SUMMARY();
    return (tests_run == tests_passed) ? 0 : 1;
}
