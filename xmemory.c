#include "rune.h"

void* xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        emergencySave();
        error("malloc");
    }

    return ptr;
}

void* xrealloc(void* ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
    {
        emergencySave();
        error("realloc");   
    }

    return new_ptr;
}

char* xstrdup(const char* str)
{
    char *new_str = strdup(str);
    if (!new_str)
    {
        emergencySave();
        error("strdup");
    }

    return new_str;
}
