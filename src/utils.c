
// file: utils.c
// author: ishimai

#include "utils.h"

[[noreturn]] void
die(const char *msg)
{
    if (msg)
        fprintf(stderr, "%s" "\n", msg);
    exit(EXIT_FAILURE);
}

char *
smprintf(char *fmt, ...)
{
    va_list args;
    char *result;
    int len;

    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (!(result = malloc(++len)))
      return NULL;

    va_start(args, fmt);
    vsnprintf(result, len, fmt, args);
    va_end(args);

    return result;
}

char *
fgetline(FILE *stream)
{
    char *result, *ptr;
    char buffer[BUFFER_SIZE];
    size_t total, length, offset;
    int ended;

    if (!stream)
        return NULL;

    total = 0;
    ended = 0;
    result = NULL;

    while (fgets(buffer, sizeof(buffer), stream))
    {
        length = strlen(buffer);
        offset = total;

        if (length >= 1 && buffer[length - 1] == '\n')
        {
            buffer[--length] = '\0';
            ended = 1;
        }

        total += length;

        if (!(ptr = realloc(result, total + 1)))
        {
            free(result);
            return NULL;
        }
        result = ptr;

        if (length)
            memcpy(result + offset, buffer, length);
        result[total] = '\0';

        if (ended)
            break;
    }

    if (result && total >= 1 && result[total - 1] == '\r')
        result[total - 1] = '\0';

    return result;
}

char *
ltrim(const char *s)
{
    if (!s)
        return NULL;

    while (isspace((unsigned char) *s))
        ++s;

    return strdup(s);
}

char *
rtrim(const char *s)
{
    char *result;
    size_t len;

    if (!s)
        return NULL;

    if (*s == '\0')
        return strdup("");

    len = strlen(s);

    while (len > 0 && isspace((unsigned char) s[len - 1]))
        --len;

    if (!(result = malloc(len + 1)))
        return NULL;

    memcpy(result, s, len);
    result[len] = '\0';

    return result;
}

char *
trim(const char *s)
{
    char *result;
    size_t len;

    if (!s)
        return NULL;

    while (isspace((unsigned char) *s))
        ++s;

    if (*s == '\0')
        return strdup("");

    len = strlen(s);

    while (len > 0 && isspace((unsigned char) s[len - 1]))
        --len;

    if (!(result = malloc(len + 1)))
        return NULL;

    memcpy(result, s, len);
    result[len] = '\0';

    return result;
}

bool
has_spaces(const char *s)
{
    return s && strpbrk(s, " \t\n\r\v\f");
}

int
imax(int a, int b)
{
    return a < b ? b : a;
}
