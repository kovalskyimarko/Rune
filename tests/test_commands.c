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

void test_unknown_command(void) {
    initEditorHeadless();
    parseCommand(":makecoffee");
    ASSERT_STR_EQ(E.statusmsg, "Unknown command");
    teardown();
}

void test_command_whitespace(void) {
    initEditorHeadless();
    
    E.statusmsg[0] = '\0';
    parseCommand("   ");
    ASSERT_STR_EQ(E.statusmsg, "");
    
    parseCommand("    :nonexistent");
    ASSERT_STR_EQ(E.statusmsg, "Unknown command");

    teardown();
}

void test_safe_commands(void) {
    initEditorHeadless();
    
    E.statusmsg[0] = '\0';
    parseCommand(":info");
    
    if (strcmp(E.statusmsg, "Unknown command") == 0) {
        FAIL("Expected :info to be recognized");
    }
    
    ASSERT_EQ(1, 1); 
    teardown();
}

void test_save_unnamed(void) {
    initEditorHeadless();
    E.filepath = NULL;
    parseCommand(":w");
    ASSERT_STR_EQ(E.statusmsg, "This file doesn't have a name");
    teardown();
}

int main(void) {
    printf("\n--- Running Command Parsing Tests ---\n");
    RUN_TEST(test_unknown_command);
    RUN_TEST(test_command_whitespace);
    RUN_TEST(test_safe_commands);
    RUN_TEST(test_save_unnamed);
    
    TEST_SUMMARY();
    return (tests_run == tests_passed) ? 0 : 1;
}
