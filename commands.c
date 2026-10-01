#include "rune.h"

bool ex_q(const char* args)
{
    (void) args;
    if (E.dirty > 0)
    {
        const char* message = "Unsaved changes. To quit without saving write :q!";
        showMessageAtCommandLine(message, strlen(message));

        return false;
    }

    else
    {
        processKey(CTRL_KEY('q'));

        return true;
    }
}

bool ex_qforce(const char* args)
{
    (void) args;
    processKey(CTRL_KEY('q'));

    return true;
}

bool ex_qsave(const char* args)
{
    char* old_filepath = E.filepath ? xstrdup(E.filepath) : NULL;

    if (args != NULL && *args != '\0') {
        char *target_path = expandPath(args);
        if (target_path)
        {
            editorSetFilename(target_path);
            free(target_path);
        }
    }

    if (savefile())
    {
        processKey(CTRL_KEY('q'));
    } else {
        if (old_filepath) {
            editorSetFilename(old_filepath);
        } else {
            if (E.filename) {
                free(E.filename);
                E.filename = NULL;
            }
            if (E.filepath) {
                free(E.filepath);
                E.filepath = NULL;
            }
        }
        if (old_filepath) free(old_filepath);
        return false;
    }

    if (old_filepath) free(old_filepath);

    return true;
}

bool ex_pwd(const char* args)
{
    (void) args;
    char buf[1024];
    if (getcwd(buf, sizeof(buf)) != NULL) {
        showMessageAtCommandLine(buf, strlen(buf));
    } else {
        const char* errormessage = "Unknown error";
        showMessageAtCommandLine(errormessage, strlen(errormessage));

        return false;
    }

    return true;
}

bool ex_info(const char* args)
{
    (void) args;
    long size = -1;
        
    if (E.filename != NULL)
    {
        FILE *fp = fopen(E.filepath, "rb");
        if (fp != NULL)
        {
            
            if (fseek(fp, 0, SEEK_END) == 0) 
            {
                size = ftell(fp);
            }

            fclose(fp);
        }
    }

    char size_buf[32];
    if (size == -1) {
        strcpy(size_buf, "? bytes");
    } else {
        snprintf(size_buf, sizeof(size_buf), "%ld bytes", size);
    }

    char buf[128];
    snprintf(buf, sizeof(buf),
        "File: %s, Number of rows: %d, Cursor pos: x: %d, y: %d, size: %s%s",
        E.filename ? E.filename : "[No Name]",
        E.numrows,
        E.cx + 1,
        E.cy + 1,
        size_buf,
        E.dirty > 0 ? " [MODIFIED]" : ""
    );

    showMessageAtCommandLine(buf, strlen(buf));

    return true;
}

bool ex_w(const char* args)
{
    char* old_filepath = E.filepath ? xstrdup(E.filepath) : NULL;

    if ((args == NULL || *args == '\0') && E.filepath == NULL)
    {
        const char* s = "This file doesn't have a name";
        showMessageAtCommandLine(s, strlen(s));
        return false;
    }

    if (args != NULL && *args != '\0')
    {
        char *target_path = expandPath(args);
        if (target_path)
        {
            editorSetFilename(target_path);
            free(target_path);
        }
    }

    if (!savefile()) {
        if (old_filepath) {
            editorSetFilename(old_filepath);
        } else {
            if (E.filename) { free(E.filename); E.filename = NULL; }
            if (E.filepath) { free(E.filepath); E.filepath = NULL; }
        }

        if (old_filepath) free(old_filepath);
        return false;
    }

    if (old_filepath) free(old_filepath);

    return true;
}

bool ex_e(const char* args)
{
    if (E.dirty > 0) {
        const char* msg = "No write since last change";
        showMessageAtCommandLine(msg, strlen(msg));
        return false;
    }

    if (args == NULL || *args == '\0')
    {
        if (E.filename == NULL)
        {
            const char* errormessage = "No file name: Usage :e file_name";
            showMessageAtCommandLine(errormessage, strlen(errormessage));
            return false;
        }

        else
        {
            openfile(E.filepath);
        }
    }

    else
    {
        char *target_path = expandPath(args);
        if (target_path)
        {
            openfile(target_path);
            free(target_path);
        }
    }

    return true;
}

bool ex_set(const char* args)
{
    if (args == NULL || *args == '\0') {
        const char* msg = "Usage: :set <option>";
        showMessageAtCommandLine(msg, strlen(msg));
        return false;
    }

    if ((strncmp(args, "nu", 2) == 0  && (args[2] == ' ' || args[2] == '\0')) || 
        ((strncmp(args, "number", 6) == 0) && (args[6] == ' ' || args[6] == '\0')))
    {
        E.showLineNumbers = true;
    }

    else if ((strncmp(args, "nonu", 4) == 0  && (args[4] == ' ' || args[4] == '\0')) || 
        ((strncmp(args, "nonumber", 8) == 0) && (args[8] == ' ' || args[8] == '\0')))
    {
        E.showLineNumbers = false;
    }

    else if ((strncmp(args, "number!", 7)) == 0  && (args[7] == ' ' || args[7] == '\0'))
    {
        E.showLineNumbers = !E.showLineNumbers;
    }

    else if ((strncmp(args, "rnu", 3) == 0  && (args[3] == ' ' || args[3] == '\0')) || 
        ((strncmp(args, "relativenumber", 14) == 0) && (args[14] == ' ' || args[14] == '\0')))
    {
        E.showRLineNumbers = true;
    }

    else if ((strncmp(args, "nornu", 5) == 0  && (args[5] == ' ' || args[5] == '\0')) || 
        ((strncmp(args, "norelativenumber", 16) == 0) && (args[16] == ' ' || args[16] == '\0')))
    {
        E.showRLineNumbers = false;
    }

    else if ((strncmp(args, "relativenumber!", 15)) == 0  && (args[15] == ' ' || args[15] == '\0'))
    {
        E.showRLineNumbers = !E.showRLineNumbers;
    }

    return true;
}

bool ex_bang(const char* args)
{
    disableAltBuff();
    disableRawMode();

    printf("\r\n[Rune] executing: %s \r\n\r\n", args);

    int status = system(args);

    if (status == -1)
    {
        enableRawMode();
        enableAltBuff();
        const char* msg = "Failed to run a command";
        showMessageAtCommandLine(msg, strlen(msg));
        return false;
    }


    printf("\r\n[Rune] Press [Any key] to return to editor...\r\n");
    fflush(stdout);

    enableRawMode();
    readKey();

    enableAltBuff();
    
    const char* msg = "Command executed successfully";
    showMessageAtCommandLine(msg, strlen(msg));

    return true;
}
