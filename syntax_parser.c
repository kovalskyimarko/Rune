#include "rune.h"
#include <ctype.h>

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

typedef struct token
{
    char* str;
    int startX;
    int endX;
    size_t len;
    size_t capacity;
} token;

void parse(erow* row)
{
    if (row->len == 0) {
        if (row->hl) free(row->hl);
        row->hl = NULL;
        return;
    }

    row->hl = realloc(row->hl, row->len);
    if (!row->hl) return;

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

        if (insideQuotes)
        {
            row->hl[i] = HL_STRING;
            if (c == quoteChar) insideQuotes = false;
            i++;
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
}
