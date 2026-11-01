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
    E.autoindent = false;
    E.tabSize = 4;
    E.isUndoing = false;
    initActions();
}

void teardown(void) {
    for (int i = 0; i < E.numrows; i++) {
        if (E.row[i].chars) free(E.row[i].chars);
        if (E.row[i].hl) free(E.row[i].hl);
    }
    if (E.row) free(E.row);
    E.row = NULL;
    E.numrows = 0;
}

void test_insert_characters(void) {
    initEditorHeadless();
    
    insertChar('A');
    insertChar('B');
    insertChar('C');
    
    ASSERT_EQ(E.numrows, 1);
    ASSERT_EQ(E.row[0].len, 3);
    ASSERT_STR_EQ(E.row[0].chars, "ABC");
    ASSERT_EQ(E.cx, 3);
    
    E.cx = 1;
    insertChar('x');
    
    ASSERT_EQ(E.row[0].len, 4);
    ASSERT_STR_EQ(E.row[0].chars, "AxBC");
    ASSERT_EQ(E.cx, 2);
    
    teardown();
}

void test_insert_string(void) {
    initEditorHeadless();
    
    insertString("Hello", 5);
    ASSERT_EQ(E.numrows, 1);
    ASSERT_STR_EQ(E.row[0].chars, "Hello");
    ASSERT_EQ(E.cx, 5);
    
    teardown();
}

void test_split_row(void) {
    initEditorHeadless();
    
    insertString("HelloWorld", 10);
    E.cx = 5;
    
    splitRow();
    
    ASSERT_EQ(E.numrows, 2);
    ASSERT_STR_EQ(E.row[0].chars, "Hello");
    ASSERT_STR_EQ(E.row[1].chars, "World");
    ASSERT_EQ(E.cy, 1);
    ASSERT_EQ(E.cx, 0);
    
    teardown();
}

void test_delete_char_backspace(void) {
    initEditorHeadless();
    
    insertString("Test", 4);
    
    deleteCharBeforeCursor();
    ASSERT_STR_EQ(E.row[0].chars, "Tes");
    ASSERT_EQ(E.cx, 3);
    
    E.cx = 1;
    deleteCharBeforeCursor();
    ASSERT_STR_EQ(E.row[0].chars, "es");
    ASSERT_EQ(E.cx, 0);
    
    teardown();
}

void test_join_rows(void) {
    initEditorHeadless();
    
    insertString("Line1", 5);
    splitRow();
    insertString("Line2", 5);
    
    ASSERT_EQ(E.numrows, 2);
    
    E.cy = 1;
    E.cx = 0;
    
    deleteCharBeforeCursor();
    
    ASSERT_EQ(E.numrows, 1);
    ASSERT_STR_EQ(E.row[0].chars, "Line1Line2");
    ASSERT_EQ(E.cx, 5);
    ASSERT_EQ(E.cy, 0);
    
    teardown();
}

void test_delete_char_at_cursor(void) {
    initEditorHeadless();
    
    insertString("Buffer", 6);
    E.cx = 0;
    
    deleteCharAtCursor();
    ASSERT_STR_EQ(E.row[0].chars, "uffer");
    
    E.cx = E.row[0].len;
    splitRow();
    insertString("Next", 4);
    
    E.cy = 0;
    E.cx = E.row[0].len;
    deleteCharAtCursor();
    
    ASSERT_EQ(E.numrows, 1);
    ASSERT_STR_EQ(E.row[0].chars, "ufferNext");
    
    teardown();
}

void test_backspace_at_origin_empty(void) {
    initEditorHeadless();
    deleteCharBeforeCursor();
    ASSERT_EQ(E.numrows, 0);
    teardown();
}

int main(void) {
    printf("\n--- Running Buffer Tests ---\n");
    RUN_TEST(test_insert_characters);
    RUN_TEST(test_insert_string);
    RUN_TEST(test_split_row);
    RUN_TEST(test_delete_char_backspace);
    RUN_TEST(test_join_rows);
    RUN_TEST(test_delete_char_at_cursor);
    RUN_TEST(test_backspace_at_origin_empty);
    
    TEST_SUMMARY();
    return (tests_run == tests_passed) ? 0 : 1;
}
