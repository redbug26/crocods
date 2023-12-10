#include "multitools.h"

#include <string.h>
#include <stdio.h>

#ifdef ANDROID
        #define MAX_PATH 2048
#elif _WIN32
        #include <windows.h>
        #include <winbase.h>
        #include <direct.h>
#elif defined(TARGET_OS_MAC)
        #include <unistd.h>
        #define MAX_PATH 2048
#elif defined(TARGET_OS_LINUX)
        #include <unistd.h>
        #define MAX_PATH 2048
#elif defined(GCW)
        #include <unistd.h>
        #define MAX_PATH 2048
#elif  defined(RPI)
        #include <unistd.h>
        #define MAX_PATH 2048
#elif defined(EMSCRIPTEN)
        #include <unistd.h>
        #define MAX_PATH 2048
#endif


#ifndef MAX_PATH
#define MAX_PATH         2048
#endif

void current_workpath(char *path)
{
    path[0] = 0;

    char sbuf[MAX_PATH];

#ifdef _WIN32
    if (GetModuleFileNameA(NULL, sbuf, MAX_PATH) != 0) {
        // printf("(%s)\n", sbuf);
        char *s = strrchr(sbuf, '\\');
        if (s != NULL) {
            *s = 0;
        }
        // printf("(%s)\n", sbuf);

        strcpy(path, sbuf);
    }
#elif defined(TARGET_OS_MAC)
    if (getcwd(sbuf, MAX_PATH) != NULL) {
        // printf("(%s)\n", sbuf);

        strcpy(path, sbuf);
    }
#elif defined(TARGET_OS_LINUX)
    if (getcwd(sbuf, MAX_PATH) != NULL) {
        // printf("(%s)\n", sbuf);

        strcpy(path, sbuf);
    }
#elif defined(GCW)
    if (getcwd(sbuf, MAX_PATH) != NULL) {
        // printf("(%s)\n", sbuf);

        strcpy(path, sbuf);
    }
#elif defined(EMSCRIPTEN)
    if (getcwd(sbuf, MAX_PATH) != NULL) {
        // printf("(%s)\n", sbuf);

        strcpy(path, sbuf);
    }
#endif /* ifdef _WIN32 */
} /* current_workpath */
