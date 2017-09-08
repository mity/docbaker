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

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include "win32compat.h"
#else
    #include <limits.h>
    #include <unistd.h>
#endif

#include "config.h"


extern int verbose_level;


/* Localization support. */

#ifdef ENABLE_I18N
    #include <libintl.h>
    #include <locale.h>

    #define _(str)          gettext(str)
#else
    #define _(str)          str
#endif


/* Function attributes. */

#ifdef __GNUC__
    #define FUNCATTR_PRINTF(fmt_index, ellipses_index)                      \
                __attribute__((format(printf, fmt_index, ellipses_index)))
#else
    #define FUNCATTR_PRINTF(fmt_index, ellipses_index)
#endif


/* Log message output. */

void print_diag(FILE* out, const char* prefix, const char* fmt, ...)
        FUNCATTR_PRINTF(3, 4);

#define FATAL(...)                                                          \
    do {                                                                    \
        print_diag(stderr, _("fatal error: "), __VA_ARGS__);                \
        exit(EXIT_FAILURE);                                                 \
    } while(0)

#undef ERROR

#define ERROR(...)                                                          \
    print_diag(stderr, _("error: "), __VA_ARGS__)

#define WARN(...)                                                           \
    print_diag(stderr, _("warning: "), __VA_ARGS__)

#define NOTE(level, ...)                                                    \
    do {                                                                    \
        if((level) <= verbose_level)                                        \
            print_diag(stdout, NULL, __VA_ARGS__);                          \
    } while(0)


/* Never-failing memory allocation. */

void* x_malloc(size_t sz);
void* x_calloc(size_t n, size_t sz);
void* x_realloc(void* mem, size_t sz);

#define malloc          x_malloc
#define calloc          x_calloc
#define realloc         x_realloc


#endif  /* DOCBAKER_MISC_H */
