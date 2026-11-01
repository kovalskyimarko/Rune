#include "../rune.h"
#include "test.h"
#include <stdbool.h>

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
    E.syntax = true;
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

void test_basic_tokens(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "int x = 42;", 11);
    
    ASSERT_EQ(E.row[0].len, 11);
    ASSERT_EQ((E.row[0].hl != NULL), 1);
    
    ASSERT_EQ(E.row[0].hl[0], HL_TYPE);
    ASSERT_EQ(E.row[0].hl[1], HL_TYPE);
    ASSERT_EQ(E.row[0].hl[2], HL_TYPE);
    
    ASSERT_EQ(E.row[0].hl[3], HL_DEFAULT);
    ASSERT_EQ(E.row[0].hl[4], HL_DEFAULT);
    
    ASSERT_EQ(E.row[0].hl[8], HL_NUMBER);
    ASSERT_EQ(E.row[0].hl[9], HL_NUMBER);
    
    teardown();
}

void test_strings_and_brackets(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "printf(\"test\");", 15);
    
    ASSERT_EQ(E.row[0].hl[6], HL_BRACKETS);
    ASSERT_EQ(E.row[0].hl[7], HL_STRING);
    ASSERT_EQ(E.row[0].hl[8], HL_STRING);
    ASSERT_EQ(E.row[0].hl[12], HL_STRING);
    ASSERT_EQ(E.row[0].hl[13], HL_BRACKETS);

    teardown();
}

void test_single_line_comments(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "x = 1; // test", 14);
    
    ASSERT_EQ(E.row[0].hl[4], HL_NUMBER);
    ASSERT_EQ(E.row[0].hl[6], HL_DEFAULT);
    
    ASSERT_EQ(E.row[0].hl[7], HL_COMMENT);
    ASSERT_EQ(E.row[0].hl[8], HL_COMMENT);
    ASSERT_EQ(E.row[0].hl[9], HL_COMMENT);
    ASSERT_EQ(E.row[0].hl[13], HL_COMMENT);
    
    ASSERT_EQ(E.row[0].hlOpenComment, false);

    teardown();
}

void test_multiline_comments_chain(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "/* start", 8);
    insertRowWithText(1, "middle", 6);
    insertRowWithText(2, "end */ int", 10);
    
    ASSERT_EQ(E.row[0].hl[0], HL_COMMENT);
    ASSERT_EQ(E.row[0].hl[7], HL_COMMENT);
    ASSERT_EQ(E.row[0].hlOpenComment, true);
    
    ASSERT_EQ(E.row[1].hl[0], HL_COMMENT);
    ASSERT_EQ(E.row[1].hl[5], HL_COMMENT);
    ASSERT_EQ(E.row[1].hlOpenComment, true);
    
    ASSERT_EQ(E.row[2].hl[0], HL_COMMENT);
    ASSERT_EQ(E.row[2].hl[4], HL_COMMENT);
    ASSERT_EQ(E.row[2].hl[5], HL_COMMENT);
    
    ASSERT_EQ(E.row[2].hl[7], HL_TYPE);
    ASSERT_EQ(E.row[2].hlOpenComment, false);

    teardown();
}

int main(void) {
    printf("\n--- Running Syntax Parsing Tests ---\n");
    RUN_TEST(test_basic_tokens);
    RUN_TEST(test_strings_and_brackets);
    RUN_TEST(test_single_line_comments);
    RUN_TEST(test_multiline_comments_chain);
    
    TEST_SUMMARY();
    return (tests_run == tests_passed) ? 0 : 1;
}
