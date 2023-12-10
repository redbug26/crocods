#include "os.h"

#ifdef TARGET_OS_MAC
#ifndef _WIN32

#include <stdio.h>
#include <stdlib.h>


void * os_fopen(const char *filename, const char *mode)
{
    return fopen(filename, mode);
}

#endif
#endif