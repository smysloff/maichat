
// file: lib/io.h

#pragma once

#include "process.h"   // die()

#include <errno.h>     // errno
#include <stdbool.h>   // true, false
#include <stddef.h>    // NULL, size_t
#include <stdint.h>    // SIZE_MAX
#include <stdio.h>     // FILE
#include <stdlib.h>    // malloc(), realloc(), free()
#include <sys/types.h> // ssize_t
#include <unistd.h>    // exit(), read(), write()

// fd
size_t fd_write(int fd, const void *buf, size_t count);
size_t fd_write_s(int fd, const char *s);
size_t fd_read(int fd, void *buf, size_t count);
char  *fd_read_l(int fd); // alloc
char  *fd_read_s(int fd); // alloc

// FILE (stream)
size_t file_write(FILE *stream, const void *buf, size_t count);
size_t file_write_s(FILE *stream, const char *s);
size_t file_read(FILE *stream, void *s, size_t count);
char  *file_read_l(FILE *stream); // alloc
char  *file_read_s(FILE *stream); // alloc
