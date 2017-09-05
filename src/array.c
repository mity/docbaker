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

#include "array.h"


static void
array_clear_(ARRAY* array, ARRAY_DTORFUNC dtor_func)
{
    off_t index;

    if(dtor_func != NULL) {
        for(index = 0; index < array_size(array); index++)
            dtor_func(array_item(array, index));
    }

    free(array->data);
}

void
array_init(ARRAY* array)
{
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

void
array_fini(ARRAY* array, ARRAY_DTORFUNC dtor_func)
{
    array_clear_(array, dtor_func);
}

void
array_reserve(ARRAY* array, size_t n_items)
{
    size_t needed_capacity = array->size + n_items;

    if(needed_capacity > array->capacity) {
        array->data = realloc(array->data, needed_capacity * sizeof(void*));
        array->capacity = needed_capacity;
    }
}

void**
array_insert_raw(ARRAY* array, off_t index)
{
    if(array->size >= array->capacity) {
        if(array->capacity > 0)
            array->capacity *= 2;
        else
            array->capacity = 4;
        array->data = realloc(array->data, array->capacity * sizeof(void*));
    }

    if(index < array->size)
        memmove(array_item(array, index+1), array_item(array, index), (array->size - index) * sizeof(void*));

    array->size++;
    return &array->data[index];
}

void
array_insert(ARRAY* array, off_t index, void* item)
{
    void** ptr;

    ptr = array_insert_raw(array, index);
    *ptr = item;
}

void
array_remove(ARRAY* array, off_t index, ARRAY_DTORFUNC dtor_func)
{
    if(array->size == 1) {    /* Removing last element? */
        array_clear(array, dtor_func);
        return;
    }

    if(dtor_func != NULL)
        dtor_func(array_item(array, index));

    if(index < array->size - 1)
        memmove(array_item(array, index), array_item(array, index+1), (array->size - index - 1) * sizeof(void*));

    array->size--;

    /* Shrink the array if its usage is below 25%. */
    if(4 * array->size < array->capacity) {
        array->capacity /= 2;
        array->data = realloc(array->data, array->capacity * sizeof(void*));
    }
}

void
array_clear(ARRAY* array, ARRAY_DTORFUNC dtor_func)
{
    array_clear_(array, dtor_func);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

