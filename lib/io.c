
// file: lib/io.c

#include "io.h"

#define DEFAULT_BUFFER_CAPACITY 64

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

char *fd_read_l(int fd)
{
  char c;
  char *buf, *tmp;
  size_t cap, len;
  ssize_t ret;

  if (fd < 0) {
    errno = EBADF;
    die("fd_read_l() error");
  }

  cap = DEFAULT_BUFFER_CAPACITY;
  if (!(buf = malloc(cap + 1))) // @todo malloc_or_die()
    die("fd_read_l() error");
  buf[0] = '\0';

  for (len = 0; true; ++len) {

    ret = read(fd, &c, 1);

    if (ret == -1) {
      if (errno == EINTR)
        continue;
      free(buf);
      die("fd_read_l() error");
    }

    if (ret == 0 || c == '\r' || c == '\n')
      break;

    if (len == cap) {
      if (cap > SIZE_MAX / 2) {
        free(buf);
        errno = ENOMEM;
        die("fd_read_l() error");
      }
      cap *= 2;
      if (!(tmp = realloc(buf, cap + 1))) {
        free(buf);
        die("fd_read_l() error");
      }
      buf = tmp;
    }

    buf[len] = c;
    buf[len + 1] = '\0';
  }

  // shrink to fit
  tmp = realloc(buf, len + 1);
  return tmp ? tmp : buf;
}

char *fd_read_s(int fd)
{
  char c;
  char *buf, *tmp;
  size_t cap, len;
  ssize_t ret;

  if (fd < 0) {
    errno = EBADF;
    die("fd_read_s() error");
  }

  cap = DEFAULT_BUFFER_CAPACITY;
  if (!(buf = malloc(cap + 1))) // @todo malloc_or_die()
    die("fd_read_s() error");
  buf[0] = '\0';

  for (len = 0; true; ++len) {

    ret = read(fd, &c, 1);

    if (ret == -1) {
      if (errno == EINTR)
        continue;
      free(buf);
      die("fd_read_s() error");
    }

    if (ret == 0)
      break;

    if (len == cap) {

      if (cap > SIZE_MAX / 2) {
        free(buf);
        errno = ENOMEM;
        die("fd_read_s() error");
      }

      cap *= 2;
      if (!(tmp = realloc(buf, cap + 1))) {
        free(buf);
        die("fd_read_s() error");
      }

      buf = tmp;
    }

    buf[len] = c;
    buf[len + 1] = '\0';
  }

  // shrink to fit
  tmp = realloc(buf, len + 1);
  return tmp ? tmp : buf;
}


size_t file_write(FILE *stream, const void *buf, size_t count)
{

  return 0;
}


size_t file_write_s(FILE *stream, const char *s)
{
  int ret;

  if (!s || !stream) {
    errno = EINVAL;
    die("fputs() error");
  }

  if ((ret = fputs(s, stream)) == -1)
    die("fputs() error");

  return ret;
}

size_t file_read(FILE *stream, char *s, size_t count)
{
  if (!s || !stream || count <= 0) {
    errno = EINVAL;
    die("fgets() error");
  }

  if (!(s = fgets(s, count, stream)))
    die("fgets() error");

  return 0;
}

