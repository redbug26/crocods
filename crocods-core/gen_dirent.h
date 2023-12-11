// dirent

#if !defined (_WIN32)
#include <dirent.h>
#define DIRENT_INCLUDED
#endif

#ifndef DIRENT_INCLUDED
#define DIRENT_INCLUDED

// Implementation of POSIX directory browsing functions and types for Win32.

// Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
// History: Created March 1997. Updated June 2003 and July 2012.

#ifndef S_IFMT
#define S_IFMT  0170000                        /* [XSI] type of file mask */
#endif

#ifndef S_IFDIR
#define S_IFDIR 0040000                        /* [XSI] directory */
#endif

#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)   /* directory */
#endif

typedef struct DIR DIR;

struct dirent {
    char *d_name;
};

DIR * opendir(const char *);
int           closedir(DIR *);
struct dirent * readdir(DIR *);
void          rewinddir(DIR *);


#include <errno.h>
#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
#include <stdlib.h>
#include <string.h>

typedef ptrdiff_t handle_type; /* C99's intptr_t not sufficiently portable */

struct DIR {
    handle_type handle;         /* -1 for failed rewind */
    struct _finddata_t info;
    struct dirent result;       /* d_name null iff first time */
    char *name;                 /* null-terminated char string */
};

#endif /* ifndef DIRENT_INCLUDED */