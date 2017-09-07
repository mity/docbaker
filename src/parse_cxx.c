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

#include "parse_cxx.h"
#include "array.h"
#include "store.h"

#include <clang-c/Index.h>


static void
parse_cxx_function(CXCursor cursor)
{
    CXString name;
    int i, n;

    n = clang_Cursor_getNumArguments(cursor);

    //name = clang_getCursorSpelling(cursor);
    name = clang_getCursorDisplayName(cursor);
    NOTE(0, "Detected function %s.", clang_getCString(name));

    for(i = 0; i < n; i++) {
        CXCursor arg;
        CXString arg_name;
        CXString arg_type;

        arg = clang_Cursor_getArgument(cursor, i);
        arg_name = clang_getCursorDisplayName(arg);
        arg_type = clang_getTypeSpelling(clang_getCursorType(arg));

        NOTE(0, "  arg %s %s.", clang_getCString(arg_type), clang_getCString(arg_name));

        clang_disposeString(arg_name);
        clang_disposeString(arg_type);
    }

    clang_disposeString(name);
}

static void
parse_cxx_macro(CXCursor cur)
{
    CXString name;

    //name = clang_getCursorSpelling(cur);
    name = clang_getCursorDisplayName(cur);
    NOTE(0, "Detected macro %s.", clang_getCString(name));

    clang_disposeString(name);
}


static enum CXChildVisitResult
parse_cxx_callback(CXCursor cur, CXCursor parent_cur, CXClientData data)
{
    CXSourceLocation loc;

    /* Ignore things not directly in the given file (i.e. anything what
     * was #included from elsewhere.). */
    loc = clang_getCursorLocation(cur);
    if(!clang_Location_isFromMainFile(loc))
        return CXChildVisit_Continue;

    switch(cur.kind) {
        case CXCursor_FunctionDecl:     parse_cxx_function(cur); break;
        case CXCursor_MacroDefinition:  parse_cxx_macro(cur); break;
        default:                        break;
    }

    return CXChildVisit_Continue;
}

void
parse_cxx(const char* path, const char** clang_opts, VALUE* store)
{
    ARRAY argv = ARRAY_INIT;
    int i;
    CXIndex index;
    CXTranslationUnit unit;
    enum CXErrorCode err;

    store_file(store, path);

    /* Build options for libclang. */
    array_append(&argv, "-DDOCBAKER");
    // FIXME: This is needed on my machine. This should be either guessed
    //        automagically or provided as an input during package building.
    //        (Some option propagated through CMake???)
    array_append(&argv, "-isystem/usr/lib64/clang/3.8.0/include");
    for(i = 0; clang_opts[i] != NULL; i++)
        array_append(&argv, (void*) clang_opts[i]);
    array_append(&argv, NULL);

    /* Make libclang to do all the hard work. */
    index = clang_createIndex(0, 1);
    if(index == NULL) {
        ERROR(_("Function %s failed."), "clang_createIndex()");
        goto err_createIndex;
    }
    err = clang_parseTranslationUnit2(index, path,
                array_data(&argv), array_size(&argv)-1, NULL, 0,
                CXTranslationUnit_DetailedPreprocessingRecord |
                CXTranslationUnit_Incomplete |
                CXTranslationUnit_SkipFunctionBodies,
                &unit);
    if(err != CXError_Success) {
        ERROR(_("Function %s failed."), "clang_parseTranslationUnit2()");
        goto err_parseTranslationUnit2;
    }

    /* Gather all things to be documented in the translation unit and its
     * documentation. */
    clang_visitChildren(clang_getTranslationUnitCursor(unit),
                        parse_cxx_callback, (CXClientData) store);

err_parseTranslationUnit2:
    clang_disposeIndex(index);
err_createIndex:
    array_fini(&argv, NULL);
}
