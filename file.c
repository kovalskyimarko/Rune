#include "rune.h"

const char *path_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

char* expand_path(const char *input) {
    if (input[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) return strdup(input);

        size_t len = strlen(home) + strlen(input + 1) + 1;
        char *dest = malloc(len);    
        if (dest) {
            snprintf(dest, len, "%s%s", home, input + 1);
        }
        return dest;
    }
    return strdup(input);
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
    char *target_path = NULL;

    if (E.lastrow && E.lastrow->len > 3) {
        target_path = expand_path(&E.lastrow->chars[3]);
        if (!target_path) return;
        
        
        editor_set_filename(target_path);
    } 
    
    else {
        if (!E.filepath) return;
        target_path = strdup(E.filepath);
    }

    FILE *fptr = fopen(target_path, "w");
    if (!fptr) { 
        free(target_path);
        return;
    }

    for (int i = 0; i < E.numrows; i++) {
        fputs(E.row[i].chars, fptr);
        fputc('\n', fptr);
    }

    fclose(fptr);
    free(target_path);
}

void openfile(void) {
    if (!E.lastrow || E.lastrow->len <= 3) return;

    char *fullpath = expand_path(&E.lastrow->chars[3]);
    if (!fullpath) return;

    FILE *fp = fopen(fullpath, "r");
    if (!fp) {
        free(fullpath);
        return;
    }

    editor_clear_buffer();

    editor_set_filename(fullpath);
    if (!E.filepath || !E.filename) {
        fclose(fp);
        free(fullpath);
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
    free(fullpath);
}
