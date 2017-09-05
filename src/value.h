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

#ifndef DOCBAKER_VALUE_H
#define DOCBAKER_VALUE_H

#include "misc.h"
#include "array.h"
#include "dict.h"


typedef enum VALUE_TYPE {
    VALUE_NULL,
    VALUE_INT,
    VALUE_STR,
    VALUE_ARRAY,
    VALUE_DICT
} VALUE_TYPE;


typedef struct VALUE VALUE;


/* All the factory functions return VALUE with ref. count set to 1. */
VALUE* value_create_null(void);
VALUE* value_create_int(int64_t integer);
VALUE* value_create_str_sz(const char* str, size_t size);
VALUE* value_create_str(const char* str);
VALUE* value_create_array(void);
VALUE* value_create_dict(void);


/* Inc./dec. value's ref. counter.
 * When it drops to zero, it is destroyed and all nested values (in case of
 * array or dict) are unref'ed. */
VALUE* value_ref(VALUE* value);
void value_unref(VALUE* value);


VALUE_TYPE value_type(const VALUE* value);

/* VALUE_INT specific. */
int64_t value_int(const VALUE* value);

/* VALUE_STR specific. */
const char* value_str(const VALUE* value);
size_t value_strlen(const VALUE* value);

/* VALUE_ARRAY specific.
 * Note: value_array_append() consumes caller's reference to item VALUE. */
off_t value_array_append(VALUE* array, VALUE* item);
size_t value_array_size(const VALUE* array);
VALUE* value_array_item(const VALUE* array, off_t index);

/* VALUE_DICT specific.
 * Note: value_dict_set() consumes caller's reference to item VALUE. */
int value_dict_set(VALUE* dict, const char* key, VALUE* item);
VALUE* value_dict_item(const VALUE* dict, const char* key);
void value_dict_collect_keys(const VALUE* dict, ARRAY* array);


#endif  /* DOCBAKER_VALUE_H */
