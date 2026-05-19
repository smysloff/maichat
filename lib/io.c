
// file: lib/io.c

#include "io.h"

#define DEFAULT_BUFFER_CAPACITY 64


// Write exactly count bytes from buf to file descriptor fd.
// Returns the number of bytes actually written (may be less than count on closed descriptor).
// Calls die() on fatal I/O error (no return).
size_t fd_write(int fd, const void *buf, size_t count)
{
  ssize_t ret;
  size_t total = 0;
  const char *ptr = (const char *) buf;

  if (fd < 0) {
    errno = EBADF;
    die("fd_write() error");
  }

  if (!buf) {
    errno = EINVAL;
    die("fd_write() error");
  }

  while (total < count) {

    ret = write(fd, ptr + total, count - total);

    if (ret == -1) {
      if (errno == EINTR)
        continue;
      die("fd_write() error");
    }

    if (ret == 0)
      break;

    total += (size_t) ret;
  }

  return total;
}


// Write a NUL-terminated string s to file descriptor fd.
// Returns the number of bytes written.
// Calls die() on error.
size_t fd_write_s(int fd, const char *s)
{
  if (fd < 0) {
    errno = EBADF;
    die("fd_write_s() error");
  }

  if (!s) {
    errno = EINVAL;
    die("fd_write_s() error");
  }

  return fd_write(fd, s, strlen(s));
}


// Read up to count bytes into buf from file descriptor fd.
// Returns the number of bytes actually read (0 on EOF).
// Calls die() on fatal I/O error (no return).
size_t fd_read(int fd, void *buf, size_t count)
{
  ssize_t ret;
  size_t total = 0;
  char *ptr = (char *) buf;

  if (fd < 0) {
    errno = EBADF;
    die("fd_read() error");
  }

  if (!buf) {
    errno = EINVAL;
    die("fd_read() error");
  }

  while (total < count) {

    ret = read(fd, ptr + total, count - total);

    if (ret == -1) {
      if (errno == EINTR)
        continue;
      die("fd_read() error");
    }

    if (ret == 0)
      break;

    total += (size_t) ret;
  }

  return total;
}


// Read from fd until '\n' or '\r' or EOF; the terminating newline/carriage-return is not included.
// Returns a NUL-terminated malloc'd string (caller must free), or NULL if EOF was encountered before any bytes were read.
// Calls die() on I/O error (no return).
char *fd_read_l(int fd)
{
  unsigned char c;
  char *buf;
  size_t cap, len;
  ssize_t ret;

  if (fd < 0) {
    errno = EBADF;
    die("fd_read_l() error");
  }

  cap = DEFAULT_BUFFER_CAPACITY;
  buf = xmalloc(cap + 1);

  for (len = 0; true; ++len) {

    ret = read(fd, &c, 1);

    if (ret == -1) {
      if (errno == EINTR)
        continue;
      free(buf);
      die("fd_read_l() error");
    }

    if (ret == 0) {
      if (!len) {
        free(buf);
        return NULL;
      }
      break;
    }

    if ((char) c == '\n' || (char) c == '\r')
      break;

    if (len == cap) {
      if (cap > SIZE_MAX / 2) {
        free(buf);
        errno = ENOMEM;
        die("fd_read_l() error");
      }
      cap *= 2;
      buf = xrealloc(buf, cap + 1);
    }

    buf[len] = c;
  }

  // null-termination & shrink to fit
  buf[len] = '\0';
  return xrealloc(buf, len + 1);
}

// Read from fd until EOF.
// Returns a NUL-terminated malloc'd string (caller must free), or NULL if EOF was encountered before any bytes were read.
// Calls die() on I/O error (no return).
char *fd_read_s(int fd)
{
  unsigned char c;
  char *buf;
  size_t cap, len;
  ssize_t ret;

  if (fd < 0) {
    errno = EBADF;
    die("fd_read_s() error");
  }

  cap = DEFAULT_BUFFER_CAPACITY;
  buf = xmalloc(cap + 1);

  for (len = 0; true; ++len) {

    ret = read(fd, &c, 1);

    if (ret == -1) {
      if (errno == EINTR)
        continue;
      free(buf);
      die("fd_read_s() error");
    }

    if (ret == 0) {
      if (!len) {
        free(buf);
        return NULL;
      }
      break;
    }

    if (len == cap) {
      if (cap > SIZE_MAX / 2) {
        free(buf);
        errno = ENOMEM;
        die("fd_read_s() error");
      }
      cap *= 2;
      buf = xrealloc(buf, cap + 1);
    }

    buf[len] = c;
  }

  // null-termination & shrink to fit
  buf[len] = '\0';
  return xrealloc(buf, len + 1);
}


// Write exactly count bytes from buf to FILE* stream.
// Returns the number of bytes written.
// Calls die() on fatal I/O error (no return).
size_t file_write(FILE *stream, const void *buf, size_t count)
{
  size_t ret;
  size_t len = 0;
  const char *ptr = buf;

  if (!stream || !buf) {
    errno = EINVAL;
    die("file_write() error");
  }

  while (len < count) {

    errno = 0; // for checking EINTR
    ret = fwrite(ptr + len, 1, count - len, stream);

    if (ret > 0) {
      len += ret;
      continue;
    }

    if (ferror(stream) && errno == EINTR) {
      clearerr(stream);
      continue;
    } // else: fallthrough to die()

    die("file_write() error");
  }

  return len;
}


// Write a NUL-terminated string s to FILE* stream.
// Returns the number of bytes written.
// Calls die() on error.
size_t file_write_s(FILE *stream, const char *s)
{
  if (!s || !stream) {
    errno = EINVAL;
    die("fputs() error");
  }
  return file_write(stream, s, strlen(s));
}


// Read up to count bytes into s from stream.
// Returns the number of bytes actually read.
// If EOF is encountered before count bytes are read, returns the bytes read so far.
// On fatal I/O error the function calls die() (no return).
size_t file_read(FILE *stream, void *s, size_t count)
{
  size_t ret;
  size_t len = 0;
  char *p = (char *) s;

  if (!s || !stream) {
    errno = EINVAL;
    die("file_read() error");
  }

  while (len < count) {

    errno = 0; // for checking EINTR
    ret = fread(p + len, 1, count - len, stream);

    if (ret > 0) {
      len += ret;
      continue;
    }

    if (feof(stream))
      return len;

    if (ferror(stream) && errno == EINTR) {
      clearerr(stream);
      continue;
    } // else: fallthrough to die()

    die("file_read() error");
  }

  return len;
}


// Read from stream until '\n' or '\r' or EOF.
// The terminating newline/carriage-return is not included in the returned string.
// Returns a NUL-terminated malloc'd string (caller must free), or NULL if 0 bytes read before EOF.
// On I/O error the function calls die() (no return).
char *file_read_l(FILE *stream)
{
  int c;
  char *buf;
  size_t cap, len;

  if (!stream) {
    errno = EINVAL;
    die("file_read_l() error");
  }

  cap = DEFAULT_BUFFER_CAPACITY;
  buf = xmalloc(cap + 1);

  for (len = 0; true; ++len) {

    errno = 0;
    c = fgetc(stream);

    if (c == EOF) {

      if (feof(stream)) {
        if (!len) {
          free(buf);
          return NULL;
        }
        break;
      }

      if (ferror(stream) && errno == EINTR) {
        clearerr(stream);
        continue;
      }

      free(buf);
      die("file_read_l() error");
    }

    if (c == '\n' || c == '\r')
      break;

    if (len == cap) {
      if (cap > SIZE_MAX / 2) {
        free(buf);
        errno = ENOMEM;
        die("file_read_l() error");
      }
      cap *= 2;
      buf = xrealloc(buf, cap + 1);
    }

    buf[len] = (char) c;
  }

  // null-termination & shrink to fit
  buf[len] = '\0';
  return xrealloc(buf, len + 1);
}


// Read from stream until EOF.
// Returns a NUL-terminated malloc'd string (caller must free), or NULL if 0 bytes read before EOF.
// On I/O error the function calls die() (no return).
char *file_read_s(FILE *stream)
{
  int c;
  char *buf;
  size_t cap, len;

  if (!stream) {
    errno = EINVAL;
    die("file_read_s() error");
  }

  cap = DEFAULT_BUFFER_CAPACITY;
  buf = xmalloc(cap + 1);

  for (len = 0; true; ++len) {

    errno = 0;
    c = fgetc(stream);

    if (c == EOF) {

      if (feof(stream)) {
        if (!len) {
          free(buf);
          return NULL;
        }
        break;
      }

      if (ferror(stream) && errno == EINTR) {
        clearerr(stream);
        continue;
      }

      free(buf);
      die("file_read_s() error");
    }

    if (len == cap) {
      if (cap > SIZE_MAX / 2) {
        free(buf);
        errno = ENOMEM;
        die("file_read_s() error");
      }
      cap *= 2;
      buf = xrealloc(buf, cap + 1);
    }

    buf[len] = (char) c;
  }

  // null-termination & shrink to fit
  buf[len] = '\0';
  return xrealloc(buf, len + 1);
}
