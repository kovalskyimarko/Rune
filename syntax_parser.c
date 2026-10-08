#include "rune.h"

ThemePalette themes[NUM_THEMES] = {
    { 
        "\x1b[49m", "\x1b[39m", "\x1b[39m", "\x1b[39m", "\x1b[39m", 
        "\x1b[39m", "\x1b[39m", "\x1b[39m", "\x1b[39m", "\x1b[39m", "\x1b[39m",
        "\x1b[47m", "\x1b[30m", "\x1b[92m", "\x1b[90m"
    },
    
    { 
        "\x1b[49m", "\x1b[37m", "\x1b[92m", "\x1b[93m", "\x1b[95m",
        "\x1b[35m", "\x1b[91m", "\x1b[94m", "\x1b[96m", "\x1b[95m", "\x1b[33m",
        "\x1b[47m", "\x1b[30m", "\x1b[96m", "\x1b[90m"
    },
    
    { 
        "\x1b[47m", "\x1b[30m", "\x1b[32m", "\x1b[31m", "\x1b[34m", 
        "\x1b[35m", "\x1b[31m", "\x1b[34m", "\x1b[36m", "\x1b[35m", "\x1b[33m",
        "\x1b[100m", "\x1b[97m", "\x1b[34m", "\x1b[90m"
    },
    
    { 
        "\x1b[40m", "\x1b[32m", "\x1b[32m", "\x1b[32m", "\x1b[32m", 
        "\x1b[32m", "\x1b[32m", "\x1b[32m", "\x1b[32m", "\x1b[32m", "\x1b[32m" ,
        "\x1b[42m", "\x1b[30m", "\x1b[92m", "\x1b[90m"
    }
};

const char *type_keywords[] = {
    "void",
    "char",
    "short",
    "int",
    "long",
    "float",
    "double",
    "signed",
    "unsigned",
    "_Bool",
    "_Complex",
    "_Imaginary",
};

const char *control_keywords[] = {
    "if",
    "else",
    "switch",
    "case",
    "default",
    "for",
    "while",
    "do",
    "break",
    "continue",
    "goto",
    "return",
};

const char *storage_keywords[] = {
    "auto",
    "extern",
    "register",
    "static",
    "typedef",
};

const char *qualifier_keywords[] = {
    "const",
    "volatile",
    "restrict",
    "_Atomic",
};

const char *type_construct_keywords[] = {
    "struct",
    "union",
    "enum",
};

const char *special_keywords[] = {
    "sizeof",
    "_Alignas",
    "_Alignof",
    "_Static_assert",
    "_Generic",
    "_Noreturn",
    "_Thread_local",
};

int isInArray(const char *str, const char *array[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (strcmp(str, array[i]) == 0)
            return 1;
    }

    return 0;
}

void parse(erow* row)
{
    if (!E.syntax) return;

    if (row == E.lastrow) {
        return;
    }

    int cy = row - E.row;
    bool inComment = (cy > 0 && E.row[cy - 1].hlOpenComment);

    if (row->len == 0) {
        if (row->hl) free(row->hl);
        row->hl = NULL;

        bool changed = (row->hlOpenComment != inComment);
        row->hlOpenComment = inComment;
        
        if (changed && cy + 1 < E.numrows) parse(&E.row[cy + 1]);
        return;
    }

    row->hl = xrealloc(row->hl, row->len);

    char* str = row->chars;

    for (int i = 0; i < row->len; i++) {
        row->hl[i] = HL_DEFAULT;
    }

    bool insideQuotes = false;
    char quoteChar = '\0';

    int i = 0;
    while (i < row->len)
    {
        char c = str[i];

        if (inComment)
        {
            row->hl[i] = HL_COMMENT;
            if (c == '*' && i < row->len - 1 && str[i+1] == '/') {
                row->hl[i+1] = HL_COMMENT;
                inComment = false;
                i += 2;
                continue;
            }
            i++;
            continue;
        }

        else if (insideQuotes)
        {
            row->hl[i] = HL_STRING;
            if (c == quoteChar) insideQuotes = false;
            i++;
            continue;
        }

        else if (c == '/' && i < row->len - 1 && str[i+1] == '/')
        {
            while (i < row->len) {
                row->hl[i] = HL_COMMENT;
                i++;
            }
            break; 
        }

        else if (c == '/' && i < row->len - 1 && str[i+1] == '*')
        {
            row->hl[i] = HL_COMMENT;
            row->hl[i+1] = HL_COMMENT;
            inComment = true;
            i += 2;
            continue;
        }

        else if (c == '"' || c == '\'')
        {
            insideQuotes = true;
            quoteChar = c;
            row->hl[i] = HL_STRING;
            i++;
            continue;
        }

        if (strchr("[](){}", c) != NULL)
        {
            row->hl[i] = HL_BRACKETS;
            i++;
            continue;
        }

        if (isdigit((unsigned char)c) && (i == 0 || !isalpha((unsigned char)str[i-1])))
        {
            row->hl[i] = HL_NUMBER;
            i++;
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_')
        {
            int startX = i;
            while (i < row->len && (isalnum((unsigned char)str[i]) || str[i] == '_')) {
                i++;
            }
            
            int wordLen = i - startX;

            if (wordLen < 32)
            {
                char buf[32];
                memcpy(buf, &str[startX], wordLen);
                buf[wordLen] = '\0';

                int hl = HL_DEFAULT;

                if (isInArray(buf, type_keywords, sizeof(type_keywords) / sizeof(type_keywords[0])))
                    hl = HL_TYPE;
                else if (isInArray(buf, control_keywords, sizeof(control_keywords) / sizeof(control_keywords[0])))
                    hl = HL_CONTROL;
                else if (isInArray(buf, storage_keywords, sizeof(storage_keywords) / sizeof(storage_keywords[0])))
                    hl = HL_STORAGE;
                else if (isInArray(buf, qualifier_keywords, sizeof(qualifier_keywords) / sizeof(qualifier_keywords[0])))
                    hl = HL_QUALIFIER;
                else if (isInArray(buf, type_construct_keywords, sizeof(type_construct_keywords) / sizeof(type_construct_keywords[0])))
                    hl = HL_STRUCT;
                else if (isInArray(buf, special_keywords, sizeof(special_keywords) / sizeof(special_keywords[0])))
                    hl = HL_SPECIAL;

                if (hl != HL_DEFAULT) {
                    for (int j = startX; j < i; j++) {
                        row->hl[j] = hl;
                    }
                }
            }
            continue;
        }

        i++;
    }

    bool changed = (row->hlOpenComment != inComment);
    row->hlOpenComment = inComment;

    if (changed && cy + 1 < E.numrows) {
        parse(&E.row[cy + 1]);
    }
}
