
// file: main.c
// author: ishimai

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define INITIAL_BUFFER_SIZE 8


char *read_line_fd(int fd);


int main(void)
{
  char *line;
  line = read_line_fd(STDIN_FILENO);
  printf("[%s]", line ? line : "(null)");
  return EXIT_SUCCESS;
}


/**
 * Reads a single line from a file descriptor.
 *
 * Reads characters one by one until EOF, newline (\n), or carriage return (\r)
 * is encountered. The line terminator is not included in the returned string.
 * The buffer grows dynamically as needed, starting from INITIAL_BUFFER_SIZE.
 *
 * @param fd Valid file descriptor to read from
 * @return Newly allocated string containing the line (may be empty),
 *         or NULL on error (with errno set appropriately)
 */
char *read_line_fd(int fd)
{

  // Final string to return
  char *result;


  // Growing buffer for accumulating characters, and a temporary pointer for realloc
  char *buffer, *tmp;


  // Single byte read from the file descriptor
  char ch;


  // Result from read() system call
  ssize_t bytes_read;


  // Current allocated size of the buffer (starts small, doubles when full)
  size_t buffer_size = INITIAL_BUFFER_SIZE;


  // Number of characters accumulated in the buffer (excluding the terminator)
  size_t data_len = 0;


  // Validate the file descriptor
  if (fd < 0) {
    errno = EBADF;
    return NULL;
  }


  // Allocate the initial buffer
  buffer = malloc(buffer_size);
  if (!buffer)
    return NULL;


  // Read characters until we hit EOF or a line terminator
  while (1) {


    // Read one byte, retrying if interrupted by a signal
    do {
      bytes_read = read(fd, &ch, 1);
    } while (bytes_read == -1 && errno == EINTR);


    // Handle read error
    if (bytes_read == -1) {
      free(buffer);
      return NULL;
    }


    // Stop reading on EOF, newline, or carriage return
    // Note: Neither '\n' nor '\r' is stored in the buffer
    if (bytes_read == 0 || ch == '\n' || ch == '\r')
      break;


    // Store the character and advance the counter
    buffer[data_len++] = ch;


    // If the buffer is full, double its size
    if (data_len == buffer_size) {
      buffer_size *= 2;
      tmp = realloc(buffer, buffer_size);
      if (!tmp) {
        free(buffer);
        return NULL;
      }
      buffer = tmp;
    }
  }


  // No characters were read (empty input)
  if (data_len == 0) {
    free(buffer);
    return calloc(1, 1); // Return an empty string
  }


  // Allocate exactly enough memory for the result
  result = malloc(data_len + 1);
  if (!result) {
    free(buffer);
    return NULL;
  }


  // Copy the accumulated data and null-teminate
  memcpy(result, buffer, data_len);
  result[data_len] = '\0';


  // Clean up the temporary buffer
  free(buffer);


  // Exit success
  return result;
}
