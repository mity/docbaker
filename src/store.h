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

#ifndef DOCBAKER_STORE_H
#define DOCBAKER_STORE_H

#include "misc.h"
#include "value.h"


/* These functions create new (empty) VALUE corresponding to the given
 * entity, add it into the store, and return the new VALUE, without
 * increasing a ref. count. If the caller wants to keep the value fir his
 * own purposes for possibly longer time then the store's existence, he should
 * gc_ref() it. */

VALUE* store_file(VALUE* store, const char* fname);


VALUE* store_create(void);
void store_destroy(VALUE* store);


#endif  /* DOCBAKER_STORE_H */
