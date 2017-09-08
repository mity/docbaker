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

#include "gen_json.h"

#include <inttypes.h>


static void
gen_json_dump_indent(FILE* f, int indent)
{
    static const char indent_str[32] = "                                ";

    while(indent > sizeof(indent_str)) {
        fprintf(f, "%.*s", (int) sizeof(indent_str), indent_str);
        indent -= (int) sizeof(indent_str);
    }

    fprintf(f, "%.*s", indent, indent_str);
}

static int
gen_json_key_cmp(const void* a, const void* b)
{
    return strcmp(*(const char**) a, *(const char**) b);
}

static void
gen_json_dump_value(FILE* f, const char* key, const VALUE* value, int indent)
{
    gen_json_dump_indent(f, indent);

    if(key != NULL)
        fprintf(f, "%s: ", key);

    switch(value_type(value)) {
        case VALUE_NULL:
            fprintf(f, "null");
            break;

        case VALUE_INT:
            fprintf(f, "%"PRIi64, value_int(value));
            break;

        case VALUE_STR:
            // FIXME: escaping
            fprintf(f, "\"%.*s\"", (int) value_strlen(value), value_str(value));
            break;

        case VALUE_ARRAY:
        {
            size_t i, n;

            fprintf(f, "[\n");
            n = value_array_size(value);
            for(i = 0; i < n; i++) {
                gen_json_dump_value(f, NULL, value_array_item(value, i), indent + 4);
                fprintf(f, (i < n-1) ? ",\n" : "\n");
            }
            gen_json_dump_indent(f, indent);
            fprintf(f, "]");
            break;
        }

        case VALUE_DICT:
        {
            ARRAY keys = ARRAY_INIT;
            size_t i, n;

            value_dict_collect_keys(value, &keys);
            qsort(array_data(&keys), array_size(&keys), sizeof(void*), gen_json_key_cmp);

            fprintf(f, "{\n");

            n = array_size(&keys);
            for(i = 0; i < n; i++) {
                const char* key = (const char*) array_item(&keys, i);
                gen_json_dump_value(f, key, value_dict_item(value, key), indent + 4);
                fprintf(f, (i < n-1) ? ",\n" : "\n");
            }

            gen_json_dump_indent(f, indent);
            fprintf(f, "}");
            array_fini(&keys, NULL);
        }
    }
}

void
gen_json(const char* json_output_file, const VALUE* store)
{
    FILE* f;

    f = fopen(json_output_file, "wt");
    if(f == NULL)
        FATAL("%s (%s)", strerror(errno), json_output_file);

    gen_json_dump_value(f, NULL, store, 0);

    fprintf(f, "\n");
    fclose(f);
}
