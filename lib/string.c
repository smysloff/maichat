
// file: lib/string.c

#include "string.h"

char *char_replace(char *s, char search, char replace, size_t count)
{
  if (!s)
    return NULL;

  if (!count)
    count = SIZE_MAX;

  for (size_t i = 0; s[i] && count; ++i) {
    if (s[i] == search) {
      s[i] = replace;
      --count;
    }
  }

  return s;
}
