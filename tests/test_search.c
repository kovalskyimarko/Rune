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

void test_search_forward(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "hello world", 11);
    insertRowWithText(1, "find this text", 14);
    insertRowWithText(2, "hello again", 11);

    int found_y = -1;
    
    char *match = baseNextFind("this", 0, 0, 0, &found_y);
    ASSERT_EQ(found_y, 1);
    ASSERT_EQ((int)(match - E.row[1].chars), 5);

    match = baseNextFind("hello", 0, 0, 1, &found_y);
    ASSERT_EQ(found_y, 2);
    ASSERT_EQ((int)(match - E.row[2].chars), 0);

    teardown();
}

void test_search_reverse(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "first target", 12);
    insertRowWithText(1, "some noise", 10);
    insertRowWithText(2, "second target", 13);

    int found_y = -1;
    
    char *match = baseReverseFind("target", 0, 2, &found_y);
    ASSERT_EQ(found_y, 0);
    ASSERT_EQ((int)(match - E.row[0].chars), 6);

    insertRowWithText(3, "abc abc abc", 11);
    match = baseReverseFind("abc", 7, 3, &found_y); 
    
    ASSERT_EQ(found_y, 3);
    ASSERT_EQ((int)(match - E.row[3].chars), 4);

    teardown();
}

void test_search_edge_cases(void) {
    initEditorHeadless();
    
    insertRowWithText(0, "aaaaa", 5);
    
    int found_y = -1;
    char *match = baseNextFind("aa", 0, 0, 1, &found_y);
    
    ASSERT_EQ(found_y, 0);
    ASSERT_EQ((int)(match - E.row[0].chars), 1);

    match = baseNextFind("danger", 0, 0, 0, &found_y);
    ASSERT_EQ((match == NULL), 1); 
    
    teardown();
}

void test_search_empty_buffer(void) {
    initEditorHeadless();
    int found_y = -1;
    char *match = baseNextFind("test", 0, 0, 0, &found_y);
    ASSERT_NULL(match);
    ASSERT_EQ(found_y, -1);
    teardown();
}

int main(void) {
    printf("\n--- Running Search Tests ---\n");
    RUN_TEST(test_search_forward);
    RUN_TEST(test_search_reverse);
    RUN_TEST(test_search_edge_cases);
    RUN_TEST(test_search_empty_buffer);
    
    TEST_SUMMARY();
    return (tests_run == tests_passed) ? 0 : 1;
}
