#include "rune.h"

void loadRCFile(void)
{
    const char *home = getenv("HOME");
    if (home == NULL) return;

    size_t len = strlen(home) + strlen("/.runerc") + 1;
    char *path = xmalloc(len);

    snprintf(path, len, "%s/.runerc", home);

    FILE* file = fopen(path, "r");

    if (file == NULL)
    {
        file = fopen(path, "w");

        if (file != NULL) {
            fclose(file);
        }
        free(path);
        return;
    }
    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        char command[32];
        char option[64];

        if (sscanf(line, "%31s %63s", command, option) != 2)
            continue;

        if (strcmp(command, "set") == 0) {

            if (strcmp(option, "nu") == 0 ||
                strcmp(option, "number") == 0) {

                E.showLineNumbers = 1;

            } else if (strcmp(option, "rnu") == 0 ||
                       strcmp(option, "relativenumber") == 0) {

                E.showRLineNumbers = 1;
            }

        } else if (strcmp(command, "theme") == 0) {

            if (strcasecmp(option, "none") == 0) {
                E.theme = THEME_NONE;

            } else if (strcasecmp(option, "astra") == 0) {
                E.theme = THEME_ASTRA;

            } else if (strcasecmp(option, "clouds") == 0) {
                E.theme = THEME_CLOUDS;

            } else if (strcasecmp(option, "green") == 0) {
                E.theme = THEME_GREEN;
            }
        } else if (strcmp(command, "tab_size") == 0) {
        
            int val = atoi(option);
            if (val > 0 && val < 16)
            {
                E.tabSize = val;
            }
            
        } else if (strcmp(command, "status_line") == 0) {

            if (strcasecmp(option, "minimal") == 0) {
                E.statusSize = STATUS_MINIMAL;

            } else if (strcasecmp(option, "full") == 0) {
                E.statusSize = STATUS_FULL;

            } else if (strcasecmp(option, "off") == 0) {
                E.statusSize = STATUS_OFF;
            }

        } else if (strcmp(command, "autoindent") == 0) {
            if (strcasecmp(option, "off") == 0) {
                E.autoindent = false;

            } else if (strcasecmp(option, "on") == 0) {
                E.autoindent = true;
            }
        } else if (strcmp(command, "syntax") == 0) {
            if (strcmp(option, "off") == 0) {
                E.syntax = false;

            } else if (strcmp(option, "on")  == 0) {
                E.syntax = true;
            }
        } else if (strcmp(command, "ignorecase")  == 0) {
            if (strcmp(option, "on") == 0)
            {
                E.ignoreCase = true;
            } else if (strcmp(option, "off") == 0) {
                E.ignoreCase = false;
            }
        }
    }

    fclose(file);
    free(path);
}
