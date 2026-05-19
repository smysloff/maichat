
// file: lib/process.h

#pragma once

#include <errno.h>     // errno
#include <stdarg.h>    // ..., va_list, va_start(), va_end()
#include <stddef.h>    // size_t
#include <stdio.h>     // fprintf(), vfprintf(),
#include <stdlib.h>    // EXIT_FAILURE, malloc()
#include <string.h>    // strerror()
#include <unistd.h>    // exit()

[[noreturn]] void die(const char *fmt, ...);
void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
