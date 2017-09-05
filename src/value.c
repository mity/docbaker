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

#include "value.h"


#define VALUE_STRINPLACE_MAXSIZE    (sizeof(void*) * 3 - 1)


typedef struct VALUE_STRINPLACE {
    char size;
    char buf[VALUE_STRINPLACE_MAXSIZE];
} VALUE_STRINPLACE;

typedef struct VALUE_STRPTR {
    size_t size;
    char* buf;
} VALUE_STRPTR;


struct VALUE {
    VALUE_TYPE type;
    unsigned n_refs         : sizeof(unsigned) - 1;
    unsigned is_inplace_str : 1;

    union {
        int64_t integer;
        VALUE_STRINPLACE str_inplace;
        VALUE_STRPTR str_ptr;
        ARRAY array;
        DICT dict;
    } data;
};


VALUE*
value_create_null(void)
{
    static VALUE value = { VALUE_NULL, 1, 0 };
    return &value;
}

VALUE*
value_create_int(int64_t integer)
{
    VALUE* value;

    value = (VALUE*) malloc(sizeof(VALUE));
    value->type = VALUE_INT;
    value->n_refs = 1;
    value->data.integer = integer;

    return value;
}

VALUE*
value_create_str_sz(const char* str, size_t size)
{
    VALUE* value;

    value = (VALUE*) malloc(sizeof(VALUE));
    value->type = VALUE_STR;
    value->n_refs = 1;

    if(size <= VALUE_STRINPLACE_MAXSIZE) {
        value->is_inplace_str = 1;
        value->data.str_inplace.size = (char) size;
        memcpy(value->data.str_inplace.buf, str, size);
    } else {
        value->is_inplace_str = 0;
        value->data.str_ptr.size = size;
        value->data.str_ptr.buf = (char*) malloc(size);
        memcpy(value->data.str_ptr.buf, str, size);
    }

    return value;
}

VALUE*
value_create_str(const char* str)
{
    return value_create_str_sz(str, strlen(str));
}

VALUE*
value_create_array(void)
{
    VALUE* value;

    value = (VALUE*) malloc(sizeof(VALUE));
    value->type = VALUE_ARRAY;
    value->n_refs = 1;
    array_init(&value->data.array);

    return value;
}

VALUE*
value_create_dict(void)
{
    VALUE* value;

    value = (VALUE*) malloc(sizeof(VALUE));
    value->type = VALUE_DICT;
    value->n_refs = 1;
    dict_init(&value->data.dict);

    return value;
}

VALUE*
value_ref(VALUE* value)
{
    value->n_refs++;
    return value;
}

void
value_unref(VALUE* value)
{
    value->n_refs--;

    if(value->n_refs == 0) {
        switch(value->type) {
            case VALUE_NULL:
                /* noop, not even free(value) */
                return;

            case VALUE_INT:
                /* noop */
                break;

            case VALUE_STR:
                if(!value->is_inplace_str)
                    free(value->data.str_ptr.buf);
                break;

            case VALUE_ARRAY:
                array_fini(&value->data.array, (ARRAY_DTORFUNC) value_unref);
                break;

            case VALUE_DICT:
                dict_fini(&value->data.dict, (DICT_DTORFUNC) value_unref);
                break;
        }
    }

    free(value);
}

VALUE_TYPE
value_type(const VALUE* value)
{
    return value->type;
}

int64_t
value_int(const VALUE* value)
{
    if(value->type != VALUE_INT)
        return -1;

    return value->data.integer;
}

const char*
value_str(const VALUE* value)
{
    if(value->type != VALUE_STR)
        return NULL;

    if(value->is_inplace_str)
        return value->data.str_inplace.buf;
    else
        return value->data.str_ptr.buf;
}

size_t
value_strlen(const VALUE* value)
{
    if(value->type != VALUE_STR)
        return 0;

    if(value->is_inplace_str)
        return value->data.str_inplace.size;
    else
        return value->data.str_ptr.size;
}

off_t
value_array_append(VALUE* array, VALUE* item)
{
    if(array->type != VALUE_ARRAY)
        return -1;

    return array_append(&array->data.array, item);
}

size_t
value_array_size(const VALUE* array)
{
    if(array->type != VALUE_ARRAY)
        return 0;

    return array_size(&array->data.array);
}

VALUE*
value_array_item(const VALUE* array, off_t index)
{
    if(array->type != VALUE_ARRAY)
        return NULL;

    if(index < 0  ||  array_size(&array->data.array) <= index)
        return NULL;

    return array_item(&array->data.array, index);
}

int
value_dict_set(VALUE* dict, const char* key, VALUE* item)
{
    if(dict->type != VALUE_DICT)
        return -1;

    dict_set(&dict->data.dict, key, item, (DICT_DTORFUNC) value_unref);
    return 0;
}

VALUE*
value_dict_item(const VALUE* dict, const char* key)
{
    if(dict->type != VALUE_DICT)
        return NULL;

    return dict_get((DICT*) &dict->data.dict, key);
}

void
value_dict_collect_keys(const VALUE* dict, ARRAY* array)
{
    if(dict->type != VALUE_DICT)
        return;

    dict_collect_keys((DICT*) &dict->data.dict, array);
}
