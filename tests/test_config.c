#include "../rune.h"
#include "test.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

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
    
    E.showLineNumbers = 0;
    E.showRLineNumbers = 0;
    E.theme = THEME_NONE;
    E.tabSize = 4;
    E.statusSize = STATUS_FULL;
    E.autoindent = true;
    E.syntax = false;
    
    E.isUndoing = false;
}

static char temp_home[] = "/tmp/rune_test_XXXXXX";

void setup_sandbox(void) {
    if (mkdtemp(temp_home) == NULL) {
        perror("mkdtemp failed");
        exit(1);
    }
    setenv("HOME", temp_home, 1);
}

void teardown_sandbox(void) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.runerc", temp_home);
    unlink(path);
    rmdir(temp_home);
}

void write_runerc(const char *content) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.runerc", temp_home);
    FILE *f = fopen(path, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

void test_missing_runerc(void) {
    initEditorHeadless();
    
    char path[512];
    snprintf(path, sizeof(path), "%s/.runerc", temp_home);
    unlink(path);
    
    loadRCFile();
    
    FILE *f = fopen(path, "r");
    ASSERT_EQ((f != NULL), 1);
    if (f) fclose(f);
}

void test_valid_config(void) {
    initEditorHeadless();
    
    write_runerc(
        "set nu\n"
        "theme astra\n"
        "tab_size 8\n"
        "status_line minimal\n"
        "autoindent off\n"
        "syntax on\n"
    );
    
    loadRCFile();
    
    ASSERT_EQ(E.showLineNumbers, 1);
    ASSERT_EQ(E.theme, THEME_ASTRA);
    ASSERT_EQ(E.tabSize, 8);
    ASSERT_EQ(E.statusSize, STATUS_MINIMAL);
    ASSERT_EQ(E.autoindent, false);
    ASSERT_EQ(E.syntax, true);
}

void test_invalid_config(void) {
    initEditorHeadless();
    
    write_runerc(
        "set unknown_option\n"
        "theme nonexistent\n"
        "tab_size 999\n"
        "tab_size -5\n"
        "just_some_random_text without args\n"
        "status_line magic\n"
    );
    
    loadRCFile();
    
    ASSERT_EQ(E.showLineNumbers, 0);
    ASSERT_EQ(E.theme, THEME_NONE);
    ASSERT_EQ(E.tabSize, 4);
    ASSERT_EQ(E.statusSize, STATUS_FULL);
}

int main(void) {
    setup_sandbox();
    
    printf("\n--- Running Config Parser Tests ---\n");
    RUN_TEST(test_missing_runerc);
    RUN_TEST(test_valid_config);
    RUN_TEST(test_invalid_config);
    
    TEST_SUMMARY();
    
    teardown_sandbox();
    return (tests_run == tests_passed) ? 0 : 1;
}
