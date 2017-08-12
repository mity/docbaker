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
#include "parse_cxx.h"


int verbose_level = 0;

static char* argv0;
static ARRAY argv_paths = ARRAY_INITIALIZER;
static ARRAY clang_opts = ARRAY_INITIALIZER;



static void
print_version(void)
{
    printf("%s %s\n", argv0, VERSION);
    exit(EXIT_SUCCESS);
}

static void
print_usage(void)
{
    printf(_("Usage: %s [OPTION]... [FILE]...\n"), argv0);
    printf(_("Generate documentation from source comments.\n"));

    printf("\n%s\n", _("Options for C/C++ parser:"));
    printf("  -I <PATH>                 %s\n", _("Add path to include search path"));
    printf("  -isystem <PATH>           %s\n", _("Add path to SYSTEM include search path"));
    printf("  -D <MACRO>[=VALUE]        %s\n", _("Define macro"));

    printf("\n%s\n", _("Auxiliary options:"));
    printf("  -v, --verbose[=LEVEL]     %s\n", _("Increase/set verbose level"));
    printf("  -h, --help                %s\n", _("Display this help and exit"));
    printf("      --version             %s\n", _("Display version information and exit"));

    exit(EXIT_SUCCESS);
}


static const CMDLINE_OPTION cmdline_options[] = {
    { 'h',  "help",     'h', 0 },
    { '\0', "version",  'V', 0 },
    { 'v',  "verbose",  'v', CMDLINE_OPTFLAG_OPTIONALARG },

    { '\0', "-D",       'D', CMDLINE_OPTFLAG_COMPILERLIKE },
    { '\0', "-I",       'I', CMDLINE_OPTFLAG_COMPILERLIKE },
    { '\0', "-isystem", 'S', CMDLINE_OPTFLAG_COMPILERLIKE },

    { 0 }
};

typedef struct CMDLINE_CONTEXT {
} CMDLINE_CONTEXT;


static int
cmdline_callback(int id, const char* arg, void* userdata)
{
    switch(id) {
        case 'v':       verbose_level = (arg != NULL ? atoi(arg) : verbose_level+1); break;
        case 'h':       print_usage(); break;
        case 'V':       print_version(); break;

        /* These are propagated to parse_cxx(). */
        case 'I':       array_append(&clang_opts, (void*) "-I");
                        array_append(&clang_opts, (void*) arg); break;

        case 'D':       array_append(&clang_opts, (void*) "-D");
                        array_append(&clang_opts, (void*) arg); break;

        case 'S':       array_append(&clang_opts, (void*) "-isystem");
                        array_append(&clang_opts, (void*) arg); break;

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
process_file(const char* path)
{
    const char* fname;
    const char* ext;

    fname = strrchr(path, '/');
    if(fname != NULL)
        fname++;
    else
        fname = path;
    ext = strrchr(fname, '.');

    if(ext != NULL  &&  strcmp(ext, ".h") == 0) {
        NOTE(0, _("Parsing file %s as C/C++..."), path);
        parse_cxx(path, array_data(&clang_opts));
    } else {
        NOTE(1, _("Skipping file %s (unknown file type)."), path);
    }
}

static void process_path(const char* path);

static void
process_dir(const char* path)
{
    char buffer[PATH_MAX];
    DIR* d;
    struct dirent* dent;

    d = opendir(path);
    if(d == NULL)
        FATAL("%s (%s)", strerror(errno), path);

    while(1) {
        dent = readdir(d);
        if(dent == NULL)
            break;

        if(dent->d_name[0] == '.')
            continue;

        snprintf(buffer, PATH_MAX, "%s/%s", path, dent->d_name);
        process_path(buffer);
    }

    closedir(d);
}

static void
process_path(const char* path)
{
    struct stat s;

    if(stat(path, &s) != 0)
        FATAL("%s (%s)", strerror(errno), path);

    if(S_ISDIR(s.st_mode))
        process_dir(path);
    else
        process_file(path);
}

int
main(int argc, char** argv)
{
    size_t i;

#ifdef ENABLE_I18N
    setlocale(LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    argv0 = argv[0];
    cmdline_read(cmdline_options, argc, argv, cmdline_callback, NULL);
    array_append(&clang_opts, NULL);

    for(i = 0; i < array_size(&argv_paths); i++)
        process_path(array_item(&argv_paths, i));

    array_fini(&argv_paths, NULL);
    array_fini(&clang_opts, NULL);

    return EXIT_SUCCESS;
}
