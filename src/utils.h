
// file: utils.h
// author: ishimai

#pragma once

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

#ifdef _MSC_VER
  #define strdup _strdup
#endif

[[noreturn]] void die(const char *msg);
char *smprintf(char *fmt, ...);
char *fgetline(FILE *stream);
char *ltrim(const char *s);
char *rtrim(const char *s);
char *trim(const char *s);
bool has_spaces(const char *s);
int imax(int a, int b);
