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

#include "store.h"


/* Note: Most nodes have both "name" and "id".
 *
 * "id" is meant to be unique.
 * "name" is meant to be presented to the user.
 */



static VALUE*
store_ensure(VALUE* parent_dict, VALUE_TYPE type, const char* key)
{
    VALUE* v;

    v = value_dict_item(parent_dict, key);
    if(v != NULL) {
        assert(value_type(v) == type);
    } else {
        switch(type) {
            case VALUE_ARRAY:   v = value_create_array(); break;
            case VALUE_DICT:    v = value_create_dict(); break;
            default:            assert(0); break;
        }

        value_dict_set(parent_dict, key, v);
    }

    return v;
}

VALUE*
store_register_file(VALUE* store, const char* fname)
{
    VALUE* val_file;
    VALUE* val_name;
    VALUE* val_id;

    val_name = value_create_str(fname);
    val_id = value_ref(val_name);

    val_file = value_create_dict();
    value_dict_set(val_file, "name", val_name);
    value_dict_set(val_file, "functions", value_create_array());
    value_dict_set(val_file, "id", val_id);

    value_array_append(store_ensure(store, VALUE_ARRAY, "files"), val_file);

    return val_file;
}

VALUE*
store_register_function(VALUE* store, VALUE* file, const char* name, const char* long_name)
{
    VALUE* val_func;
    VALUE* val_name;
    VALUE* val_long_name;
    VALUE* val_id;

    val_name = value_create_str(name);
    val_long_name = value_create_str(long_name);
    val_id = value_ref(val_long_name);

    val_func = value_create_dict();
    value_dict_set(val_func, "name", val_name);
    value_dict_set(val_func, "long_name", val_long_name);
    value_dict_set(val_func, "id", val_id);

    value_array_append(value_dict_item(file, "functions"), val_func);

    return val_func;
}

void
store_register_doc(VALUE* item, const char* raw_doc)
{
    value_dict_set(item, "doc", value_create_str(raw_doc));
}

VALUE*
store_create(void)
{
    VALUE* store;

    store = value_create_dict();
    value_dict_set(store, "generator_name", value_create_str(PACKAGE_DISPLAYNAME));
    value_dict_set(store, "generator_version", value_create_str(PACKAGE_VERSION));

    return store;
}

void
store_destroy(VALUE* store)
{
    value_unref(store);
}

