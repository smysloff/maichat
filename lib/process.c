
// file: lib/process.c

#include "process.h"

[[noreturn]] void die(const char *fmt, ...)
{
  va_list ap;
  int saved_errno = errno;

  if (fmt) {
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ": ");
    va_end(ap);
  }

  if (saved_errno)
    fprintf(stderr, "%s", strerror(saved_errno));
  fprintf(stderr, "\n");

  exit(EXIT_FAILURE);
}

void *xmalloc(size_t size)
{
  void *ptr;
  if (!(ptr = malloc(size)))
    die("xmalloc() error");
  return ptr;
}

void *xrealloc(void *p, size_t size)
{
  void *tmp;
  if (!(tmp = realloc(p, size)))
    die("realloc() error");
  return tmp;
}
