/*
 * CentiJSON
 * <http://github.com/mity/centijson>
 *
 * Copyright (c) 2018 Martin Mitas
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

#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* JSON data types.
 *
 * Note that we distinguish beginning/end of the arrays and objects for
 * the purposes of the processing.
 */
typedef enum JSON_TYPE {
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_KEY,     /* String in the specific role of an object key. */
    JSON_ARRAY_BEG,
    JSON_ARRAY_END,
    JSON_OBJECT_BEG,
    JSON_OBJECT_END
} JSON_TYPE;


/* Error codes.
 */
#define JSON_ERR_SUCCESS                0
#define JSON_ERR_INTERNAL               (-1)    /* This should never happen. If you see it, report bug ;-) */
#define JSON_ERR_OUTOFMEMORY            (-2)
#define JSON_ERR_SYNTAX                 (-4)    /* Generic syntax error. (More specific error codes are preferred.) */
#define JSON_ERR_BADCLOSER              (-5)    /* Mismatch in brackets (e.g. "{ ]" or "[ }") */
#define JSON_ERR_BADROOTTYPE            (-6)    /* Root type not allowed by CONFIG::flags. */
#define JSON_ERR_EXPECTEDVALUE          (-7)    /* Something unexpected where value has to be. */
#define JSON_ERR_EXPECTEDKEY            (-8)    /* Something unexpected where key has to be. */
#define JSON_ERR_EXPECTEDVALUEORCLOSER  (-9)    /* Something unexpected where value or array/object closer has to be. */
#define JSON_ERR_EXPECTEDKEYORCLOSER    (-10)   /* Something unexpected where key or array/object closer has to be. */
#define JSON_ERR_EXPECTEDCOLON          (-11)   /* Something unexpected where colon has to be. */
#define JSON_ERR_EXPECTEDCOMMAORCLOSER  (-12)   /* Something unexpected where comma or array/object has to be. */
#define JSON_ERR_EXPECTEDEOF            (-13)   /* Something unexpected where end-of-file has to be. */
#define JSON_ERR_MAXTOTALLEN            (-14)   /* Reached JSON_CONFIG::max_total_len */
#define JSON_ERR_MAXTOTALVALUES         (-15)   /* Reached JSON_CONFIG::max_total_values */
#define JSON_ERR_MAXNESTINGLEVEL        (-16)   /* Reached JSON_CONFIG::max_nesting_level */
#define JSON_ERR_MAXNUMBERLEN           (-17)   /* Reached JSON_CONFIG::max_number_len */
#define JSON_ERR_MAXSTRINGLEN           (-18)   /* Reached JSON_CONFIG::max_string_len */
#define JSON_ERR_MAXKEYLEN              (-19)   /* Reached JSON_CONFIG::max_key_len */
#define JSON_ERR_UNCLOSEDSTRING         (-20)   /* Unclosed string */
#define JSON_ERR_UNESCAPEDCONTROL       (-21)   /* Unescaped control character (in a string) */
#define JSON_ERR_INVALIDESCAPE          (-22)   /* Invalid/unknown escape sequence (in a string) */
#define JSON_ERR_INVALIDUTF8            (-23)   /* Invalid UTF-8 (in a string) */


/* Bits for JSON_CONFIG::flags.
 */
#define JSON_NONULLASROOT           0x0001  /* Allow to be root value */
#define JSON_NOBOOLASROOT           0x0002  /* Allow false or true to be root value */
#define JSON_NONUMBERASROOT         0x0004  /* Allow number to be root value */
#define JSON_NOSTRINGASROOT         0x0008  /* Allow string to be root value */
#define JSON_NOARRAYASROOT          0x0010  /* Allow array to be root value */
#define JSON_NOOBJECTASROOT         0x0020  /* Allow object  to be root value */

#define JSON_NOSCALARROOT           (JSON_NONULLASROOT | JSON_NOBOOLASROOT |  \
                                     JSON_NONUMBERASROOT | JSON_NOSTRINGASROOT)
#define JSON_NOVECTORROOT           (JSON_NOARRAYASROOT | JSON_NOOBJECTASROOT)

#define JSON_IGNOREILLUTF8KEY       0x0100  /* Ignore ill-formed UTF-8 (for keys). */
#define JSON_FIXILLUTF8KEY          0x0200  /* Replace ill-formed UTF-8 with replacement char (for keys). */
#define JSON_IGNOREILLUTF8VALUE     0x0400  /* Ignore ill-formed UTF-8 (for string values). */
#define JSON_FIXILLUTF8VALUE        0x0800  /* Replace ill-formed UTF-8 with replacement char (for string values). */



/* Parser options, passed into json_init().
 *
 * If NULL is passed to json_init(), default values are used.
 */
typedef struct JSON_CONFIG {
    size_t max_total_len;       /* zero means no limit; default: 10 MB */
    size_t max_total_values;    /* zero means no limit; default: 0 */
    size_t max_number_len;      /* zero means no limit; default: 64 */
    size_t max_string_len;      /* zero means no limit; default: 65536 */
    size_t max_key_len;         /* zero means no limit; default: 256 */
    unsigned max_nesting_level; /* zero means no limit; default: 256 */
    unsigned flags;             /* default: 0 */
} JSON_CONFIG;


/* Helper structure describing position in the input.
 *
 * It is used to specify where in the input a parsing error occurred for
 * better diagnostics.
 */
typedef struct JSON_INPUT_POS {
    size_t offset;
    unsigned line_number;
    unsigned column_number;
} JSON_INPUT_POS;


/* Callbacks the application has to implement, to process the parsed data.
 */
typedef struct JSON_CALLBACKS {
    /* Data processing callback. For now, and maybe forever, the only callback.
     *
     * Note that `data` and `data_size` are set only for JSON_KEY, JSON_STRING
     * and JSON_NUMBER. (For the other types the callback always gets NULL and
     * 0).
     *
     * Inside an object, the application is guaranteed to get keys and their
     * corresponding values in the alternating fashion (i.e. in the order
     * as they are in the JSON input.).
     *
     * Application can abort complete parsing operation by returning non-zero.
     * Note the non-zero return value of the callback is propagated to
     * json_feed() and json_fini().
     */
    int (*process)(JSON_TYPE /*type*/, const char* /*data*/,
                   size_t /*data_size*/, void* /*user_data*/);
} JSON_CALLBACKS;


/* Internal parser state. Use pointer to this structure as an opaque handle.
 */
typedef struct JSON_PARSER {
    JSON_CALLBACKS callbacks;
    JSON_CONFIG config;
    void* user_data;

    JSON_INPUT_POS pos;
    JSON_INPUT_POS value_pos;
    JSON_INPUT_POS err_pos;

    int errcode;

    size_t value_counter;

    char* nesting_stack;
    size_t nesting_level;
    size_t nesting_stack_size;

    unsigned automaton;
    unsigned state;
    unsigned substate;

    uint32_t codepoint[2];

    char* buf;
    size_t buf_used;
    size_t buf_alloced;

    size_t last_cl_offset;  /* Offset of most recently seen '\r' */
} JSON_PARSER;



/* Fill `cfg` with options used by default.
 */
void json_default_config(JSON_CONFIG* cfg);


/* Initialize the parser, associate it with the given callbacks and
 * configuration. Returns zero on success, non-zero on an error.
 */
int json_init(JSON_PARSER* parser,
              const JSON_CALLBACKS* callbacks,
              const JSON_CONFIG* config,
              void* user_data);

/* Feed the parser with more input.
 *
 * Returns zero on success.
 *
 * If an error occurs it returns non-zero, and the application should just call
 * json_fini(); any trying to call json_feed() again shall just fail with
 * the same return value.
 */
int json_feed(JSON_PARSER* parser, const char* input, size_t size);

/* Finish parsing of the document (note it can still call some callbacks); and
 * release any resource held by the parser.
 *
 * Returns zero on success, or non-zero on failure.
 *
 * If p_pos is not NULL, it is filled with info about reached position in the
 * input. It can help in diagnostics if the parsing failed.
 *
 * Note that if the preceding call to json_feed() failed, the error status also
 * propagates into json_fini().
 *
 * Also note this function may still fail even if all preceding calls to
 * json_feed() succeeded. This typically happens if the processed JSON input
 * was not complete valid JSON document.
 */
int json_fini(JSON_PARSER* parser, JSON_INPUT_POS* p_pos);


/* Simple wrapper function for json_init() + json_feed() + json_fini(), usable
 * when the provided input contains complete JSON document.
 */
int json_parse(const char* input, size_t size,
               const JSON_CALLBACKS* callbacks, const JSON_CONFIG* config,
               void* user_data, JSON_INPUT_POS* p_pos);


/*****************
 *** Utilities ***
 *****************/

/* When implementing the callback processing the parsed data, these utilities
 * below may come handy.
 */

/* Analyze the string holding a JSON number, and analyze whether it can
 * fit into integer types.
 *
 * (Note it says no, if the number contains any fraction or exponent part.)
 */
void json_analyze_number(const char* num, size_t num_size,
                         int* p_is_int32_compatible,
                         int* p_is_uint32_compatible,
                         int* p_is_int64_compatible,
                         int* p_is_uint64_compatible);

/* Convert the string holding JSON number to the given C type.
 *
 * Note the conversion to any of the integer types is undefined unless
 * json_analyze_number() says it is fine.
 *
 * And also note that json_number_to_double() can fail with JSON_ERR_OUTOFMEMORY.
 * Hence it prototype differs.
 */
int32_t json_number_to_int32(const char* num, size_t num_size);
uint32_t json_number_to_uint32(const char* num, size_t num_size);
int64_t json_number_to_int64(const char* num, size_t num_size);
uint64_t json_number_to_uint64(const char* num, size_t num_size);
int json_number_to_double(const char* num, size_t num_size, double* p_result);


/* Helpers for writing numbers and strings.
 * (Given that all the rest is trivial, do it manually ;-)
 *
 * Note that json_dump_string() assumes the string is well-formed UTF-8 string.
 *
 * The provided writing function must write all the data provided to it
 * and return zero to indicate success, or non-zero to indicate an error
 * and abort the operation.
 *
 * Returns zero on success, JSON_ERR_OUTOFMEMORY, or an error code returned
 * from writing callback.
 */
int json_dump_int32(int32_t i32, int (*write_func)(const char*, size_t, void*), void* user_data);
int json_dump_uint32(uint32_t u32, int (*write_func)(const char*, size_t, void*), void* user_data);
int json_dump_int64(int64_t i64, int (*write_func)(const char*, size_t, void*), void* user_data);
int json_dump_uint64(uint64_t u64, int (*write_func)(const char*, size_t, void*), void* user_data);
int json_dump_double(double d, int (*write_func)(const char*, size_t, void*), void* user_data);
int json_dump_string(const char* str, size_t size, int (*write_func)(const char*, size_t, void*), void* user_data);


#ifdef __cplusplus
}  /* extern "C" { */
#endif

#endif  /* JSON_H */
