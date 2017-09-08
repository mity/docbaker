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

#include "path.h"

#ifndef _WIN32
    #include <dirent.h>
#else
    #include <io.h>
#endif


#ifdef _WIN32
struct PATH_DIR {
    intptr_t handle;
    char pattern[PATH_MAX];
};
#endif


PATH_DIR*
path_opendir(const char* path)
{
#ifdef _WIN32
    size_t len;
    PATH_DIR* dir;

    if(path == NULL || path[0] == '\0') {
        errno = ENOENT;
        return NULL;
    }

    len = strlen(path);
    if(len+2 >= PATH_MAX) {
        errno = ENOMEM;
        return NULL;
    }

    dir = (PATH_DIR*) malloc(sizeof(PATH_DIR));
    dir->handle = 0;
    strcpy(dir->pattern, path);

    if(len > 0  &&  dir->pattern[len-1] != '/'  &&  dir->pattern[len-1] != '\\')
        dir->pattern[len++] = '\\';
    dir->pattern[len++] = '*';
    dir->pattern[len++] = '\0';
    return dir;
#else
    return (PATH_DIR*) opendir(path);
#endif
}

int
path_readdir(PATH_DIR* dir, char buffer[PATH_MAX])
{
#ifdef _WIN32
    struct _finddata_t fileinfo;

    if(dir->pattern[0] != '\0') {
        dir->handle = _findfirst(dir->pattern, &fileinfo);
        if(dir->handle == -1)
            return -1;
        dir->pattern[0] = '\0';
    } else {
        if(_findnext(dir->handle, &fileinfo) == -1)
            return -1;
    }

    strcpy(buffer, fileinfo.name);
    return 0;
#else
    struct dirent* ent;

    ent = readdir((DIR*) dir);
    if(ent != NULL) {
        strcpy(buffer, ent->d_name);
        return 0;
    } else {
        return -1;
    }
#endif
}

void
path_closedir(PATH_DIR* dir)
{
#ifdef _WIN32
    _findclose(dir->handle);
    free(dir);
#else
    closedir((DIR*) dir);
#endif
}


#ifdef _WIN32
/* Helper for path_basename() below. */
static char*
strrpbrk(const char *s, const char *accept)
{
    const char* p;
    char* p0;
    char* p1;

    for(p = accept, p0 = p1 = NULL; p && *p; ++p) {
        p1 = strrchr(s, *p);
        if(p1 && p1 > p0)
            p0 = p1;
    }
    return p0;
}
#endif

const char*
path_basename(const char* path)
{
    const char* last_delim;

#ifdef _WIN32
    /* On Win32, '/' as well as '\\' may be used as path separator. */
    last_delim = strrpbrk(path, "\\/");
#else
    last_delim = strrchr(path, '/');
#endif

    return (last_delim ? last_delim+1 : path);
}

const char*
path_extension(const char* path)
{
    const char* fname;
    const char* last_dot;

    fname = path_basename(path);
    last_dot = strrchr(fname, '.');

    return (last_dot ? last_dot : "");
}

int
path_is_dir(const char* path)
{
    struct stat s;

    return (stat(path, &s) == 0  &&  S_ISDIR(s.st_mode)) ? 1 : 0;
}

