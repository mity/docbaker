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

#include "buffer.h"


void
buffer_init(BUFFER* buf)
{
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
}

void
buffer_fini(BUFFER* buf)
{
    free(buf->data);
}

void
buffer_reserve(BUFFER* buf, size_t n)
{
    size_t needed_capacity = buf->size + n;

    if(needed_capacity > buf->capacity) {
        buf->data = (uint8_t*) xrealloc(buf->data, needed_capacity);
        buf->capacity = needed_capacity;
    }
}

void
buffer_insert(BUFFER* buf, off_t index, const uint8_t* data, size_t n)
{
    if(buf->size + n > buf->capacity) {
        while(buf->size + n > buf->capacity) {
            if(buf->capacity > 0)
                buf->capacity = (buf->size + n) * 2;
            else
                buf->capacity = 4;
        }
        buf->data = (uint8_t*) xrealloc(buf->data, buf->capacity);
    }

    if(index < buf->size)
        memmove(buf->data + index + n, buf->data + index, buf->size - index);

    buf->size += n;
}

void
buffer_remove(BUFFER* buf, off_t index, size_t n)
{
    assert(index + n <= buf->size);

    if(buf->size == n) {
        buffer_clear(buf);
        return;
    }

    if(index < buf->size - n)
        memmove(buf->data + index, buf->data + index + n, buf->size - index - n);

    buf->size -= n;

    /* Shrink the buffer if its usage is below 25%. */
    if(4 * buf->size < buf->capacity) {
        buf->capacity /= 2;
        buf->data = (uint8_t*) xrealloc(buf->data, buf->capacity);
    }
}

void
buffer_clear(BUFFER* buf)
{
    buffer_fini(buf);
    buffer_init(buf);
}

