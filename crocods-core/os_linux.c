#include "os.h"

#ifdef TARGET_OS_LINUX

#include <stdio.h>
#include <stdlib.h>

void * os_fopen(const char *filename, const char *mode)
{
    return fopen(filename, mode);
}

#endif