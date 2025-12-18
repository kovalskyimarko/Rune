#include "rune.h"


const char *path_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}


void savefile(void) {
    char fullpath[1024];

    if (E.lastrow && E.lastrow->len > 3) {
        const char *input = &E.lastrow->chars[3];

        /* Expand ~ */
        if (input[0] == '~') {
            const char *home = getenv("HOME");
            if (!home) return;

            snprintf(fullpath, sizeof(fullpath),
                     "%s%s", home, input + 1);
        } else {
            snprintf(fullpath, sizeof(fullpath),
                     "%s", input);
        }

        free(E.filepath);
        E.filepath = strdup(fullpath);
        if (!E.filepath) return;

        free(E.filename);
        E.filename = strdup(path_basename(fullpath));
        if (!E.filename) return;
    }
    else {
        if (!E.filepath) return;
        snprintf(fullpath, sizeof(fullpath), "%s", E.filepath);
    }

    FILE *fptr = fopen(fullpath, "w");
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

    const char *input = &E.lastrow->chars[3];

    /* Expand ~ */
    if (input[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) return;

        snprintf(fullpath, sizeof(fullpath),
                 "%s%s", home, input + 1);
    } else {
        snprintf(fullpath, sizeof(fullpath),
                 "%s", input);
    }

    FILE *fp = fopen(fullpath, "r");
    if (!fp) return;

    for (int i = 0; i < E.numrows; i++)
        free(E.row[i].chars);
    free(E.row);

    E.row = NULL;
    E.numrows = 0;
    E.cx = 0;
    E.cy = 0;

    free(E.filepath);
    E.filepath = strdup(fullpath);

    free(E.filename);
    E.filename = strdup(path_basename(fullpath));

    if (!E.filepath || !E.filename) {
        fclose(fp);
        return;
    }

    /* Read file */
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;

    while ((len = getline(&line, &cap, fp)) != -1) {
        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            len--;

        insertRowWithText(E.numrows, line, len);
    }

    free(line);
    fclose(fp);
}
