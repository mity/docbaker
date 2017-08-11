/*
 * DocBaker
 * (http://github.com/mity/docbaker)
 *
 * Copyright (c) 2017 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef DOCBAKER_MISC_H
#define DOCBAKER_MISC_H

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define STRINGIZE_(x)       #x
#define STRINGIZE(x)        STRINGIZE_(x)


extern int verbose_level;


/* Log message output. */

void print_diag(FILE* out, const char* prefix, const char* fmt, ...);

#define FATAL(...)      do { print_diag(stderr, "fatal error: ", __VA_ARGS__); exit(EXIT_FAILURE); } while(0)
#define ERROR(...)      print_diag(stderr, "error: ", __VA_ARGS__)
#define WARN(...)       print_diag(stderr, "warning: ", __VA_ARGS__)


/* Never-failing memory allocation. */

void* x_malloc(size_t sz);
void* x_calloc(size_t n, size_t sz);
void* x_realloc(void* mem, size_t sz);

#define malloc          x_malloc
#define calloc          x_calloc
#define realloc         x_realloc


#endif  /* DOCBAKER_MISC_H */
