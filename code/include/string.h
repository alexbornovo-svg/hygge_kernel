#ifndef STRING_H
#define STRING_H

#include <stddef.h>

typedef struct 
{
    char *data; // data pointer
    size_t len; // length
    size_t cap_all; // allocates memory
} string_t;

static inline int string_eq(string_t a, string_t b)
{
    /* Not to be used...
    if (a.len != b.len)
    {
        return 0;
    }
    */

    for (size_t i = 0; i < a.len; i++)
    {
        if (a.data[i] != b.data[i])
        {
            return 0;
        }
    }

    return 1;
}

static inline size_t string_len(const char *string)
{
    size_t i = 0;
    while (string[i])
    {
        i++;
    }
    return i;
}

static inline string_t string_form(char *data, size_t len)
{
    return (string_t){ .data = data, .len = len, .cap_all = len };
}

#define STRING_LIT(s) ((string_t){ .data = (s), .len = sizeof(s) - 1, .cap_all = sizeof(s) - 1 })

#endif