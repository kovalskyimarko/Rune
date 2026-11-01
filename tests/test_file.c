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
    E.lastcx = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filepath = NULL;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.mode = NORMAL_MODE;
    E.autoindent = false;
    E.tabSize = 4;
    E.syntax = false;
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

    if (E.filepath) {
        free(E.filepath);
        E.filepath = NULL;
    }
    if (E.filename) {
        free(E.filename);
        E.filename = NULL;
    }
}

static char temp_dir[] = "/tmp/rune_file_test_XXXXXX";

void setup_sandbox(void) {
    if (mkdtemp(temp_dir) == NULL) {
        perror("mkdtemp failed");
        exit(1);
    }
    setenv("HOME", temp_dir, 1);
}

void teardown_sandbox(void) {
    char path[512];
    snprintf(path, sizeof(path), "%s/saved.txt", temp_dir);
    unlink(path);
    snprintf(path, sizeof(path), "%s/crlf.txt", temp_dir);
    unlink(path);
    snprintf(path, sizeof(path), "%s/newfile.txt", temp_dir);
    unlink(path);
    rmdir(temp_dir);
}

void test_expand_path(void) {
    initEditorHeadless();

    setenv("HOME", temp_dir, 1);
    char *expanded = expandPath("~/notes.txt");
    char expected[512];
    snprintf(expected, sizeof(expected), "%s/notes.txt", temp_dir);
    ASSERT_STR_EQ(expanded, expected);
    free(expanded);

    char *rel = expandPath("folder/file.c");
    ASSERT_STR_EQ(rel, "folder/file.c");
    free(rel);

    char *abs_p = expandPath("/usr/local/include/rune.h");
    ASSERT_STR_EQ(abs_p, "/usr/local/include/rune.h");
    free(abs_p);

    unsetenv("HOME");
    char *no_home = expandPath("~/fallback.txt");
    ASSERT_STR_EQ(no_home, "~/fallback.txt");
    free(no_home);

    setenv("HOME", temp_dir, 1);
    teardown();
}

void test_editor_set_filename(void) {
    initEditorHeadless();

    editorSetFilename("/path/to/project/main.c");
    ASSERT_STR_EQ(E.filepath, "/path/to/project/main.c");
    ASSERT_STR_EQ(E.filename, "main.c");

    editorSetFilename("README.md");
    ASSERT_STR_EQ(E.filepath, "README.md");
    ASSERT_STR_EQ(E.filename, "README.md");

    editorSetFilename(E.filepath);
    ASSERT_STR_EQ(E.filepath, "README.md");
    ASSERT_STR_EQ(E.filename, "README.md");

    teardown();
}

void test_save_and_open_file(void) {
    initEditorHeadless();

    ASSERT_FALSE(savefile());
    ASSERT_STR_EQ(E.statusmsg, "Error: No file name");

    char path[512];
    snprintf(path, sizeof(path), "%s/saved.txt", temp_dir);
    editorSetFilename(path);

    insertRowWithText(0, "First line", 10);
    insertRowWithText(1, "Second line", 11);
    insertRowWithText(2, "", 0);
    insertRowWithText(3, "Fourth line", 11);
    E.dirty = 4;

    ASSERT_TRUE(savefile());
    ASSERT_EQ(E.dirty, 0);

    insertRowWithText(4, "Unsaved extra line", 18);
    ASSERT_EQ(E.numrows, 5);

    openfile(path);
    ASSERT_EQ(E.numrows, 4);
    ASSERT_EQ(E.dirty, 0);
    ASSERT_STR_EQ(E.row[0].chars, "First line");
    ASSERT_STR_EQ(E.row[1].chars, "Second line");
    ASSERT_STR_EQ(E.row[2].chars, "");
    ASSERT_STR_EQ(E.row[3].chars, "Fourth line");

    teardown();
}

void test_open_nonexistent_and_crlf(void) {
    initEditorHeadless();

    insertRowWithText(0, "Old buffer data", 15);
    char new_path[512];
    snprintf(new_path, sizeof(new_path), "%s/newfile.txt", temp_dir);

    openfile(new_path);
    ASSERT_EQ(E.numrows, 0);
    ASSERT_EQ(E.dirty, 0);
    ASSERT_STR_EQ(E.filename, "newfile.txt");
    ASSERT_STR_EQ(E.statusmsg, "New file created");

    char crlf_path[512];
    snprintf(crlf_path, sizeof(crlf_path), "%s/crlf.txt", temp_dir);
    FILE *f = fopen(crlf_path, "wb");
    ASSERT_NOT_NULL(f);
    fputs("alpha\r\nbeta\r\ngamma\n", f);
    fclose(f);

    openfile(crlf_path);
    ASSERT_EQ(E.numrows, 3);
    ASSERT_STR_EQ(E.row[0].chars, "alpha");
    ASSERT_EQ(E.row[0].len, 5);
    ASSERT_STR_EQ(E.row[1].chars, "beta");
    ASSERT_EQ(E.row[1].len, 4);
    ASSERT_STR_EQ(E.row[2].chars, "gamma");
    ASSERT_EQ(E.row[2].len, 5);

    editorSetFilename("/tmp/nonexistent_rune_dir_12345/bad.txt");
    ASSERT_FALSE(savefile());
    ASSERT_STR_EQ(E.statusmsg, "Error: Cannot save file (permission denied?)");

    teardown();
}

void test_emergency_save(void) {
    initEditorHeadless();

    emergencySave();
    ASSERT_NULL(E.filepath);

    insertRowWithText(0, "Important work", 14);
    E.dirty = 1;

    char orig_cwd[512];
    ASSERT_NOT_NULL(getcwd(orig_cwd, sizeof(orig_cwd)));
    ASSERT_EQ(chdir(temp_dir), 0);

    emergencySave();
    ASSERT_STR_EQ(E.filename, "rune_crash_recovery.txt");
    ASSERT_EQ(E.dirty, 0);

    openfile("rune_crash_recovery.txt");
    ASSERT_EQ(E.numrows, 1);
    ASSERT_STR_EQ(E.row[0].chars, "Important work");

    unlink("rune_crash_recovery.txt");
    ASSERT_EQ(chdir(orig_cwd), 0);

    teardown();
}

int main(void) {
    setup_sandbox();

    printf("\n--- Running File Operations Tests ---\n");
    RUN_TEST(test_expand_path);
    RUN_TEST(test_editor_set_filename);
    RUN_TEST(test_save_and_open_file);
    RUN_TEST(test_open_nonexistent_and_crlf);
    RUN_TEST(test_emergency_save);

    TEST_SUMMARY();

    teardown_sandbox();
    return (tests_run == tests_passed) ? 0 : 1;
}
