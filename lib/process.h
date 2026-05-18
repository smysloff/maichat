
// file: process.h

#pragma once

#include <errno.h>     // errno
#include <stdarg.h>    // ..., va_list, va_start(), va_end()
#include <stdio.h>     // fprintf(), vfprintf(),
#include <stdlib.h>    // EXIT_FAILURE
#include <string.h>    // strerror()
#include <unistd.h>    // exit()

[[noreturn]] void die(const char *fmt, ...);
