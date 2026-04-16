
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef _MSC_VER
# define strdup _strdup
#endif

#define READ_CHUNK_SIZE 4096


char *read_fd(int fd);
char *readn_fd(int fd, size_t size);
char *read_line_fd(int fd);

int
main(void)
{
  char *line;

  line = read_line_fd(STDIN_FILENO);
  printf("[%s]" "\n", line ? line : "(null)");
  free(line);

  return EXIT_SUCCESS;
}


char *read_line_fd(int fd) {

  char *result = NULL;
  char *tmp;
  char buffer[READ_CHUNK_SIZE];
  size_t total = 0;
  size_t len;
  ssize_t bytes;
  int found_newline = 0;

  if (fd < 0) {
    errno = EBADF;
    return NULL;
  }

  while (!found_newline) {
    do {
      bytes = read(fd, buffer, sizeof(buffer) - 1);
    } while (bytes == -1 && errno == EINTR);

    if (bytes == -1) {
      free(result);
      return NULL;
    }

    if (bytes == 0)
      break;

    buffer[bytes] = '\0';

    if ((tmp = memchr(buffer, '\n', bytes))) {
      *tmp = '\0';
      if (tmp - buffer > 1)
      {
        --tmp;
        if (*tmp == '\r')
          *tmp = '\0';
        found_newline = true;
      }
    }
    len = strlen(buffer);

    if (!(tmp = realloc(result, total + len + 1))) {
      free(result);
      return NULL;
    }

    memcpy(&result[total], buffer, len + 1); // copy with \0
    if (found_newline)
      break;
    total += len;
  }

  return result ? result : calloc(1, 1);
}

char *read_fd(int fd) {

  char *result = NULL;
  char *tmp;
  char buffer[READ_CHUNK_SIZE];
  size_t total = 0;
  ssize_t bytes;

  if (fd < 0) {
    errno = EBADF;
    return NULL;
  }

  while (true) {
    do {
      bytes = read(fd, buffer, sizeof(buffer));
    } while (bytes == -1 && errno == EINTR);

    if (bytes == -1) {
      free(result);
      return NULL;
    }

    if (bytes == 0)
      break;

    if (!(tmp = realloc(result, total + bytes + 1))) {
      free(result);
      return NULL;
    }
    result = tmp;

    memcpy(&result[total], buffer, bytes);
    total += bytes;
    result[total] = '\0';
  }

  return result ? result : calloc(1, 1);
}
