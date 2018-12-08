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

#ifndef DOCBAKER_ARRAY_H
#define DOCBAKER_ARRAY_H

#include "buffer.h"


/* Member destructor. */
typedef void (*ARRAY_DTORFUNC)(void* /*item*/);


typedef struct ARRAY {
    BUFFER buffer;
} ARRAY;


#define ARRAY_INITIALIZER      { BUFFER_INITIALIZER }


void array_clear(ARRAY* array, ARRAY_DTORFUNC dtor_func);


static inline void
array_init(ARRAY* array)
{
    buffer_init(&array->buffer);
}

static inline void
array_fini(ARRAY* array, ARRAY_DTORFUNC dtor_func)
{
    array_clear(array, dtor_func);
    buffer_fini(&array->buffer);
}

static inline void*
array_data(ARRAY* array)
{
    return buffer_data(&array->buffer);
}

static inline size_t
array_size(const ARRAY* array)
{
    return buffer_size(&array->buffer) / sizeof(void*);
}

static inline void*
array_get(const ARRAY* array, size_t index)
{
    return * (void**) buffer_data_at((BUFFER*) &array->buffer, index * sizeof(void*));
}

static inline int
array_append(ARRAY* array, void* item)
{
    return buffer_append(&array->buffer, &item, sizeof(void*));
}

static inline int
array_reserve(ARRAY* array, size_t n_items)
{
    return buffer_reserve(&array->buffer, n_items * sizeof(void*));
}

static inline int
array_insert(ARRAY* array, size_t index, void* item)
{
    return buffer_insert(&array->buffer, index * sizeof(void*), &item, sizeof(void*));
}

static inline void
array_remove(ARRAY* array, size_t index, ARRAY_DTORFUNC dtor_func)
{
    if(dtor_func != NULL)
        dtor_func(array_get(array, index));
    buffer_remove(&array->buffer, index * sizeof(void*), sizeof(void*));
}


#endif  /* DOCBAKER_ARRAY_H */
