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

#ifndef DOCBAKER_BUFFER_H
#define DOCBAKER_BUFFER_H

#include "misc.h"


typedef struct BUFFER {
    uint8_t* data;
    size_t size;
    size_t capacity;
} BUFFER;


#define BUFFER_INITIALIZER      { 0 }


void buffer_init(BUFFER* buf);
void buffer_fini(BUFFER* buf);

void buffer_reserve(BUFFER* buf, size_t n);

void buffer_insert(BUFFER* buf, off_t index, const uint8_t* data, size_t n);

void buffer_remove(BUFFER* buf, off_t index, size_t n);
void buffer_clear(BUFFER* buf);


static inline uint8_t*
buffer_data(BUFFER* buf)
{
    return buf->data;
}

static inline size_t
buffer_size(BUFFER* buf)
{
    return buf->size;
}

static inline void
buffer_append(BUFFER* buf, const uint8_t* data, size_t n)
{
    buffer_insert(buf, buf->size, data, n);
}


#endif  /* DOCBAKER_BUFFER_H */
