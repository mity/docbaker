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

#ifndef DOCBAKER_CMDLINE_H
#define DOCBAKER_CMDLINE_H



/* The option may have an argument. (Affects only long option.) */
#define CMDLINE_OPTFLAG_OPTIONALARG     0x0001

/* The option must have an argument.
 * Such short option cannot be grouped within single '-abc'. */
#define CMDLINE_OPTFLAG_REQUIREDARG     0x0002

/* Enables special mode for long options.
 *
 * Note ::shortname is not supported with this flag (CMDLINE_OPTION::shortname
 * is ignored if the flag is used).
 *
 * It makes cmdline_read() to interpret its long name quite differently:
 *
 * 1. The option matches if the ::longname is the exact prefix of the token
 *    from commandline.
 *
 * 2. Double dash ("--") is not automatically prepended to ::longname.
 *    (::longname may explicitly include any leading '-' if it is desired.)
 *
 * 3. An argument (optionally after a whitespace) is required after it but
 *    there is no delimiter (no "=" between the option and its argument).
 *
 * Expected usage is for e.g. compiler-like options, for example:
 *  -DDEBUG=0
 *  -Isrc/include
 *  -isystem /usr/local/include
 *  -lmath
 */
#define CMDLINE_OPTFLAG_COMPILERLIKE    0x0004


#define CMDLINE_OPTID_NONE              0
#define CMDLINE_OPTID_UNKNOWN           (-0x7fffffff + 0)
#define CMDLINE_OPTID_MISSINGARG        (-0x7fffffff + 1)
#define CMDLINE_OPTID_BOGUSARG          (-0x7fffffff + 2)


typedef struct CMDLINE_OPTION {
    char shortname;         /* Short (single char) option or 0. */
    const char* longname;   /* Long name (after "--") or NULL. */
    int id;                 /* Non-zero ID to identify the option in the callback; or zero to denote end of options list. */
    unsigned flags;         /* Bitmask of CMDLINE_OPTFLAG_xxxx flags. */
} CMDLINE_OPTION;


/* Parse all options and their arguments as specified by argc, argv accordingly
 * with the given options. The array of supported options has to be ended
 * with member whose CMDLINE_OPTION::id is zero.
 *
 * Note argv[0] is ignored.
 *
 * The callback is called for each (validly matching) option.
 * It is also called for any positional argument (with id set to zero).
 *
 * Special cases (errorneous command line):
 *   -- If an option is encountered, which is missing in the list of known
 *      options, callback is called with id CMDLINE_OPTID_UNKNOWNOPTION
 *      and arg is the name of the option.
 *   -- If an option is encountered, which requires an argument but none is
 *      provided, callback is called with id equal to CMDLINE_OPTID_MISSINGARG
 *      and arg is the name of the option.
 *   -- If a (long) option is encountered, which does not expext an argument
 *      but one is provided (e.g. --foo=bar), callback is called with id
 *      CMDLINE_OPTID_BOGUSARG and arg is the name of the option.
 *
 * On success, zero is returned.
 *
 * If the callback returns non-zero cmdline_read() aborts any subsequent
 * parsing and it returns the same value to the caller.
 */

int cmdline_read(const CMDLINE_OPTION* options, int argc, char** argv,
        int (*callback)(int /*id*/, const char* /*arg*/, void* /*userdata*/),
        void* userdata);


#endif  /* DOCBAKER_CMDLINE_H */
