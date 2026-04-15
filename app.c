
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _MSC_VER
# define strdup _strdup
#endif

#define BUFFER_SIZE 4

char *read_fd(int fd);

int
main(void)
{
  char *line;

  line = read_fd(STDIN_FILENO);
  printf("[%s]" "\n", line ? line : "(null)");
  free(line);

  return EXIT_SUCCESS;
}


/**
 * Reads all data from a file descriptor into a newly allocated string.
 *
 * This function reads from the given file descriptor until EOF, dynamically
 * growing the buffer as needed. It's designed to work seamlessly with both
 * regular files, pipes, and sockets.
 *
 * @param fd Valid file descriptor (must be open for reading)
 * @return Newly allocated string containing all read data, or NULL on error.
 *         Returns an empty string (not NULL) if no data was read.
 */
char *
read_fd(int fd)
{
  char *result, *tmp;
  char buffer[BUFFER_SIZE];
  size_t total;
  ssize_t bytes;

  if (fd < 0)
    return NULL;

  result = NULL;
  total = 1; // Reserve one extra byte for the final null terminator.
                // This trick eliminates the need for a separate "+1" in realloc.

  while (true)
  {
    bytes = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes == -1)
      return NULL;

    if (bytes == 0) // return as it is or an empty string if there were no data
      return result ? result : strdup("");

    if (!(tmp = realloc(result, total + (size_t) bytes))) // termination \0 include in the begining
    {
      free(result);
      return NULL;
    }
    result = tmp;

    buffer[bytes] = '\0';
    memcpy(&result[total - 1], buffer, bytes + 1); // copy buffer data with termination \0
    total += (size_t) bytes;                       // and rewrite previous \0
  }

  return result;
}
