#include "rune.h"

const char *path_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

int expand_path(char *dest, size_t dest_len, const char *input) {
    if (input[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) return 0;
        snprintf(dest, dest_len, "%s%s", home, input + 1);
    } else {
        snprintf(dest, dest_len, "%s", input);
    }
    return 1;
}

void editor_set_filename(const char *path) {
    if (E.filepath) free(E.filepath);
    if (E.filename) free(E.filename);

    E.filepath = strdup(path);
    E.filename = strdup(path_basename(path));
}

void editor_clear_buffer(void) {
    for (int i = 0; i < E.numrows; i++) {
        free(E.row[i].chars);
    }
    free(E.row);
    E.row = NULL;
    E.numrows = 0;
    E.cx = 0;
    E.cy = 0;
}

void savefile(void) {
    char fullpath[1024];
    const char *target_path = NULL;

    if (E.lastrow && E.lastrow->len > 3) {
        if (!expand_path(fullpath, sizeof(fullpath), &E.lastrow->chars[3])) 
            return;
        
        target_path = fullpath;
        
        editor_set_filename(fullpath);
        if (!E.filepath) return;
    } 
    
    else {
        if (!E.filepath) return;
        target_path = E.filepath;
    }

    FILE *fptr = fopen(target_path, "w");
    if (!fptr) return;

    for (int i = 0; i < E.numrows; i++) {
        fputs(E.row[i].chars, fptr);
        fputc('\n', fptr);
    }

    fclose(fptr);
}

void openfile(void) {
    char fullpath[1024];

    if (!E.lastrow || E.lastrow->len <= 3)
        return;

    if (!expand_path(fullpath, sizeof(fullpath), &E.lastrow->chars[3]))
        return;

    FILE *fp = fopen(fullpath, "r");
    if (!fp) return;

    editor_clear_buffer();

    editor_set_filename(fullpath);
    if (!E.filepath || !E.filename) {
        fclose(fp);
        return;
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t len;

    while ((len = getline(&line, &cap, fp)) != -1) {
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            len--;

        insertRowWithText(E.numrows, line, len);
    }

    free(line);
    fclose(fp);
}
