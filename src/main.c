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

#include "misc.h"
#include "array.h"
#include "cmdline.h"
#include "gen_html.h"
#include "gen_json.h"
#include "parse_cxx.h"
#include "path.h"
#include "store.h"


int verbose_level = 0;

static int dry_run = 0;
static const char* argv0;
static ARRAY argv_paths = ARRAY_INIT;

/* For C/C++ parser. */
static ARRAY clang_opts = ARRAY_INIT;


#define HTML_GENERATOR                  0x01
#define JSON_GENERATOR                  0x02
static unsigned enabled_generators = 0;

/* For HTML generator. */
#define DEFAULT_HTML_OUTPUT_DIR     "doc-html"
#define DEFAULT_HTML_SKIN           "default"
static const char* html_output_dir = DEFAULT_HTML_OUTPUT_DIR;
static const char* html_skin = DEFAULT_HTML_SKIN;

/* For JSON generator. */
#define DEFAULT_JSON_OUTPUT_FILE    "doc.json"
static const char* json_output_file = DEFAULT_JSON_OUTPUT_FILE;

static int n_processed_files = 0;


static void
print_version(void)
{
    printf("%s %s\n", argv0, PACKAGE_VERSION);
    exit(EXIT_SUCCESS);
}

static void
print_usage(void)
{
    printf(_("Usage: %s [OPTION]... [FILE]...\n"), argv0);
    printf(_("Generate documentation from source comments.\n"));

    printf("\n%s\n", _("Options for C/C++ parser:"));
    printf("  -I <PATH>              %s\n", _("Add path to include search path"));
    printf("  -isystem <PATH>        %s\n", _("Add path to SYSTEM include search path"));
    printf("  -D <MACRO>[=VALUE]     %s\n", _("Define macro"));

    printf("\n%s\n", _("Options for HTML generator:"));
    printf("      --html[=DIR]       %s\n", _("Enable HTML generator and set its output directory"));
    printf("                         (%s: %s)\n", _("default"), DEFAULT_HTML_OUTPUT_DIR);
    printf("      --html-skin=<SKIN> %s (%s: %s)\n", _("Specify HTML skin"), _("default"), DEFAULT_HTML_SKIN);

    printf("\n%s\n", _("Options for JSON generator:"));
    printf("      --json[=FILE]      %s\n", _("Enable JSON generator and set its output file"));
    printf("                         (%s: %s)\n", _("default"), DEFAULT_JSON_OUTPUT_FILE);

    printf("\n%s\n", _("Auxiliary options:"));
    printf("  -n, --dry-run          %s\n", _("Do not generate any output"));
    printf("  -v, --verbose[=LEVEL]  %s\n", _("Increase/set verbose level"));
    printf("  -h, --help             %s\n", _("Display this help and exit"));
    printf("      --version          %s\n", _("Display version information and exit"));

    exit(EXIT_SUCCESS);
}


#define OPTID_2(a,b)     (((int)(a) << 8) | ((int)(b) << 0))
#define OPTID_3(a,b,c)   (((int)(a) << 16) | ((int)(b) << 8) | ((int)(c) << 0))
#define OPTID_CXX(a)     OPTID_2('C', (a))
#define OPTID_HTML(a)    OPTID_2('H', (a))
#define OPTID_JSON(a)    OPTID_2('J', (a))

static const CMDLINE_OPTION cmdline_options[] = {
    /* C/C++ parser options. */
    { '\0', "-D",           OPTID_CXX('D'), CMDLINE_OPTFLAG_COMPILERLIKE },
    { '\0', "-I",           OPTID_CXX('I'), CMDLINE_OPTFLAG_COMPILERLIKE },
    { '\0', "-isystem",     OPTID_CXX('S'), CMDLINE_OPTFLAG_COMPILERLIKE },

    /* HTML generator options. */
    { '\0', "html",         OPTID_HTML('H'), CMDLINE_OPTFLAG_OPTIONALARG },
    { '\0', "html-skin",    OPTID_HTML('S'), CMDLINE_OPTFLAG_REQUIREDARG },

    /* JSON generator options. */
    { '\0', "json",         OPTID_JSON('J'), CMDLINE_OPTFLAG_OPTIONALARG },

    /* Auxiliary options. */
    { 'n',  "dry-run",      'n', 0 },
    { 'h',  "help",         'h', 0 },
    { '\0', "version",      'V', 0 },
    { 'v',  "verbose",      'v', CMDLINE_OPTFLAG_OPTIONALARG },

    { 0 }
};


static int
cmdline_callback(int id, const char* arg, void* userdata)
{
    switch(id) {
        /* C/C++ parser options. */
        case OPTID_CXX('I'):
            array_append(&clang_opts, (void*) "-I");
            array_append(&clang_opts, (void*) arg);
            break;
        case OPTID_CXX('D'):
            array_append(&clang_opts, (void*) "-D");
            array_append(&clang_opts, (void*) arg);
            break;
        case OPTID_CXX('S'):
            array_append(&clang_opts, (void*) "-isystem");
            array_append(&clang_opts, (void*) arg);
            break;

        /* HTML generator options. */
        case OPTID_HTML('H'):
            enabled_generators |= HTML_GENERATOR;
            if(arg != NULL)
                html_output_dir = (const char*) arg;
            break;
        case OPTID_HTML('S'):
            html_skin = (const char*) arg;
            break;

        /* JSON generator options. */
        case OPTID_JSON('J'):
            enabled_generators |= JSON_GENERATOR;
            if(arg != NULL)
                json_output_file = (const char*) arg;
            break;

        /* Auxiliary options. */
        case 'n':       dry_run = 1; break;
        case 'v':       verbose_level = (arg != NULL ? atoi(arg) : verbose_level+1); break;
        case 'h':       print_usage(); break;
        case 'V':       print_version(); break;

        /* Non-option arguments, i.e. input files/dirs. */
        case 0:         array_append(&argv_paths, (void*) arg); break;

        /* Commandline parsing errors. */
        case CMDLINE_OPTID_UNKNOWN:
            FATAL(_("Unrecognized command line option '%s'."), arg);
        case CMDLINE_OPTID_MISSINGARG:
            FATAL(_("The command line option '%s' requires an argument."), arg);
        case CMDLINE_OPTID_BOGUSARG:
            FATAL(_("The command line option '%s' does not expect an argument."), arg);
    }

    return 0;
}

static void
process_input_file(const char* path, VALUE* store)
{
    const char* ext;

    ext = path_extension(path);
    if(strcmp(ext, ".h") == 0) {
        NOTE(0, _("Parsing file %s as C/C++..."), path);
        parse_cxx(path, array_data(&clang_opts), store);
    } else {
        NOTE(1, _("Skipping file %s (unknown file type)."), path);
        return;
    }

    n_processed_files++;
}

static void process_input_path(const char* path, VALUE* store);

static void
process_input_dir(const char* path, VALUE* store)
{
    char buffer[PATH_MAX];
    char dir_item[PATH_MAX];
    PATH_DIR* d;

    d = path_opendir(path);
    if(d == NULL)
        FATAL("%s (%s)", strerror(errno), path);

    while(path_readdir(d, dir_item) == 0) {

        /* Slip '.' and '..' */
        if(dir_item[0] == '.' && (dir_item[1] == '\0' || (dir_item[1] == '.' && dir_item[2] == '\0')))
            continue;

        snprintf(buffer, PATH_MAX, "%s/%s", path, dir_item);
        process_input_path(buffer, store);
    }

    path_closedir(d);
}

static void
process_input_path(const char* path, VALUE* store)
{
    if(path_is_dir(path))
        process_input_dir(path, store);
    else
        process_input_file(path, store);
}

static void
generate_output(const VALUE* store)
{
    if(dry_run)
        return;

    if(enabled_generators & HTML_GENERATOR)
        gen_html(html_output_dir, html_skin, store);

    if(enabled_generators & JSON_GENERATOR)
        gen_json(json_output_file, store);
}

int
main(int argc, char** argv)
{
    size_t i;
    VALUE* store;

#ifdef ENABLE_I18N
    setlocale(LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    argv0 = argv[0];
    path_init(argv0);

    cmdline_read(cmdline_options, argc, argv, cmdline_callback, NULL);
    array_append(&clang_opts, NULL);
    if(enabled_generators == 0)
        enabled_generators = HTML_GENERATOR;

    /* Create main data store. */
    store = store_create();

    /* Process input files. */
    for(i = 0; i < array_size(&argv_paths); i++)
        process_input_path(array_item(&argv_paths, i), store);

    if(n_processed_files == 0)
        FATAL(_("No files to process."));

    array_fini(&argv_paths, NULL);
    array_fini(&clang_opts, NULL);

    /* Generate output. */
    generate_output(store);

    /* Release data store. */
    store_destroy(store);

    return EXIT_SUCCESS;
}
