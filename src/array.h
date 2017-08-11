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

#include "misc.h"


typedef struct ARRAY {
    void** data;
    size_t size;
    size_t capacity;
} ARRAY;


#define ARRAY_INITIALIZER       { 0 }


/* Destructor. */
typedef void (*ARRAY_DTOR)(void* /*item*/);


void array_init(ARRAY* array);
void array_fini(ARRAY* array, ARRAY_DTOR dtor_func);

void array_reserve(ARRAY* array, size_t n_items);

void** array_insert_raw(ARRAY* array, off_t index);
void array_insert(ARRAY* array, off_t index, void* item);

void array_remove(ARRAY* array, off_t index, ARRAY_DTOR dtor_func);
void array_clear(ARRAY* array, ARRAY_DTOR dtor_func);


static inline void*
array_data(const ARRAY* array)
{
    return array->data;
}

static inline size_t
array_size(const ARRAY* array)
{
    return array->size;
}

static inline off_t
array_index(const ARRAY* array, void* item)
{
    return (off_t)((size_t)(((uint8_t*)item - (uint8_t*)array->data)));
}

static inline void*
array_item(const ARRAY* array, off_t index)
{
    return array->data[index];
}

static inline void**
array_append_raw(ARRAY* array)
{
    return array_insert_raw(array, array->size);
}

static inline void
array_append(ARRAY* array, void* item)
{
    array_insert(array, array->size, item);
}


#endif  /* DOCBAKER_ARRAY_H */
