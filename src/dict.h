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

#ifndef DOCBAKER_DICT_H
#define DOCBAKER_DICT_H

#include "misc.h"
#include "array.h"


/* Value destructor. */
typedef void (*DICT_DTORFUNC)(void* /*item*/);


struct DICT_BUCKET;

typedef struct DICT {
    struct DICT_BUCKET** buckets;
    size_t n_buckets;
    size_t n_members;
} DICT;


void dict_init(DICT* dict);
void dict_fini(DICT* dict, DICT_DTORFUNC dtor_func);

void dict_set(DICT* dict, const char* key, void* value, DICT_DTORFUNC dtor_func);
void* dict_get(DICT* dict, const char* key);

void dict_collect_keys(DICT* dict, ARRAY* array);


#endif  /* DOCBAKER_DICT_H */
