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
#include "path_util.h"
#include "store.h"

#include <clang-c/Index.h>


typedef struct PARSE_CXX_CONTEXT {
    ARRAY comments;
    VALUE* store;
    VALUE* val_file;
} PARSE_CXX_CONTEXT;


#if 0
static void
parse_cxx_comment2doc(PARSE_CXX_CONTEXT* ctx, VALUE* val, const char* raw_comment_text)
{
    store_register_doc(val, raw_comment_text);
}
#endif

static void
parse_cxx_function(PARSE_CXX_CONTEXT* ctx, CXCursor cursor)
{
    CXString name;
    CXString spelling;
    CXString comment;

    name = clang_getCursorDisplayName(cursor);
    spelling = clang_getCursorSpelling(cursor);
    comment = clang_Cursor_getRawCommentText(cursor);

    NOTE(1, "Detected function %s.", clang_getCString(spelling));
    store_register_function(ctx->store, ctx->val_file,
                    clang_getCString(spelling), clang_getCString(name));

    //parse_cxx_comment2doc(ctx, val_func, clang_getCString(comment));

#if 0
    int i, n;

    n = clang_Cursor_getNumArguments(cursor);
    for(i = 0; i < n; i++) {
        CXCursor arg;
        CXString arg_name;
        CXString arg_type;

        arg = clang_Cursor_getArgument(cursor, i);
        arg_name = clang_getCursorDisplayName(arg);
        arg_type = clang_getTypeSpelling(clang_getCursorType(arg));

        NOTE(2, "Detected function param %s %s.", clang_getCString(arg_type), clang_getCString(arg_name));

        clang_disposeString(arg_name);
        clang_disposeString(arg_type);
    }
#endif

    clang_disposeString(name);
    clang_disposeString(spelling);
    clang_disposeString(comment);
}

static void
parse_cxx_macro(PARSE_CXX_CONTEXT* ctx, CXCursor cur)
{
    CXString name;

    //name = clang_getCursorSpelling(cur);
    name = clang_getCursorDisplayName(cur);
    NOTE(1, "Detected macro %s.", clang_getCString(name));

    clang_disposeString(name);
}


static enum CXChildVisitResult
parse_cxx_callback(CXCursor cur, CXCursor parent_cur, CXClientData data)
{
    PARSE_CXX_CONTEXT* ctx = (PARSE_CXX_CONTEXT*) data;
    CXSourceLocation loc;

    /* Ignore things not directly in the given file (i.e. anything what
     * was #included from elsewhere.). */
    loc = clang_getCursorLocation(cur);
    if(!clang_Location_isFromMainFile(loc))
        return CXChildVisit_Continue;

    switch(cur.kind) {
        case CXCursor_FunctionDecl:     parse_cxx_function(ctx, cur); break;
        case CXCursor_MacroDefinition:  parse_cxx_macro(ctx, cur); break;
        default:                        break;
    }

    return CXChildVisit_Continue;
}

void
parse_cxx(const char* path, const char** clang_opts, VALUE* store)
{
    char opt_sysincdir[PATH_MAX] = { 0 };
    ARRAY argv = ARRAY_INITIALIZER;
    int i;
    CXIndex index;
    CXTranslationUnit unit;
    CXCursor unit_cursor;
    enum CXErrorCode err;
    PARSE_CXX_CONTEXT ctx;

    ctx.store = store;
    ctx.val_file = store_register_file(store, path);

    /* Build options for libclang. */
    CHECK(array_append(&argv, "-DDOCBAKER") == 0);

#ifdef _WIN32
    /* On Windows, we distribute the headers for libclang in the package
     * and use path relative to the main executable for the case when user
     * moves whole app directory elsewhere. */
    snprintf(opt_sysincdir, PATH_MAX-1, "-isystem%s%s", path_to_executable(), CLANG_SYSINCDIR);
#else
    snprintf(opt_sysincdir, PATH_MAX-1, "-isystem%s", CLANG_SYSINCDIR);
#endif
    CHECK(array_append(&argv, opt_sysincdir) == 0);
    for(i = 0; clang_opts[i] != NULL; i++)
        CHECK(array_append(&argv, (void*) clang_opts[i]) == 0);
    CHECK(array_append(&argv, NULL) == 0);

    /* Parse the translation unit. */
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
    unit_cursor = clang_getTranslationUnitCursor(unit);

    /* Gather all things to be documented in the translation unit and its
     * documentation. */
    clang_visitChildren(unit_cursor, parse_cxx_callback, (CXClientData) &ctx);

    clang_disposeTranslationUnit(unit);
err_parseTranslationUnit2:
    clang_disposeIndex(index);
err_createIndex:
    array_fini(&argv, NULL);
}
