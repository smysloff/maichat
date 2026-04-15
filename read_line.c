
// file: read_line.c
// author: ishimai

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _MCS_VER
# define strdup _strdup
#endif

#define BUFFER_SIZE 4096

char *
read_line(int fd)
{
  char *result, *tmp;
  char buffer[BUFFER_SIZE];
  size_t total;
  ssize_t bytes;

  if (fd < 0)
    return NULL;

  result = NULL;
  total = 1;

  while (true)
  {
    bytes = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes == -1)
    {
      free(result);
      return NULL;
    }

    if (bytes == 0)
      return result ? result : strdup("");

    buffer[bytes] = '\0';

    if (!(tmp = realloc(result, total + (size_t) bytes)))
    {
      free(result);
      return NULL;
    }

    result = tmp;
    memcpy(&result[total - 1], buffer, (size_t) bytes);
    total += (size_t) bytes;
    result[total] = '\0';
  }

  return result;
}

int
main(void)
{
  char *line;

  if (!(line = read_line(STDIN_FILENO)))
    return EXIT_FAILURE;

  printf("[%s]" "\n", line);
  free(line);

  return EXIT_SUCCESS;
}
