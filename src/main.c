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
#include "cmdline.h"


int verbose_level = 0;
static char* argv0;


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

    printf(_("\nAuxiliary options:\n"));
    printf("  -v, --verbose[=LEVEL]     %s\n", _("Increase/set verbose level"));
    printf("  -h, --help                %s\n", _("Display this help and exit"));
    printf("      --version             %s\n", _("Display version information and exit"));

    exit(EXIT_SUCCESS);
}


static const CMDLINE_OPTION cmdline_options[] = {
    { 'h',  "help",     'h', 0 },
    { '\0', "version",  'V', 0 },
    { 'v',  "verbose",  'v', CMDLINE_OPTFLAG_OPTIONALARG },
    { 0 }
};

static int
cmdline_callback(int id, const char* arg, void* userdata)
{
    switch(id) {
        case 'v':       verbose_level = (arg != NULL ? atoi(arg) : verbose_level+1); break;
        case 'h':       print_usage(); break;
        case 'V':       print_version(); break;

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


int
main(int argc, char** argv)
{
#ifdef ENABLE_I18N
    setlocale(LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    argv0 = argv[0];

    cmdline_read(cmdline_options, argc, argv, cmdline_callback, NULL);

    return EXIT_SUCCESS;
}
