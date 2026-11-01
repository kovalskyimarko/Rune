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

void test_insert_empty_rows(void) {
    initEditorHeadless();
    
    insertRow(0);
    ASSERT_EQ(E.numrows, 1);
    ASSERT_EQ(E.row[0].len, 0);
    ASSERT_STR_EQ(E.row[0].chars, "");
    
    insertRow(0);
    ASSERT_EQ(E.numrows, 2);
    
    insertRow(2);
    ASSERT_EQ(E.numrows, 3);
    
    teardown();
}

void test_insert_row_with_text(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "First", 5);
    insertRowWithText(1, "Third", 5);
    insertRowWithText(1, "Second", 6);
    
    ASSERT_EQ(E.numrows, 3);
    ASSERT_STR_EQ(E.row[0].chars, "First");
    ASSERT_STR_EQ(E.row[1].chars, "Second");
    ASSERT_STR_EQ(E.row[2].chars, "Third");
    
    teardown();
}

void test_delete_rows(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "Line 1", 6);
    insertRowWithText(1, "Line 2", 6);
    insertRowWithText(2, "Line 3", 6);
    
    deleteRow(1);
    ASSERT_EQ(E.numrows, 2);
    ASSERT_STR_EQ(E.row[0].chars, "Line 1");
    ASSERT_STR_EQ(E.row[1].chars, "Line 3");
    
    deleteRow(0);
    ASSERT_EQ(E.numrows, 1);
    ASSERT_STR_EQ(E.row[0].chars, "Line 3");
    
    deleteRow(0);
    ASSERT_EQ(E.numrows, 0);
    
    teardown();
}

void test_row_bounds_safety(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "Safe", 4);
    
    deleteRow(-1);
    deleteRow(5);
    ASSERT_EQ(E.numrows, 1);
    
    insertRowWithText(-1, "Bad", 3);
    insertRowWithText(50, "Bad", 3);
    ASSERT_EQ(E.numrows, 1);
    
    teardown();
}

int main(void) {
    printf("\n--- Running Row Operations Tests ---\n");
    RUN_TEST(test_insert_empty_rows);
    RUN_TEST(test_insert_row_with_text);
    RUN_TEST(test_delete_rows);
    RUN_TEST(test_row_bounds_safety);
    
    TEST_SUMMARY();
    return (tests_run == tests_passed) ? 0 : 1;
}
